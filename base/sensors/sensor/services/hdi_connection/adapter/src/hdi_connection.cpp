/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "hdi_connection.h"

#include <map>
#include <mutex>
#include <thread>

#include "hisysevent.h"
#include "iproxy_broker.h"
#include "v2_0/isensor_interface.h"

#include "sensor_agent_type.h"
#include "sensor_errors.h"
#include "sensor_event_callback.h"

#undef LOG_TAG
#define LOG_TAG "HdiConnection"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;
using OHOS::HDI::Sensor::V2_0::ISensorInterface;
using OHOS::HDI::Sensor::V2_0::ISensorCallback;
using OHOS::HDI::Sensor::V2_0::HdfSensorInformation;
namespace {
sptr<ISensorInterface> g_sensorInterface = nullptr;
sptr<ISensorCallback> g_eventCallback  = nullptr;
std::map<int32_t, SensorBasicInfo> g_sensorBasicInfoMap;
std::mutex g_sensorBasicInfoMutex;
constexpr int32_t GET_HDI_SERVICE_COUNT = 25;
constexpr uint32_t WAIT_MS = 200;
constexpr int32_t HEADPOSTURE_FIFO_COUNT = 5;
}  // namespace

ReportDataCb HdiConnection::reportDataCb_ = nullptr;
sptr<ReportDataCallback> HdiConnection::reportDataCallback_ = nullptr;

int32_t HdiConnection::ConnectHdi()
{
    CALL_LOG_ENTER;
    int32_t retry = 0;
    while (retry < GET_HDI_SERVICE_COUNT) {
        g_sensorInterface = ISensorInterface::Get();
        if (g_sensorInterface != nullptr) {
            SEN_HILOGI("Connect v2_0 hdi success");
            g_eventCallback  = new (std::nothrow) SensorEventCallback();
            CHKPR(g_eventCallback, ERR_NO_INIT);
            RegisterHdiDeathRecipient();
            return ERR_OK;
        }
        retry++;
        SEN_HILOGW("Connect hdi service failed, retry:%{public}d", retry);
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
    }
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::SENSOR, "HDF_SERVICE_EXCEPTION",
        HiSysEvent::EventType::FAULT, "PKG_NAME", "ConnectHdi", "ERROR_CODE", CONNECT_SENSOR_HDF_ERR);
    SEN_HILOGE("Connect V2_0 hdi failed");
    return ERR_NO_INIT;
}

int32_t HdiConnection::GetSensorList(std::vector<Sensor> &sensorList)
{
    CALL_LOG_ENTER;
    CHKPR(g_sensorInterface, ERR_NO_INIT);
    std::vector<HdfSensorInformation> sensorInfos;
    int32_t ret = g_sensorInterface->GetAllSensorInfo(sensorInfos);
    if (ret != 0) {
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::SENSOR, "HDF_SERVICE_EXCEPTION",
            HiSysEvent::EventType::FAULT, "PKG_NAME", "GetSensorList", "ERROR_CODE", ret);
        SEN_HILOGE("Get sensor list failed");
        return ret;
    }
    size_t count = sensorInfos.size();
    if (count > MAX_SENSOR_COUNT) {
        SEN_HILOGD("SensorInfos size:%{public}zu", count);
        count = MAX_SENSOR_COUNT;
    }
    for (size_t i = 0; i < count; i++) {
        Sensor sensor;
        sensor.SetSensorId(sensorInfos[i].sensorId);
        sensor.SetSensorTypeId(sensorInfos[i].sensorId);
        sensor.SetFirmwareVersion(sensorInfos[i].firmwareVersion);
        sensor.SetHardwareVersion(sensorInfos[i].hardwareVersion);
        sensor.SetMaxRange(sensorInfos[i].maxRange);
        sensor.SetSensorName(sensorInfos[i].sensorName);
        sensor.SetVendorName(sensorInfos[i].vendorName);
        sensor.SetResolution(sensorInfos[i].accuracy);
        sensor.SetPower(sensorInfos[i].power);
        sensor.SetMinSamplePeriodNs(sensorInfos[i].minDelay);
        sensor.SetMaxSamplePeriodNs(sensorInfos[i].maxDelay);
        if (sensorInfos[i].sensorId == SENSOR_TYPE_ID_HEADPOSTURE) {
            sensor.SetFifoMaxEventCount(HEADPOSTURE_FIFO_COUNT);
        }
        sensorList.push_back(sensor);
    }
    return ERR_OK;
}

int32_t HdiConnection::EnableSensor(int32_t sensorId)
{
    CHKPR(g_sensorInterface, ERR_NO_INIT);
    int32_t ret = g_sensorInterface->Enable(sensorId);
    if (ret != 0) {
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::SENSOR, "HDF_SERVICE_EXCEPTION",
            HiSysEvent::EventType::FAULT, "PKG_NAME", "EnableSensor", "ERROR_CODE", ret);
        SEN_HILOGE("Connect V2_0 hdi failed");
        return ret;
    }
    SetSensorBasicInfoState(sensorId, true);
    return ERR_OK;
}

int32_t HdiConnection::DisableSensor(int32_t sensorId)
{
    CHKPR(g_sensorInterface, ERR_NO_INIT);
    int32_t ret = g_sensorInterface->Disable(sensorId);
    if (ret != 0) {
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::SENSOR, "HDF_SERVICE_EXCEPTION",
            HiSysEvent::EventType::FAULT, "PKG_NAME", "DisableSensor", "ERROR_CODE", ret);
        SEN_HILOGE("Disable is failed");
        return ret;
    }
    DeleteSensorBasicInfoState(sensorId);
    return ERR_OK;
}

int32_t HdiConnection::SetBatch(int32_t sensorId, int64_t samplingInterval, int64_t reportInterval)
{
    CHKPR(g_sensorInterface, ERR_NO_INIT);
    int32_t ret = g_sensorInterface->SetBatch(sensorId, samplingInterval, reportInterval);
    if (ret != 0) {
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::SENSOR, "HDF_SERVICE_EXCEPTION",
            HiSysEvent::EventType::FAULT, "PKG_NAME", "SetBatch", "ERROR_CODE", ret);
        SEN_HILOGE("SetBatch is failed");
        return ret;
    }
    UpdateSensorBasicInfo(sensorId, samplingInterval, reportInterval);
    return ERR_OK;
}

int32_t HdiConnection::SetMode(int32_t sensorId, int32_t mode)
{
    CALL_LOG_ENTER;
    CHKPR(g_sensorInterface, ERR_NO_INIT);
    int32_t ret = g_sensorInterface->SetMode(sensorId, mode);
    if (ret != 0) {
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::SENSOR, "HDF_SERVICE_EXCEPTION",
            HiSysEvent::EventType::FAULT, "PKG_NAME", "SetMode", "ERROR_CODE", ret);
        SEN_HILOGE("SetMode is failed");
        return ret;
    }
    return ERR_OK;
}

int32_t HdiConnection::RegisterDataReport(ReportDataCb cb, sptr<ReportDataCallback> reportDataCallback)
{
    CALL_LOG_ENTER;
    CHKPR(reportDataCallback, ERR_NO_INIT);
    CHKPR(g_sensorInterface, ERR_NO_INIT);
    int32_t ret = g_sensorInterface->Register(0, g_eventCallback);
    if (ret != 0) {
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::SENSOR, "HDF_SERVICE_EXCEPTION",
            HiSysEvent::EventType::FAULT, "PKG_NAME", "RegisterDataReport", "ERROR_CODE", ret);
        SEN_HILOGE("Register is failed");
        return ret;
    }
    reportDataCb_ = cb;
    reportDataCallback_ = reportDataCallback;
    return ERR_OK;
}

int32_t HdiConnection::DestroyHdiConnection()
{
    CALL_LOG_ENTER;
    CHKPR(g_sensorInterface, ERR_NO_INIT);
    int32_t ret = g_sensorInterface->Unregister(0, g_eventCallback);
    if (ret != 0) {
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::SENSOR, "HDF_SERVICE_EXCEPTION",
            HiSysEvent::EventType::FAULT, "PKG_NAME", "DestroyHdiConnection", "ERROR_CODE", ret);
        SEN_HILOGE("Unregister is failed");
        return ret;
    }
    g_eventCallback  = nullptr;
    UnregisterHdiDeathRecipient();
    return ERR_OK;
}

ReportDataCb HdiConnection::GetReportDataCb()
{
    if (reportDataCb_ == nullptr) {
        SEN_HILOGE("reportDataCb_ cannot be null");
    }
    return reportDataCb_;
}

sptr<ReportDataCallback> HdiConnection::GetReportDataCallback()
{
    if (reportDataCallback_ == nullptr) {
        SEN_HILOGE("reportDataCallback_ cannot be null");
    }
    return reportDataCallback_;
}

void HdiConnection::UpdateSensorBasicInfo(int32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs)
{
    std::lock_guard<std::mutex> sensorInfoLock(g_sensorBasicInfoMutex);
    SensorBasicInfo sensorBasicInfo;
    sensorBasicInfo.SetSamplingPeriodNs(samplingPeriodNs);
    sensorBasicInfo.SetMaxReportDelayNs(maxReportDelayNs);
    auto it = g_sensorBasicInfoMap.find(sensorId);
    if (it != g_sensorBasicInfoMap.end()) {
        if (g_sensorBasicInfoMap[sensorId].GetSensorState()) {
            sensorBasicInfo.SetSensorState(true);
        }
    }
    g_sensorBasicInfoMap[sensorId] = sensorBasicInfo;
}

void HdiConnection::SetSensorBasicInfoState(int32_t sensorId, bool state)
{
    std::lock_guard<std::mutex> sensorInfoLock(g_sensorBasicInfoMutex);
    auto it = g_sensorBasicInfoMap.find(sensorId);
    if (it == g_sensorBasicInfoMap.end()) {
        SEN_HILOGW("Should set batch first");
        return;
    }
    g_sensorBasicInfoMap[sensorId].SetSensorState(state);
}

void HdiConnection::DeleteSensorBasicInfoState(int32_t sensorId)
{
    std::lock_guard<std::mutex> sensorInfoLock(g_sensorBasicInfoMutex);
    auto it = g_sensorBasicInfoMap.find(sensorId);
    if (it != g_sensorBasicInfoMap.end()) {
        g_sensorBasicInfoMap.erase(sensorId);
    }
}

void HdiConnection::RegisterHdiDeathRecipient()
{
    CALL_LOG_ENTER;
    CHKPV(g_sensorInterface);
    if (hdiDeathObserver_ == nullptr) {
        hdiDeathObserver_ = new (std::nothrow) DeathRecipientTemplate(*const_cast<HdiConnection *>(this));
        CHKPV(hdiDeathObserver_);
    }
    OHOS::HDI::hdi_objcast<ISensorInterface>(g_sensorInterface)->AddDeathRecipient(hdiDeathObserver_);
}

void HdiConnection::UnregisterHdiDeathRecipient()
{
    CALL_LOG_ENTER;
    CHKPV(g_sensorInterface);
    CHKPV(hdiDeathObserver_);
    OHOS::HDI::hdi_objcast<ISensorInterface>(g_sensorInterface)->RemoveDeathRecipient(hdiDeathObserver_);
}

void HdiConnection::ProcessDeathObserver(const wptr<IRemoteObject> &object)
{
    CALL_LOG_ENTER;
    sptr<IRemoteObject> hdiService = object.promote();
    CHKPV(hdiService);
    CHKPV(hdiDeathObserver_);
    hdiService->RemoveDeathRecipient(hdiDeathObserver_);
    g_eventCallback  = nullptr;
    Reconnect();
}

void HdiConnection::Reconnect()
{
    CALL_LOG_ENTER;
    int32_t ret = ConnectHdi();
    if (ret != 0) {
        SEN_HILOGE("Failed to get an instance of hdi service");
        return;
    }
    ret = g_sensorInterface->Register(0, g_eventCallback);
    if (ret != 0) {
        SEN_HILOGE("Register callback fail");
        return;
    }
    std::vector<Sensor> sensorList;
    ret = GetSensorList(sensorList);
    if (ret != 0) {
        SEN_HILOGE("Get sensor list fail");
        return;
    }
    std::lock_guard<std::mutex> sensorInfoLock(g_sensorBasicInfoMutex);
    for (const auto &sensorInfo: g_sensorBasicInfoMap) {
        int32_t sensorTypeId = sensorInfo.first;
        SensorBasicInfo info = sensorInfo.second;
        if (info.GetSensorState() != true) {
            SEN_HILOGE("sensorTypeId:%{public}d don't need enable sensor", sensorTypeId);
            continue;
        }
        ret = g_sensorInterface->SetBatch(sensorTypeId, info.GetSamplingPeriodNs(), info.GetMaxReportDelayNs());
        if (ret != 0) {
            SEN_HILOGE("sensorTypeId:%{public}d set batch fail, error:%{public}d", sensorTypeId, ret);
            continue;
        }
        ret = g_sensorInterface->Enable(sensorTypeId);
        if (ret != 0) {
            SEN_HILOGE("Enable sensor fail, sensorTypeId:%{public}d, error:%{public}d", sensorTypeId, ret);
        }
    }
}
}  // namespace Sensors
}  // namespace OHOS
