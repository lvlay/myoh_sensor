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

#include "medical_sensor_channel_info.h"

namespace OHOS {
namespace Sensors {
MedicalSensorChannelInfo::MedicalSensorChannelInfo() : uid_(0), sensorId_(0), samplingPeriodNs_(0), fifoCount_(0)
{}

int32_t MedicalSensorChannelInfo::GetUid() const
{
    return uid_;
}

void MedicalSensorChannelInfo::SetUid(int32_t uid)
{
    uid_ = uid;
}

std::string MedicalSensorChannelInfo::GetPackageName() const
{
    return packageName_;
}

void MedicalSensorChannelInfo::SetPackageName(const std::string &packageName)
{
    packageName_ = packageName;
}

uint32_t MedicalSensorChannelInfo::GetSensorId() const
{
    return sensorId_;
}

void MedicalSensorChannelInfo::SetSensorId(uint32_t sensorId)
{
    sensorId_ = sensorId;
}

int64_t MedicalSensorChannelInfo::GetSamplingPeriodNs() const
{
    return samplingPeriodNs_;
}

void MedicalSensorChannelInfo::SetSamplingPeriodNs(int64_t samplingPeriodNs)
{
    samplingPeriodNs_ = samplingPeriodNs;
}

int32_t MedicalSensorChannelInfo::GetFifoCount() const
{
    return fifoCount_;
}

void MedicalSensorChannelInfo::SetFifoCount(int32_t fifoCount)
{
    fifoCount_ = fifoCount;
}

std::vector<int32_t> MedicalSensorChannelInfo::GetCmdType() const
{
    return cmdType_;
}

void MedicalSensorChannelInfo::SetCmdType(const std::vector<int32_t> &cmdType)
{
    cmdType_ = cmdType;
}
}  // namespace Sensors
}  // namespace OHOS
