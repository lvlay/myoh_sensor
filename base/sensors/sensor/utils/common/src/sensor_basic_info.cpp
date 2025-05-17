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

#include "sensor_basic_info.h"

namespace OHOS {
namespace Sensors {
SensorBasicInfo::SensorBasicInfo() : samplingPeriodNs_(0L), maxReportDelayNs_(0L), sensorState_(false)
{}

int64_t SensorBasicInfo::GetSamplingPeriodNs() const
{
    return samplingPeriodNs_;
}

void SensorBasicInfo::SetSamplingPeriodNs(int64_t samplingPeriodNs)
{
    samplingPeriodNs_ = samplingPeriodNs;
}

int64_t SensorBasicInfo::GetMaxReportDelayNs() const
{
    return maxReportDelayNs_;
}

void SensorBasicInfo::SetMaxReportDelayNs(int64_t maxReportDelayNs)
{
    maxReportDelayNs_ = maxReportDelayNs;
}

bool SensorBasicInfo::GetSensorState() const
{
    return sensorState_;
}

void SensorBasicInfo::SetSensorState(bool sensorState)
{
    sensorState_ = sensorState;
}

bool SensorBasicInfo::GetPermState() const
{
    return permState_;
}

void SensorBasicInfo::SetPermState(bool permState)
{
    permState_ = permState;
}
} // namespace Sensors
} // namespace OHOS
