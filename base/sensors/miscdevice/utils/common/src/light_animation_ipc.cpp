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

#include "light_animation_ipc.h"

#include "sensors_errors.h"

#undef LOG_TAG
#define LOG_TAG "LightAnimationIPC"

namespace OHOS {
namespace Sensors {
int32_t LightAnimationIPC::GetMode() const
{
    return mode_;
}
    
void LightAnimationIPC::SetMode(int32_t mode)
{
    mode_ = mode;
}

int32_t LightAnimationIPC::GetOnTime() const
{
    return onTime_;
}

void LightAnimationIPC::SetOnTime(int32_t onTime)
{
    onTime_ = onTime;
}
    
int32_t LightAnimationIPC::GetOffTime() const
{
    return offTime_;
}

void LightAnimationIPC::SetOffTime(int32_t offTime)
{
    offTime_ = offTime;
}

bool LightAnimationIPC::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(mode_)) {
        MISC_HILOGE("Failed, write mode failed");
        return false;
    }
    if (!parcel.WriteInt32(onTime_)) {
        MISC_HILOGE("Failed, write onTime failed");
        return false;
    }
    if (!parcel.WriteInt32(offTime_)) {
        MISC_HILOGE("Failed, write offTime failed");
        return false;
    }
    return true;
}

std::unique_ptr<LightAnimationIPC> LightAnimationIPC::Unmarshalling(Parcel &parcel)
{
    auto lightAnimation = std::make_unique<LightAnimationIPC>();
    if (!lightAnimation->ReadFromParcel(parcel)) {
        MISC_HILOGE("ReadFromParcel is failed");
        return nullptr;
    }
    return lightAnimation;
}

bool LightAnimationIPC::ReadFromParcel(Parcel &parcel)
{
    return (parcel.ReadInt32(mode_)) && (parcel.ReadInt32(onTime_)) && (parcel.ReadInt32(offTime_));
}
}  // namespace Sensors
}  // namespace OHOS