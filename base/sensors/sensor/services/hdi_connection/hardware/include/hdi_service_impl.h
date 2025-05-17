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

#ifndef HDI_SERVICE_IMPL_H
#define HDI_SERVICE_IMPL_H

#include <atomic>
#include <thread>
#include <vector>
#include "sensor_agent_type.h"
#include "singleton.h"

namespace OHOS {
namespace Sensors {
class HdiServiceImpl : public Singleton<HdiServiceImpl> {
public:
    HdiServiceImpl() = default;
    virtual ~HdiServiceImpl() {}
    int32_t GetSensorList(std::vector<SensorInfo> &sensorList);
    int32_t EnableSensor(int32_t sensorId);
    int32_t DisableSensor(int32_t sensorId);
    int32_t SetBatch(int32_t sensorId, int64_t samplingInterval, int64_t reportInterval);
    int32_t SetMode(int32_t sensorId, int32_t mode);
    int32_t Register(RecordSensorCallback cb);
    int32_t Unregister();

private:
    DISALLOW_COPY_AND_MOVE(HdiServiceImpl);
    static void DataReportThread();
    static void GenerateEvent();
    static void GenerateAccelerometerEvent();
    static void GenerateColorEvent();
    static void GenerateSarEvent();
    static void GenerateHeadPostureEvent();
    static void GenerateProximityEvent();
    static std::vector<int32_t> enableSensors_;
    std::thread dataReportThread_;
    static std::vector<RecordSensorCallback> callbacks_;
    static int64_t samplingInterval_;
    static int64_t reportInterval_;
    static std::atomic_bool isStop_;
};
} // namespace Sensors
} // namespace OHOS
#endif // HDI_SERVICE_IMPL_H