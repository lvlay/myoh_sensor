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

#ifndef SENSOR_BASIC_INFO_H
#define SENSOR_BASIC_INFO_H

#include <cstdint>

namespace OHOS {
namespace Sensors {
class SensorBasicInfo {
public:
    SensorBasicInfo();
    virtual ~SensorBasicInfo() = default;
    int64_t GetSamplingPeriodNs() const;
    void SetSamplingPeriodNs(int64_t samplingPeriodNs);
    int64_t GetMaxReportDelayNs() const;
    void SetMaxReportDelayNs(int64_t maxReportDelayNs);
    bool GetSensorState() const;
    void SetSensorState(bool sensorState);
    bool GetPermState() const;
    void SetPermState(bool permState);

private:
    int64_t samplingPeriodNs_;
    int64_t maxReportDelayNs_;
    bool sensorState_ = false;
    bool permState_ = true;
};
} // namespace Sensors
} // namespace OHOS
#endif // SENSOR_BASIC_INFO_H
