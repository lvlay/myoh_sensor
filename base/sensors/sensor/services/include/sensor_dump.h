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

#ifndef SENSOR_DUMP_H
#define SENSOR_DUMP_H

#include <vector>

#include "singleton.h"

#include "nocopyable.h"

#include "client_info.h"
#include "sensor.h"
#include "sensor_agent_type.h"

namespace OHOS {
namespace Sensors {
class SensorDump : public Singleton<SensorDump> {
public:
    SensorDump() = default;
    virtual ~SensorDump() = default;
    void ParseCommand(int32_t fd, const std::vector<std::string> &args, const std::vector<Sensor> &sensors,
        ClientInfo &clientInfo);
    void DumpHelp(int32_t fd);
    bool DumpSensorList(int32_t fd, const std::vector<Sensor> &sensors);
    bool DumpSensorChannel(int32_t fd, ClientInfo &clientInfo);
    bool DumpOpeningSensor(int32_t fd, const std::vector<Sensor> &sensors, ClientInfo &clientInfo);
    bool DumpSensorData(int32_t fd, ClientInfo &clientInfo);

private:
    DISALLOW_COPY_AND_MOVE(SensorDump);
    void DumpCurrentTime(int32_t fd);
    int32_t GetDataDimension(int32_t sensorId);
    std::string GetDataBySensorId(int32_t sensorId, SensorData &sensorData);
    static std::unordered_map<int32_t, std::string> sensorMap_;
    void RunSensorDump(int32_t fd, int32_t optionIndex, const std::vector<std::string> &args, char **argv);
    std::vector<Sensor> sensors_;
    ClientInfo &clientInfo_ = ClientInfo::GetInstance();
};
} // namespace Sensors
} // namespace OHOS
#endif // SENSOR_DUMP_H
