/*
 * Copyright (c) 2022 Chipsea Technologies (Shenzhen) Corp., Ltd.
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

#include "iproxy_broker.h"
#include "sensor_event_callback.h"
#include "v1_0/isensor_interface.h"
#include "medical_errors.h"
#include "medical_log_domain.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;
using OHOS::HDI::Sensor::V1_0::ISensorInterface;
using OHOS::HDI::Sensor::V1_0::ISensorCallback;
using OHOS::HDI::Sensor::V1_0::HdfSensorInformation;
namespace {
constexpr HiLogLabel LABEL = {
    LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_HDI_ADAPTOR, "MedicalSensor_HdiConnection"
};
sptr<ISensorInterface> sensorInterface_ = nullptr;
sptr<ISensorCallback> eventCallback_ = nullptr;
std::map<int32_t, MedicalSensorBasicInfo> sensorBasicInfoMap_;
std::mutex sensorBasicInfoMutex_;
constexpr int32_t GET_HDI_SERVICE_COUNT = 20;
constexpr uint32_t WAIT_MS = 1000;
}

DataCacheFunc HdiConnection::cacheData_ = nullptr;
sptr<ReportDataCache> HdiConnection::reportDataCache_ = nullptr;

int32_t HdiConnection::ConnectHdi()
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    int32_t retry = 0;
    while (retry < GET_HDI_SERVICE_COUNT) {
        sensorInterface_ = ISensorInterface::Get();
        if (sensorInterface_ != nullptr) {
            HiLog::Info(LABEL, "%{public}s connect v1_0 hdi success", __func__);
            eventCallback_ = new (std::nothrow) SensorEventCallback();
            if (eventCallback_ == nullptr) {
                HiLog::Error(LABEL, "%{public}s failed to initialize eventCallback", __func__);
                return ERR_NO_INIT;
            }
            RegisterHdiDeathRecipient();
            return ERR_OK;
        }
        retry++;
        HiLog::Warn(LABEL, "%{public}s connect hdi service failed, retry : %{public}d", __func__, retry);
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
    }
    HiLog::Error(LABEL, "%{public}s connect v1_0 hdi failed", __func__);
    return ERR_NO_INIT;
}

int32_t HdiConnection::GetSensorList(std::vector<MedicalSensor>& sensorList)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s connect v1_0 hdi failed", __func__);
        return ERR_NO_INIT;
    }
    std::vector<HdfSensorInformation> sensorInfos;
    int32_t ret = sensorInterface_->GetAllSensorInfo(sensorInfos);
    if (ret != 0) {
        HiLog::Error(LABEL, "%{public}s get sensor list failed", __func__);
        return ret;
    }
    for (int32_t i = 0; i < static_cast<int32_t>(sensorInfos.size()); i++) {
        const std::string sensorName(sensorInfos[i].sensorName);
        const std::string vendorName(sensorInfos[i].vendorName);
        const int32_t sensorId = sensorInfos[i].sensorId;
        const float maxRange = sensorInfos[i].maxRange;
        MedicalSensor sensor;
        sensor.SetSensorId(sensorId);
        sensor.SetMaxRange(maxRange);
        sensor.SetName(sensorName.c_str());
        sensor.SetVendor(vendorName.c_str());
        sensorList.push_back(sensor);
    }
    return ERR_OK;
}

int32_t HdiConnection::EnableSensor(int32_t sensorId)
{
    if (sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s connect v1_0 hdi failed", __func__);
        return ERR_NO_INIT;
    }
    int32_t ret = sensorInterface_->Enable(sensorId);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s is failed", __func__);
        return ret;
    }
    setSensorBasicInfoState(sensorId, SENSOR_ENABLED);
    return ERR_OK;
}

int32_t HdiConnection::DisableSensor(int32_t sensorId)
{
    if (sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s connect v1_0 hdi failed", __func__);
        return ERR_NO_INIT;
    }
    int32_t ret = sensorInterface_->Disable(sensorId);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s is failed", __func__);
        return ret;
    }
    deleteSensorBasicInfoState(sensorId);
    return ERR_OK;
}

int32_t HdiConnection::SetBatch(int32_t sensorId, int64_t samplingInterval, int64_t reportInterval)
{
    if (sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s connect v1_0 hdi failed", __func__);
        return ERR_NO_INIT;
    }
    int32_t ret = sensorInterface_->SetBatch(sensorId, samplingInterval, reportInterval);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s failed", __func__);
        return ret;
    }
    updateSensorBasicInfo(sensorId, samplingInterval, reportInterval);
    return ERR_OK;
}

int32_t HdiConnection::SetMode(int32_t sensorId, int32_t mode)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s connect v1_0 hdi failed", __func__);
        return ERR_NO_INIT;
    }
    int32_t ret = sensorInterface_->SetMode(sensorId, mode);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s is failed", __func__);
        return ret;
    }
    return ERR_OK;
}

int32_t HdiConnection::SetOption(int32_t sensorId, int32_t option)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s connect v1_0 hdi failed", __func__);
        return ERR_NO_INIT;
    }
    int32_t ret = sensorInterface_->SetOption(sensorId, option);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s is failed", __func__);
        return ret;
    }
    return ERR_OK;
}

int32_t HdiConnection::RegisteDataReport(DataCacheFunc cacheData, sptr<ReportDataCache> reportDataCache)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (reportDataCache == nullptr || sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s failed, reportDataCache or sensorInterface_ cannot be null", __func__);
        return ERR_NO_INIT;
    }
    int32_t ret = sensorInterface_->Register(1, eventCallback_);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s failed", __func__);
        return ret;
    }
    cacheData_ = cacheData;
    reportDataCache_ = reportDataCache;
    return ERR_OK;
}

int32_t HdiConnection::DestroyHdiConnection()
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s connect v1_0 hdi failed", __func__);
        return ERR_NO_INIT;
    }
    int32_t ret = sensorInterface_->Unregister(1, eventCallback_);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s failed", __func__);
        return ret;
    }
    eventCallback_ = nullptr;
    UnregisterHdiDeathRecipient();
    return ERR_OK;
}

int32_t HdiConnection::RunCommand(int32_t sensorId, int32_t cmd, int32_t params)
{
    return 0;
}

DataCacheFunc HdiConnection::getReportDataCb()
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (cacheData_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s cacheData_ cannot be null", __func__);
    }
    return cacheData_;
}

sptr<ReportDataCache> HdiConnection::getReportDataCallback()
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (reportDataCache_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s reportDataCache_ cannot be null", __func__);
    }
    return reportDataCache_;
}

void HdiConnection::updateSensorBasicInfo(int32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs)
{
    std::lock_guard<std::mutex> sensorInfoLock(sensorBasicInfoMutex_);
    MedicalSensorBasicInfo sensorBasicInfo;
    sensorBasicInfo.SetSamplingPeriodNs(samplingPeriodNs);
    sensorBasicInfo.SetMaxReportDelayNs(maxReportDelayNs);
    sensorBasicInfoMap_[sensorId] = sensorBasicInfo;
}

void HdiConnection::setSensorBasicInfoState(int32_t sensorId, MedicalSensorState state)
{
    std::lock_guard<std::mutex> sensorInfoLock(sensorBasicInfoMutex_);
    auto it = sensorBasicInfoMap_.find(sensorId);
    if (it == sensorBasicInfoMap_.end()) {
        HiLog::Warn(LABEL, "%{public}s should set batch first", __func__);
    }
    sensorBasicInfoMap_[sensorId].SetSensorState(state);
}

void HdiConnection::deleteSensorBasicInfoState(int32_t sensorId)
{
    std::lock_guard<std::mutex> sensorInfoLock(sensorBasicInfoMutex_);
    auto it = sensorBasicInfoMap_.find(sensorId);
    if (it != sensorBasicInfoMap_.end()) {
        sensorBasicInfoMap_.erase(sensorId);
    }
}

void HdiConnection::RegisterHdiDeathRecipient()
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (sensorInterface_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s connect v1_0 hdi failed", __func__);
        return;
    }
    hdiDeathObserver_ = new (std::nothrow) DeathRecipientTemplate(*const_cast<HdiConnection *>(this));
    if (hdiDeathObserver_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s hdiDeathObserver_ cannot be null", __func__);
        return;
    }
    OHOS::HDI::hdi_objcast<ISensorInterface>(sensorInterface_)->AddDeathRecipient(hdiDeathObserver_);
}

void HdiConnection::UnregisterHdiDeathRecipient()
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (sensorInterface_ == nullptr || hdiDeathObserver_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s sensorInterface_ or hdiDeathObserver_ is null", __func__);
        return;
    }
    OHOS::HDI::hdi_objcast<ISensorInterface>(sensorInterface_)->RemoveDeathRecipient(hdiDeathObserver_);
}

void HdiConnection::ProcessDeathObserver(const wptr<IRemoteObject> &object)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    sptr<IRemoteObject> hdiService = object.promote();
    if (hdiService == nullptr) {
        HiLog::Error(LABEL, "%{public}s invalid remote object", __func__);
        return;
    }
    hdiService->RemoveDeathRecipient(hdiDeathObserver_);
    eventCallback_ = nullptr;
    reconnect();
}

void HdiConnection::reconnect()
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    int32_t ret = ConnectHdi();
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s failed to get an instance of hdi service", __func__);
        return;
    }
    ret = sensorInterface_->Register(0, eventCallback_);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s register callback fail", __func__);
        return;
    }
    std::vector<MedicalSensor> sensorList;
    ret = GetSensorList(sensorList);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s get sensor list fail", __func__);
        return;
    }
    std::lock_guard<std::mutex> sensorInfoLock(sensorBasicInfoMutex_);
    for (const auto &sensorInfo: sensorBasicInfoMap_) {
        int32_t sensorTypeId = sensorInfo.first;
        ret = SetBatch(sensorTypeId, sensorInfo.second.GetSamplingPeriodNs(),
            sensorInfo.second.GetMaxReportDelayNs());
        if (ret < 0 || sensorInfo.second.GetSensorState() != SENSOR_ENABLED) {
            HiLog::Error(LABEL, "%{public}s sensorTypeId: %{public}d set batch fail or not need enable sensor",
                __func__, sensorTypeId);
            continue;
        }
        ret = EnableSensor(sensorTypeId);
        if (ret < 0) {
            HiLog::Error(LABEL, "%{public}s enable sensor fail, sensorTypeId: %{public}d", __func__, sensorTypeId);
        }
    }
}
}  // namespace Sensors
}  // namespace OHOS
