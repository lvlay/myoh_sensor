/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "print_sensor_data.h"

#include <string>
#include <vector>

#include "sensor_errors.h"

#undef LOG_TAG
#define LOG_TAG "PrintSensorData"

namespace OHOS {
namespace Sensors {
namespace {
enum {
    ONE_DIMENSION = 1,
    TWO_DIMENSION = 2,
    THREE_DIMENSION = 3,
    SEVEN_DIMENSION = 7,
    DEFAULT_DIMENSION = 16
};
constexpr int64_t LOG_INTERVAL = 60000000000L;
constexpr int32_t FIRST_PRINT_TIMES = 20;
constexpr float LOG_FORMAT_DIVIDER = 1e9f;

const std::vector<int32_t> g_triggerSensorType = {
    SENSOR_TYPE_ID_HALL_EXT,
    SENSOR_TYPE_ID_PROXIMITY,
    SENSOR_TYPE_ID_HALL,
    SENSOR_TYPE_ID_WEAR_DETECTION,
};
const std::vector<int32_t> g_continuousSensorType = {
    SENSOR_TYPE_ID_ACCELEROMETER,
    SENSOR_TYPE_ID_POSTURE,
    SENSOR_TYPE_ID_AMBIENT_LIGHT,
    SENSOR_TYPE_ID_AMBIENT_LIGHT1,
    SENSOR_TYPE_ID_GYROSCOPE,
    SENSOR_TYPE_ID_MAGNETIC_FIELD,
};
}

void PrintSensorData::ControlSensorHdiPrint(const SensorData &sensorData)
{
    auto triggerIt = std::find(g_triggerSensorType.begin(), g_triggerSensorType.end(), sensorData.sensorTypeId);
    if (triggerIt != g_triggerSensorType.end()) {
        PrintHdiData(sensorData);
    }
    std::lock_guard<std::mutex> hdiLoginfoLock(hdiLoginfoMutex_);
    auto it = hdiLoginfo_.find(sensorData.sensorTypeId);
    if (it == hdiLoginfo_.end()) {
        return;
    }
    if (it->second.count < FIRST_PRINT_TIMES) {
        PrintHdiData(sensorData);
        if (it->second.count == FIRST_PRINT_TIMES - 1) {
            it->second.lastTime = sensorData.timestamp;
        }
        it->second.count++;
    } else {
        if (sensorData.timestamp - it->second.lastTime >= LOG_INTERVAL) {
            PrintHdiData(sensorData);
            it->second.lastTime = sensorData.timestamp;
        }
    }
}

void PrintSensorData::PrintHdiData(const SensorData &sensorData)
{
    std::string str;
    str += "sensorId: " + std::to_string(sensorData.sensorTypeId) + ", ";
    str += "timestamp: " + std::to_string(sensorData.timestamp / LOG_FORMAT_DIVIDER) + ", ";
    int32_t dataDim = GetDataDimension(sensorData.sensorTypeId);
    auto data = reinterpret_cast<const float *>(sensorData.data);
    CHKPV(data);
    for (int32_t i = 0; i < dataDim; ++i) {
        str.append(std::to_string(*data));
        if (i != dataDim - 1) {
            str.append(", ");
        }
        ++data;
    }
    str.append("\n");
    SEN_HILOGI("SensorData: %{public}s", str.c_str());
}

int32_t PrintSensorData::GetDataDimension(int32_t sensorId)
{
    switch (sensorId) {
        case SENSOR_TYPE_ID_HALL:
        case SENSOR_TYPE_ID_PROXIMITY:
        case SENSOR_TYPE_ID_WEAR_DETECTION:
            return ONE_DIMENSION;
        case SENSOR_TYPE_ID_HALL_EXT:
            return TWO_DIMENSION;
        case SENSOR_TYPE_ID_POSTURE:
            return SEVEN_DIMENSION;
        case SENSOR_TYPE_ID_AMBIENT_LIGHT:
        case SENSOR_TYPE_ID_AMBIENT_LIGHT1:
        case SENSOR_TYPE_ID_ACCELEROMETER:
        case SENSOR_TYPE_ID_GYROSCOPE:
        case SENSOR_TYPE_ID_MAGNETIC_FIELD:
            return THREE_DIMENSION;
        default:
            SEN_HILOGW("Unknown sensorId:%{public}d, size:%{public}d", sensorId, DEFAULT_DIMENSION);
            return DEFAULT_DIMENSION;
    }
}

void PrintSensorData::ControlSensorClientPrint(const RecordSensorCallback callback, const SensorEvent &event)
{
    auto triggerIt = std::find(g_triggerSensorType.begin(), g_triggerSensorType.end(), event.sensorTypeId);
    if (triggerIt != g_triggerSensorType.end()) {
        PrintClientData(event);
    }

    auto continuosIt = std::find(g_continuousSensorType.begin(), g_continuousSensorType.end(), event.sensorTypeId);
    if (continuosIt == g_continuousSensorType.end()) {
        return;
    }
    std::lock_guard<std::mutex> clientLoginfoLock(clientLoginfoMutex_);
    auto it = clientLoginfo_.find(callback);
    if (it == clientLoginfo_.end()) {
        return;
    }
    if (it->second.count < FIRST_PRINT_TIMES) {
        PrintClientData(event);
        if (it->second.count == FIRST_PRINT_TIMES - 1) {
            it->second.lastTime = event.timestamp;
        }
        it->second.count++;
    } else {
        if (event.timestamp - it->second.lastTime >= LOG_INTERVAL) {
            PrintClientData(event);
            it->second.lastTime = event.timestamp;
        }
    }
}

void PrintSensorData::PrintClientData(const SensorEvent &event)
{
    std::string str;
    str += "sensorId: " + std::to_string(event.sensorTypeId) + ", ";
    str += "timestamp: " + std::to_string(event.timestamp / LOG_FORMAT_DIVIDER) + ", ";
    int32_t dataDim = GetDataDimension(event.sensorTypeId);
    auto data = reinterpret_cast<const float *>(event.data);
    CHKPV(data);
    for (int32_t i = 0; i < dataDim; ++i) {
        str.append(std::to_string(*data));
        if (i != dataDim - 1) {
            str.append(", ");
        }
        ++data;
    }
    str.append("\n");
    SEN_HILOGI("SensorData: %{public}s", str.c_str());
}

bool PrintSensorData::IsContinuousType(int32_t sensorId)
{
    return std::find(g_continuousSensorType.begin(), g_continuousSensorType.end(),
        sensorId) != g_continuousSensorType.end();
}

void PrintSensorData::SavePrintUserInfo(const RecordSensorCallback callback)
{
    CHKPV(callback);
    std::lock_guard<std::mutex> clientLoginfoLock(clientLoginfoMutex_);
    if (clientLoginfo_.find(callback) != clientLoginfo_.end()) {
        return;
    }
    LogPrintInfo info;
    auto status = clientLoginfo_.insert(std::make_pair(callback, info));
    if (!status.second) {
        SEN_HILOGD("callback has been saved");
    }
}

void PrintSensorData::RemovePrintUserInfo(const RecordSensorCallback callback)
{
    CHKPV(callback);
    std::lock_guard<std::mutex> clientLoginfoLock(clientLoginfoMutex_);
    if (clientLoginfo_.find(callback) == clientLoginfo_.end()) {
        return;
    }
    clientLoginfo_.erase(callback);
}

void PrintSensorData::ResetHdiCounter(int32_t sensorId)
{
    std::lock_guard<std::mutex> hdiLoginfoLock(hdiLoginfoMutex_);
    auto it = hdiLoginfo_.find(sensorId);
    if (it == hdiLoginfo_.end()) {
        return;
    }
    it->second.count = 0;
    it->second.lastTime = 0;
}
} // namespace Sensors
} // namespace OHOS
