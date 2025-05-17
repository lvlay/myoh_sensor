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

#ifndef SENSOR_CHANNEL_INFO_H
#define SENSOR_CHANNEL_INFO_H

#include <string>
#include <vector>

namespace OHOS {
namespace Sensors {
class MedicalSensorChannelInfo {
public:
    MedicalSensorChannelInfo();
    virtual ~MedicalSensorChannelInfo() = default;
    int32_t GetUid() const;
    void SetUid(int32_t uid);
    std::string GetPackageName() const;
    void SetPackageName(const std::string &packageName);
    uint32_t GetSensorId() const;
    void SetSensorId(uint32_t sensorId);
    int64_t GetSamplingPeriodNs() const;
    void SetSamplingPeriodNs(int64_t samplingPeriodNs);
    int32_t GetFifoCount() const;
    void SetFifoCount(int32_t fifoCount);
    std::vector<int32_t> GetCmdType() const;
    void SetCmdType(const std::vector<int32_t> &cmdType);

private:
    int32_t uid_;
    std::string packageName_;
    uint32_t sensorId_;
    int64_t samplingPeriodNs_;
    uint32_t fifoCount_;
    std::vector<int32_t> cmdType_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // SENSOR_CHANNEL_INFO_H
