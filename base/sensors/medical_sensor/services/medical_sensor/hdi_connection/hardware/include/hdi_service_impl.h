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

#ifndef HDI_SERVICE_IMPL_H
#define HDI_SERVICE_IMPL_H

#include <atomic>
#include <thread>
#include <vector>
#include "medical_native_type.h"
#include "sensor_if.h"
#include "singleton.h"

namespace OHOS {
namespace Sensors {
class HdiServiceImpl : public Singleton<HdiServiceImpl> {
public:
    HdiServiceImpl() = default;

    virtual ~HdiServiceImpl() {}

    int32_t GetSensorList(std::vector<SensorInformation>& sensorList);

    int32_t EnableSensor(uint32_t sensorId);

    int32_t DisableSensor(uint32_t sensorId);

    int32_t SetBatch(int32_t sensorId, int64_t samplingInterval, int64_t reportInterval);

    int32_t SetMode(int32_t sensorId, int32_t mode);

    int32_t SetOption(int32_t sensorId, uint32_t option);

    int32_t RunCommand(uint32_t sensorId, int32_t cmd, int32_t params);

    int32_t Register(RecordDataCallback cb);

    int32_t Unregister();

private:
    DISALLOW_COPY_AND_MOVE(HdiServiceImpl);
    static void DataReportThread();
    std::vector<int32_t> g_enableSensos;
    std::thread dataReportThread_;
    static RecordDataCallback g_callback;
    static int64_t g_samplingInterval;
    static int64_t g_reportInterval;
    static std::atomic_bool g_isStop;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // HDI_SERVICE_IMPL_H