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
#include "hdi_service_impl.h"

#include "medical_errors.h"
#include "medical_log_domain.h"
#include "unistd.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = {
    LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_HDI_HARDWARE, "MedicalSensor_HdiServiceImpl"
};
constexpr int64_t SAMPLING_INTERVAL_NS = 200000000;
constexpr int32_t CONVERT_MULTIPLES = 1000;
std::vector<SensorInformation> g_sensorInfos = {
    {"sensor_test", "default", "1.0.0", "1.0.0", 0, 0, 9999.0, 0.000001, 23.0},
};
std::vector<int32_t> supportSensors = {0};
float g_testData[] = {9.8F};
SensorEvents testEvent = {
    .sensorId = 0,
    .data = static_cast<uint8_t *>(g_testData),
    .dataLen = 4
};
}
RecordDataCallback HdiServiceImpl::g_callback;
int64_t HdiServiceImpl::g_samplingInterval = -1;
int64_t HdiServiceImpl::g_reportInterval = -1;
std::atomic_bool HdiServiceImpl::g_isStop = false;

int32_t HdiServiceImpl::GetSensorList(std::vector<SensorInformation>& sensorList)
{
    HiLog::Info(LABEL, "%{public}s in", __func__);
    sensorList.assign(g_sensorInfos.begin(), g_sensorInfos.end());
    return ERR_OK;
}

void HdiServiceImpl::DataReportThread()
{
    HiLog::Info(LABEL, "%{public}s in", __func__);
    while (true) {
        usleep(g_samplingInterval / CONVERT_MULTIPLES);
        g_callback(&testEvent);
        if (g_isStop) {
            break;
        }
    }
    HiLog::Info(LABEL, "%{public}s thread stop", __func__);
    return;
}

int32_t HdiServiceImpl::EnableSensor(uint32_t sensorId)
{
    HiLog::Info(LABEL, "%{public}s in", __func__);
    if (g_callback == nullptr) {
        HiLog::Error(LABEL, "%{public}s enable sensor failed", __func__);
        return -1;
    }
    if (std::count(supportSensors.begin(), supportSensors.end(), sensorId) == 0) {
        HiLog::Error(LABEL, "%{public}s not support enable sensorId: %{public}d", __func__, sensorId);
        return -1;
    }
    if (std::count(g_enableSensos.begin(), g_enableSensos.end(), sensorId) != 0) {
        HiLog::Info(LABEL, "%{public}s sensorId: %{public}d has been enabled", __func__, sensorId);
        return ERR_OK;
    }
    g_enableSensos.push_back(sensorId);
    if (!dataReportThread_.joinable() || g_isStop) {
        if (dataReportThread_.joinable()) {
            dataReportThread_.join();
        }
        std::thread senocdDataThread(HdiServiceImpl::DataReportThread);
        dataReportThread_ = std::move(senocdDataThread);
        g_isStop = false;
    }
    return ERR_OK;
};

int32_t HdiServiceImpl::DisableSensor(uint32_t sensorId)
{
    HiLog::Info(LABEL, "%{public}s in", __func__);
    if (std::count(supportSensors.begin(), supportSensors.end(), sensorId) == 0) {
        HiLog::Error(LABEL, "%{public}s not support disable sensorId: %{public}d", __func__, sensorId);
        return -1;
    }
    if (std::count(g_enableSensos.begin(), g_enableSensos.end(), sensorId) == 0) {
        HiLog::Error(LABEL, "%{public}s sensorId: %{public}d should be enable first", __func__, sensorId);
        return -1;
    }
    auto iter = std::remove(g_enableSensos.begin(), g_enableSensos.end(), sensorId);
    g_enableSensos.erase(iter, g_enableSensos.end());
    if (g_enableSensos.empty()) {
        g_isStop = true;
    }
    return ERR_OK;
}

int32_t HdiServiceImpl::SetBatch(int32_t sensorId, int64_t samplingInterval, int64_t reportInterval)
{
    HiLog::Info(LABEL, "%{public}s in", __func__);
    if (samplingInterval < 0 || reportInterval < 0) {
        samplingInterval = SAMPLING_INTERVAL_NS;
        reportInterval = 0;
    }
    g_samplingInterval = samplingInterval;
    g_reportInterval = reportInterval;
    return ERR_OK;
}

int32_t HdiServiceImpl::SetMode(int32_t sensorId, int32_t mode)
{
    return ERR_OK;
}

int32_t HdiServiceImpl::RunCommand(uint32_t sensorId, int32_t cmd, int32_t params)
{
    return ERR_OK;
}

int32_t HdiServiceImpl::SetOption(int32_t sensorId, uint32_t option)
{
    return ERR_OK;
}

int32_t HdiServiceImpl::Register(RecordDataCallback cb)
{
    if (cb == nullptr) {
        HiLog::Error(LABEL, "%{public}s cb cannot be null", __func__);
        return -1;
    }
    g_callback = cb;
    return ERR_OK;
}

int32_t HdiServiceImpl::Unregister()
{
    g_isStop = true;
    return ERR_OK;
}
}  // namespace Sensors
}  // namespace OHOS