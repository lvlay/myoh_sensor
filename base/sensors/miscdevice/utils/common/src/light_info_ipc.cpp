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

#include "light_info_ipc.h"

#include "sensors_errors.h"

#undef LOG_TAG
#define LOG_TAG "LightInfoIPC"

namespace OHOS {
namespace Sensors {
std::string LightInfoIPC::GetLightName() const
{
    return lightName_;
}

void LightInfoIPC::SetLightName(const std::string &lightName)
{
    lightName_ = lightName;
}

int32_t LightInfoIPC::GetLightId() const
{
    return lightId_;
}
    
void LightInfoIPC::SetLightId(int32_t lightId)
{
    lightId_ = lightId;
}

int32_t LightInfoIPC::GetLightNumber() const
{
    return lightNumber_;
}

void LightInfoIPC::SetLightNumber(int32_t lightNumber)
{
    lightNumber_ = lightNumber;
}
    
int32_t LightInfoIPC::GetLightType() const
{
    return lightType_;
}

void LightInfoIPC::SetLightType(int32_t lightType)
{
    lightType_ = lightType;
}

bool LightInfoIPC::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(lightName_)) {
        MISC_HILOGE("Failed, write lightName failed");
        return false;
    }
    if (!parcel.WriteInt32(lightId_)) {
        MISC_HILOGE("Failed, write lightId failed");
        return false;
    }
    if (!parcel.WriteInt32(lightNumber_)) {
        MISC_HILOGE("Failed, write lightNumber failed");
        return false;
    }
    if (!parcel.WriteInt32(lightType_)) {
        MISC_HILOGE("Failed, write lightType failed");
        return false;
    }
    return true;
}

std::unique_ptr<LightInfoIPC> LightInfoIPC::Unmarshalling(Parcel &parcel)
{
    auto lightInfo = std::make_unique<LightInfoIPC>();
    if (!lightInfo->ReadFromParcel(parcel)) {
        MISC_HILOGE("ReadFromParcel is failed");
        return nullptr;
    }
    return lightInfo;
}

bool LightInfoIPC::ReadFromParcel(Parcel &parcel)
{
    return (parcel.ReadString(lightName_)) && (parcel.ReadInt32(lightId_)) &&
        (parcel.ReadInt32(lightNumber_)) && (parcel.ReadInt32(lightType_));
}
}  // namespace Sensors
}  // namespace OHOS