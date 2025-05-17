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

#include "medical_sensor_basic_info.h"

namespace OHOS {
namespace Sensors {
MedicalSensorBasicInfo::MedicalSensorBasicInfo()
    : samplingPeriodNs_(0L), maxReportDelayNs_(0L), sensorState_(SENSOR_DISABLED)
{}

int64_t MedicalSensorBasicInfo::GetSamplingPeriodNs() const
{
    return samplingPeriodNs_;
}

void MedicalSensorBasicInfo::SetSamplingPeriodNs(int64_t samplingPeriodNs)
{
    samplingPeriodNs_ = samplingPeriodNs;
}

int64_t MedicalSensorBasicInfo::GetMaxReportDelayNs() const
{
    return maxReportDelayNs_;
}

void MedicalSensorBasicInfo::SetMaxReportDelayNs(int64_t maxReportDelayNs)
{
    maxReportDelayNs_ = maxReportDelayNs;
}

MedicalSensorState MedicalSensorBasicInfo::GetSensorState() const
{
    return sensorState_;
}

void MedicalSensorBasicInfo::SetSensorState(MedicalSensorState sensorState)
{
    sensorState_ = sensorState;
}
}  // namespace Sensors
}  // namespace OHOS
