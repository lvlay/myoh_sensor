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

#ifndef PRINT_SENSOR_DATA
#define PRINT_SENSOR_DATA

#include <map>

#include "singleton.h"

#include "sensor_agent_type.h"
#include "sensor_data_event.h"

namespace OHOS {
namespace Sensors {


class PrintSensorData : public Singleton<PrintSensorData> {
public:
    PrintSensorData() = default;
    virtual ~PrintSensorData() {};
    void ControlSensorClientPrint(const RecordSensorCallback callback, const SensorEvent &event);
    void ControlSensorHdiPrint(const SensorData &sensorData);
    void ResetHdiCounter(int32_t sensorId);
    bool IsContinuousType(int32_t sensorId);
    void SavePrintUserInfo(const RecordSensorCallback callback);
    void RemovePrintUserInfo(const RecordSensorCallback callback);

private:
    void PrintClientData(const SensorEvent &event);
    void PrintHdiData(const SensorData &sensorData);
    int32_t GetDataDimension(int32_t sensorId);
    struct LogPrintInfo {
        int32_t count { 0 };
        int64_t lastTime { 0 };
    };
    std::mutex hdiLoginfoMutex_;
    std::mutex clientLoginfoMutex_;
    LogPrintInfo info_;
    std::map<int32_t, LogPrintInfo> hdiLoginfo_ = {
        {SENSOR_TYPE_ID_ACCELEROMETER, info_},
        {SENSOR_TYPE_ID_GYROSCOPE, info_},
        {SENSOR_TYPE_ID_POSTURE, info_},
        {SENSOR_TYPE_ID_AMBIENT_LIGHT, info_},
        {SENSOR_TYPE_ID_AMBIENT_LIGHT1, info_},
        {SENSOR_TYPE_ID_MAGNETIC_FIELD, info_},
    };
    std::map<RecordSensorCallback, LogPrintInfo> clientLoginfo_;
};
} // namespace Sensors
} // namespace OHOS
#endif // PRINT_SENSOR_DATA
