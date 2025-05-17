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

#ifndef MEDICAL_SENSOR_H
#define MEDICAL_SENSOR_H

#include <string>
#include <vector>

#include "parcel.h"

namespace OHOS {
namespace Sensors {
class MedicalSensor : public Parcelable {
public:
    MedicalSensor();
    virtual ~MedicalSensor() = default;
    uint32_t GetSensorId() const;
    void SetSensorId(uint32_t sensorId);
    std::string GetName() const;
    void SetName(const std::string &name);
    std::string GetVendor() const;
    void SetVendor(const std::string &vendor);
    uint32_t GetVersion() const;
    void SetVersion(uint32_t version);
    float GetMaxRange() const;
    void SetMaxRange(float maxRange);
    float GetResolution() const;
    void SetResolution(float resolution);
    uint32_t GetFlags() const;
    void SetFlags(uint32_t flags);
    int32_t GetFifoMaxEventCount() const;
    void SetFifoMaxEventCount(int32_t fifoMaxEventCount);
    int64_t GetMinSamplePeriodNs() const;
    void SetMinSamplePeriodNs(int64_t minSamplePeriodNs);
    int64_t GetMaxSamplePeriodNs() const;
    void SetMaxSamplePeriodNs(int64_t maxSamplePeriodNs);
    std::vector<uint32_t> GetReserved() const;
    void SetReserved(const std::vector<uint32_t> &reserved);
    bool ReadFromParcel(Parcel &parcel);
    static std::unique_ptr<MedicalSensor> Unmarshalling(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;

private:
    uint32_t sensorId_;
    std::string name_;
    std::string vendor_;
    uint32_t version_;
    float maxRange_;
    float resolution_;
    uint32_t flags_;
    int32_t fifoMaxEventCount_;
    int64_t minSamplePeriodNs_;
    int64_t maxSamplePeriodNs_;
    std::vector<uint32_t> reserved_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // MEDICAL_SENSOR_H