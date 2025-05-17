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
#include "hdi_service_impl.h"

#include <cmath>
#include <random>
#include <sys/prctl.h>
#include <thread>
#include <unistd.h>

#include "sensor_errors.h"

#undef LOG_TAG
#define LOG_TAG "HdiServiceImpl"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

namespace {
constexpr int64_t SAMPLING_INTERVAL_NS = 200000000;
constexpr float TARGET_SUM = 9.8F * 9.8F;
constexpr float MAX_RANGE = 9999.0F;
const std::string SENSOR_PRODUCE_THREAD_NAME = "OS_SenMock";
std::vector<SensorInfo> g_sensorInfos = {
    {"sensor_test", "default", "1.0.0", "1.0.0", 1, 1, 9999.0, 0.000001, 23.0, 100000000, 1000000000},
};
std::vector<int32_t> g_supportSensors = {
    SENSOR_TYPE_ID_ACCELEROMETER,
    SENSOR_TYPE_ID_COLOR,
    SENSOR_TYPE_ID_SAR,
    SENSOR_TYPE_ID_HEADPOSTURE,
    SENSOR_TYPE_ID_PROXIMITY1
};
float g_accData[3];
float g_colorData[2];
float g_sarData[1];
float g_headPostureData[5];
float g_proximityData[1];
SensorEvent g_accEvent = {
    .sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER,
    .option = 3,
    .dataLen = 12
};
SensorEvent g_colorEvent = {
    .sensorTypeId = SENSOR_TYPE_ID_COLOR,
    .option = 3,
    .dataLen = 8
};
SensorEvent g_sarEvent = {
    .sensorTypeId = SENSOR_TYPE_ID_SAR,
    .option = 3,
    .dataLen = 4
};
SensorEvent g_headPostureEvent = {
    .sensorTypeId = SENSOR_TYPE_ID_HEADPOSTURE,
    .option = 3,
    .dataLen = 20
};
SensorEvent g_proximityEvent = {
    .sensorTypeId = SENSOR_TYPE_ID_PROXIMITY1,
    .option = 3,
    .dataLen = 4
};
} // namespace
std::vector<int32_t> HdiServiceImpl::enableSensors_;
std::vector<RecordSensorCallback> HdiServiceImpl::callbacks_;
int64_t HdiServiceImpl::samplingInterval_ = -1;
int64_t HdiServiceImpl::reportInterval_ = -1;
std::atomic_bool HdiServiceImpl::isStop_ = false;

void HdiServiceImpl::GenerateEvent()
{
    for (const auto &sensorId : enableSensors_) {
        switch (sensorId) {
            case SENSOR_TYPE_ID_ACCELEROMETER:
                GenerateAccelerometerEvent();
                break;
            case SENSOR_TYPE_ID_COLOR:
                GenerateColorEvent();
                break;
            case SENSOR_TYPE_ID_SAR:
                GenerateSarEvent();
                break;
            case SENSOR_TYPE_ID_HEADPOSTURE:
                GenerateHeadPostureEvent();
                break;
            case SENSOR_TYPE_ID_PROXIMITY1:
                GenerateProximityEvent();
                break;
            default:
                SEN_HILOGW("Unknown sensorId:%{public}d", sensorId);
                break;
        }
    }
}

void HdiServiceImpl::GenerateAccelerometerEvent()
{
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<float> distr(0.0, TARGET_SUM);
    float num1 = 0.0;
    float num2 = 0.0;
    while (true) {
        num1 = distr(eng);
        num2 = distr(eng);
        if ((num1 > num2) && (std::fabs(num1 - num2) > std::numeric_limits<float>::epsilon())) {
            float temp = num1;
            num1 = num2;
            num2 = temp;
            break;
        }
    }
    g_accData[0] = static_cast<float>(sqrt(num1));
    g_accData[1] = static_cast<float>(sqrt(num2 - num1));
    g_accData[2] = static_cast<float>(sqrt(TARGET_SUM - num2));
    g_accEvent.data = reinterpret_cast<uint8_t *>(g_accData);
}

void HdiServiceImpl::GenerateColorEvent()
{
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<float> distr(0.0, MAX_RANGE);
    g_colorData[0] = distr(eng);
    g_colorData[1] = distr(eng);
    g_colorEvent.data = reinterpret_cast<uint8_t *>(g_colorData);
}

void HdiServiceImpl::GenerateSarEvent()
{
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<float> distr(0.0, MAX_RANGE);
    g_sarData[0] = distr(eng);
    g_sarEvent.data = reinterpret_cast<uint8_t *>(g_sarData);
}

void HdiServiceImpl::GenerateHeadPostureEvent()
{
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<float> distr(0.0, 1.0);
    std::vector<float> nums(4);
    while (true) {
        nums[0] = distr(eng);
        nums[1] = distr(eng);
        nums[2] = distr(eng);
        nums[3] = distr(eng);
        sort(nums.begin(), nums.end());
        if ((std::fabs(nums[1] - nums[0]) > std::numeric_limits<float>::epsilon()) &&
            (std::fabs(nums[2] - nums[1]) > std::numeric_limits<float>::epsilon()) &&
            (std::fabs(nums[3] - nums[2]) > std::numeric_limits<float>::epsilon())) {
            break;
        }
    }
    g_headPostureData[0] = static_cast<float>(sqrt(nums[0]));
    g_headPostureData[1] = static_cast<float>(sqrt(nums[1] - nums[0]));
    g_headPostureData[2] = static_cast<float>(sqrt(nums[2] - nums[1]));
    g_headPostureData[3] = static_cast<float>(sqrt(nums[3] - nums[2]));
    g_headPostureData[4] = static_cast<float>(sqrt(1.0 - nums[3]));
    g_headPostureEvent.data = reinterpret_cast<uint8_t *>(g_headPostureData);
}

void HdiServiceImpl::GenerateProximityEvent()
{
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<float> distr(0.0, MAX_RANGE);
    g_proximityData[0] = distr(eng);
    g_proximityEvent.data = reinterpret_cast<uint8_t *>(g_proximityData);
}

int32_t HdiServiceImpl::GetSensorList(std::vector<SensorInfo> &sensorList)
{
    CALL_LOG_ENTER;
    sensorList.assign(g_sensorInfos.begin(), g_sensorInfos.end());
    return ERR_OK;
}

void HdiServiceImpl::DataReportThread()
{
    CALL_LOG_ENTER;
    prctl(PR_SET_NAME, SENSOR_PRODUCE_THREAD_NAME.c_str());
    while (true) {
        GenerateEvent();
        std::this_thread::sleep_for(std::chrono::nanoseconds(samplingInterval_));
        for (const auto &it : callbacks_) {
            if (it == nullptr) {
                SEN_HILOGW("RecordSensorCallback is null");
                continue;
            }
            for (const auto &sensorId : enableSensors_) {
                switch (sensorId) {
                    case SENSOR_TYPE_ID_ACCELEROMETER:
                        it(&g_accEvent);
                        break;
                    case SENSOR_TYPE_ID_COLOR:
                        it(&g_colorEvent);
                        break;
                    case SENSOR_TYPE_ID_SAR:
                        it(&g_sarEvent);
                        break;
                    case SENSOR_TYPE_ID_HEADPOSTURE:
                        it(&g_headPostureEvent);
                        break;
                    case SENSOR_TYPE_ID_PROXIMITY1:
                        it(&g_proximityEvent);
                        break;
                    default:
                        SEN_HILOGW("Unknown sensorId:%{public}d", sensorId);
                        break;
                }
            }
        }
        if (isStop_) {
            break;
        }
    }
    SEN_HILOGI("Thread stop");
    return;
}

int32_t HdiServiceImpl::EnableSensor(int32_t sensorId)
{
    CALL_LOG_ENTER;
    if (std::find(g_supportSensors.begin(), g_supportSensors.end(), sensorId) == g_supportSensors.end()) {
        SEN_HILOGE("Not support enable sensorId:%{public}d", sensorId);
        return ERR_NO_INIT;
    }
    if (std::find(enableSensors_.begin(), enableSensors_.end(), sensorId) != enableSensors_.end()) {
        SEN_HILOGI("sensorId:%{public}d has been enabled", sensorId);
        return ERR_OK;
    }
    enableSensors_.push_back(sensorId);
    if (!dataReportThread_.joinable() || isStop_) {
        if (dataReportThread_.joinable()) {
            dataReportThread_.join();
        }
        std::thread senocdDataThread(HdiServiceImpl::DataReportThread);
        dataReportThread_ = std::move(senocdDataThread);
        isStop_ = false;
    }
    return ERR_OK;
};

int32_t HdiServiceImpl::DisableSensor(int32_t sensorId)
{
    CALL_LOG_ENTER;
    if (std::find(g_supportSensors.begin(), g_supportSensors.end(), sensorId) == g_supportSensors.end()) {
        SEN_HILOGE("Not support disable sensorId:%{public}d", sensorId);
        return ERR_NO_INIT;
    }
    if (std::find(enableSensors_.begin(), enableSensors_.end(), sensorId) == enableSensors_.end()) {
        SEN_HILOGE("sensorId:%{public}d should be enable first", sensorId);
        return ERR_NO_INIT;
    }
    std::vector<int32_t>::iterator iter;
    for (iter = enableSensors_.begin(); iter != enableSensors_.end();) {
        if (*iter == sensorId) {
            iter = enableSensors_.erase(iter);
            break;
        } else {
            ++iter;
        }
    }
    if (enableSensors_.empty()) {
        isStop_ = true;
    }
    return ERR_OK;
}

int32_t HdiServiceImpl::SetBatch(int32_t sensorId, int64_t samplingInterval, int64_t reportInterval)
{
    CALL_LOG_ENTER;
    if (samplingInterval < 0 || reportInterval < 0) {
        samplingInterval = SAMPLING_INTERVAL_NS;
        reportInterval = 0;
    }
    samplingInterval_ = samplingInterval;
    reportInterval_ = reportInterval;
    return ERR_OK;
}

int32_t HdiServiceImpl::SetMode(int32_t sensorId, int32_t mode)
{
    return ERR_OK;
}

int32_t HdiServiceImpl::Register(RecordSensorCallback cb)
{
    CHKPR(cb, ERROR);
    callbacks_.push_back(cb);
    return ERR_OK;
}

int32_t HdiServiceImpl::Unregister()
{
    isStop_ = true;
    return ERR_OK;
}
} // namespace Sensors
} // namespace OHOS