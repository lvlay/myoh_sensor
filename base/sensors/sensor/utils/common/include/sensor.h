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

#ifndef SENSOR_H
#define SENSOR_H

#include <string>
#include <vector>

#include "parcel.h"

namespace OHOS {
namespace Sensors {
class Sensor : public Parcelable {
public:
    Sensor();
    virtual ~Sensor() = default;
    int32_t GetSensorId() const;
    void SetSensorId(int32_t sensorId);
    int32_t GetSensorTypeId() const;
    void SetSensorTypeId(int32_t sensorTypeId);
    std::string GetSensorName() const;
    void SetSensorName(const std::string &sensorName);
    std::string GetVendorName() const;
    void SetVendorName(const std::string &vendorName);
    std::string GetHardwareVersion() const;
    void SetHardwareVersion(const std::string &hardwareVersion);
    std::string GetFirmwareVersion() const;
    void SetFirmwareVersion(const std::string &firmwareVersion);
    float GetMaxRange() const;
    void SetMaxRange(float maxRange);
    float GetResolution() const;
    void SetResolution(float resolution);
    float GetPower() const;
    void SetPower(float power);
    uint32_t GetFlags() const;
    void SetFlags(uint32_t flags);
    int32_t GetFifoMaxEventCount() const;
    void SetFifoMaxEventCount(int32_t fifoMaxEventCount);
    int64_t GetMinSamplePeriodNs() const;
    void SetMinSamplePeriodNs(int64_t minSamplePeriodNs);
    int64_t GetMaxSamplePeriodNs() const;
    void SetMaxSamplePeriodNs(int64_t maxSamplePeriodNs);
    bool ReadFromParcel(Parcel &parcel);
    static std::unique_ptr<Sensor> Unmarshalling(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;

private:
    int32_t sensorId_;
    int32_t sensorTypeId_;
    std::string sensorName_;
    std::string vendorName_;
    std::string firmwareVersion_;
    std::string hardwareVersion_;
    float maxRange_;
    float resolution_;
    float power_;
    uint32_t flags_;
    int32_t fifoMaxEventCount_;
    int64_t minSamplePeriodNs_;
    int64_t maxSamplePeriodNs_;
};
} // namespace Sensors
} // namespace OHOS
#endif // SENSOR_H
