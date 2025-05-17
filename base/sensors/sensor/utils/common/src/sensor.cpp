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

#include "sensor.h"

#include "sensor_errors.h"

#undef LOG_TAG
#define LOG_TAG "Sensor"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

Sensor::Sensor()
    : sensorId_(0),
      sensorTypeId_(0),
      sensorName_(""),
      vendorName_(""),
      firmwareVersion_(""),
      hardwareVersion_(""),
      maxRange_(0.0),
      resolution_(0.0),
      power_(0.0),
      flags_(0),
      fifoMaxEventCount_(0),
      minSamplePeriodNs_(0),
      maxSamplePeriodNs_(0)
{}

int32_t Sensor::GetSensorId() const
{
    return sensorId_;
}

void Sensor::SetSensorId(int32_t sensorId)
{
    sensorId_ = sensorId;
}

int32_t Sensor::GetSensorTypeId() const
{
    return sensorTypeId_;
}

void Sensor::SetSensorTypeId(int32_t sensorTypeId)
{
    sensorTypeId_ = sensorTypeId;
}

std::string Sensor::GetSensorName() const
{
    return sensorName_;
}

void Sensor::SetSensorName(const std::string &sensorName)
{
    sensorName_ = sensorName;
}

std::string Sensor::GetVendorName() const
{
    return vendorName_;
}

void Sensor::SetVendorName(const std::string &vendorName)
{
    vendorName_ = vendorName;
}

std::string Sensor::GetHardwareVersion() const
{
    return hardwareVersion_;
}

void Sensor::SetHardwareVersion(const std::string &hardwareVersion)
{
    hardwareVersion_ = hardwareVersion;
}

std::string Sensor::GetFirmwareVersion() const
{
    return firmwareVersion_;
}

void Sensor::SetFirmwareVersion(const std::string &firmwareVersion)
{
    firmwareVersion_ = firmwareVersion;
}

float Sensor::GetMaxRange() const
{
    return maxRange_;
}

void Sensor::SetMaxRange(float maxRange)
{
    maxRange_ = maxRange;
}

float Sensor::GetResolution() const
{
    return resolution_;
}

void Sensor::SetResolution(float resolution)
{
    resolution_ = resolution;
}

float Sensor::GetPower() const
{
    return power_;
}

void Sensor::SetPower(float power)
{
    power_ = power;
}

uint32_t Sensor::Sensor::GetFlags() const
{
    return flags_;
}

void Sensor::SetFlags(uint32_t flags)
{
    flags_ = flags;
}

int32_t Sensor::GetFifoMaxEventCount() const
{
    return fifoMaxEventCount_;
}

void Sensor::SetFifoMaxEventCount(int32_t fifoMaxEventCount)
{
    fifoMaxEventCount_ = fifoMaxEventCount;
}

int64_t Sensor::GetMinSamplePeriodNs() const
{
    return minSamplePeriodNs_;
}

void Sensor::SetMinSamplePeriodNs(int64_t minSamplePeriodNs)
{
    minSamplePeriodNs_ = minSamplePeriodNs;
}

int64_t Sensor::GetMaxSamplePeriodNs() const
{
    return maxSamplePeriodNs_;
}

void Sensor::SetMaxSamplePeriodNs(int64_t maxSamplePeriodNs)
{
    maxSamplePeriodNs_ = maxSamplePeriodNs;
}

bool Sensor::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(sensorId_)) {
        SEN_HILOGE("Failed, write sensorId failed");
        return false;
    }
    if (!parcel.WriteInt32(sensorTypeId_)) {
        SEN_HILOGE("Failed, write sensorTypeId failed");
        return false;
    }
    if (!parcel.WriteString(sensorName_)) {
        SEN_HILOGE("Failed, write sensorName failed");
        return false;
    }
    if (!parcel.WriteString(vendorName_)) {
        SEN_HILOGE("Failed, write vendorName failed");
        return false;
    }
    if (!parcel.WriteString(firmwareVersion_)) {
        SEN_HILOGE("Failed, write firmwareVersion failed");
        return false;
    }
    if (!parcel.WriteString(hardwareVersion_)) {
        SEN_HILOGE("Failed, write hardwareVersion failed");
        return false;
    }
    if (!parcel.WriteFloat(maxRange_)) {
        SEN_HILOGE("Failed, write maxRange failed");
        return false;
    }
    if (!parcel.WriteFloat(resolution_)) {
        SEN_HILOGE("Failed, write resolution failed");
        return false;
    }
    if (!parcel.WriteFloat(power_)) {
        SEN_HILOGE("Failed, write power failed");
        return false;
    }
    if (!parcel.WriteUint32(flags_)) {
        SEN_HILOGE("Failed, write flags failed");
        return false;
    }
    if (!parcel.WriteInt32(fifoMaxEventCount_)) {
        SEN_HILOGE("Failed, write fifoMaxEventCount failed");
        return false;
    }
    if (!parcel.WriteInt64(minSamplePeriodNs_)) {
        SEN_HILOGE("Failed, write minSamplePeriodNs failed");
        return false;
    }
    if (!parcel.WriteInt64(maxSamplePeriodNs_)) {
        SEN_HILOGE("Failed, write maxSamplePeriodNs failed");
        return false;
    }
    return true;
}

std::unique_ptr<Sensor> Sensor::Unmarshalling(Parcel &parcel)
{
    auto sensor = std::make_unique<Sensor>();
    if (!sensor->ReadFromParcel(parcel)) {
        SEN_HILOGE("ReadFromParcel is failed");
        return nullptr;
    }
    return sensor;
}

bool Sensor::ReadFromParcel(Parcel &parcel)
{
    if ((!parcel.ReadInt32(sensorId_)) ||
        (!parcel.ReadInt32(sensorTypeId_)) ||
        (!parcel.ReadString(sensorName_)) ||
        (!parcel.ReadString(vendorName_)) ||
        (!parcel.ReadString(firmwareVersion_)) ||
        (!parcel.ReadString(hardwareVersion_)) ||
        (!parcel.ReadFloat(power_)) ||
        (!parcel.ReadFloat(maxRange_)) ||
        (!parcel.ReadFloat(resolution_)) ||
        (!parcel.ReadUint32(flags_)) ||
        (!parcel.ReadInt32(fifoMaxEventCount_)) ||
        (!parcel.ReadInt64(minSamplePeriodNs_)) ||
        (!parcel.ReadInt64(maxSamplePeriodNs_))) {
        return false;
    }
    return true;
}
} // namespace Sensors
} // namespace OHOS
