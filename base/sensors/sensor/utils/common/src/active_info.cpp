/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "active_info.h"

#include "sensor_errors.h"

#undef LOG_TAG
#define LOG_TAG "ActiveInfo"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

ActiveInfo::ActiveInfo(int32_t pid, int32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs)
    :pid_(pid), sensorId_(sensorId), samplingPeriodNs_(samplingPeriodNs), maxReportDelayNs_(maxReportDelayNs)
{}

int32_t ActiveInfo::GetPid() const
{
    return pid_;
}

void ActiveInfo::SetPid(int32_t pid)
{
    pid_ = pid;
}

int32_t ActiveInfo::GetSensorId() const
{
    return sensorId_;
}

void ActiveInfo::SetSensorId(int32_t sensorId)
{
    sensorId_ = sensorId;
}

int64_t ActiveInfo::GetSamplingPeriodNs() const
{
    return samplingPeriodNs_;
}

void ActiveInfo::SetSamplingPeriodNs(int64_t samplingPeriodNs)
{
    samplingPeriodNs_ = samplingPeriodNs;
}

int64_t ActiveInfo::GetMaxReportDelayNs() const
{
    return maxReportDelayNs_;
}

void ActiveInfo::SetMaxReportDelayNs(int64_t maxReportDelayNs)
{
    maxReportDelayNs_ = maxReportDelayNs;
}

bool ActiveInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(pid_)) {
        SEN_HILOGE("Write pid failed");
        return false;
    }
    if (!parcel.WriteInt32(sensorId_)) {
        SEN_HILOGE("Write sensorId failed");
        return false;
    }
    if (!parcel.WriteInt64(samplingPeriodNs_)) {
        SEN_HILOGE("Write samplingPeriodNs failed");
        return false;
    }
    if (!parcel.WriteInt64(maxReportDelayNs_)) {
        SEN_HILOGE("Write maxReportDelayNs failed");
        return false;
    }
    return true;
}

std::unique_ptr<ActiveInfo> ActiveInfo::Unmarshalling(Parcel &parcel)
{
    int32_t pid = -1;
    int32_t sensorId = -1;
    int64_t samplingPeriodNs = -1;
    int64_t maxReportDelayNs = -1;
    if (!(parcel.ReadInt32(pid) && parcel.ReadInt32(sensorId) &&
          parcel.ReadInt64(samplingPeriodNs) && parcel.ReadInt64(maxReportDelayNs))) {
        SEN_HILOGE("Read from parcel is failed");
        return nullptr;
    }
    auto activeInfo = std::make_unique<ActiveInfo>();
    activeInfo->SetPid(pid);
    activeInfo->SetSensorId(sensorId);
    activeInfo->SetSamplingPeriodNs(samplingPeriodNs);
    activeInfo->SetMaxReportDelayNs(maxReportDelayNs);
    return activeInfo;
}
} // namespace Sensors
} // namespace OHOS