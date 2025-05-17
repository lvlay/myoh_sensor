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

#ifndef SENSOR_DUMP_H
#define SENSOR_DUMP_H

#include <vector>

#include "singleton.h"

#include "client_info.h"
#include "medical_sensor.h"
#include "nocopyable.h"
#include "medical_native_type.h"

namespace OHOS {
namespace Sensors {
class MedicalSensorDump : public Singleton<MedicalSensorDump> {
public:
    MedicalSensorDump() = default;
    virtual ~MedicalSensorDump() = default;
    void DumpHelp(int32_t fd);
    bool DumpSensorHelp(int32_t fd, const std::vector<std::u16string> &args);
    bool DumpSensorList(int32_t fd, const std::vector<MedicalSensor> &sensors, const std::vector<std::u16string> &args);
    bool DumpSensorChannel(int32_t fd, ClientInfo &clientInfo, const std::vector<std::u16string> &args);
    bool DumpOpeningSensor(int32_t fd, const std::vector<MedicalSensor> &sensors, ClientInfo &clientInfo,
                           const std::vector<std::u16string> &args);
    bool DumpSensorData(int32_t fd, ClientInfo &clientInfo, const std::vector<std::u16string> &args);
    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args,
                  std::vector<MedicalSensor> &sensors, ClientInfo &clientInfo);

private:
    DISALLOW_COPY_AND_MOVE(MedicalSensorDump);
    void DumpCurrentTime(int32_t fd);
    int32_t DataSizeBySensorId(uint32_t sensorId);
    std::string GetDataBySensorId(uint32_t sensorId, struct SensorEvent &sensorData);
    static std::unordered_map<uint32_t, std::string> sensorMap_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // SENSOR_DUMP_H
