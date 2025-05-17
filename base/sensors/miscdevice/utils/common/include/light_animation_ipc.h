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

#ifndef LIGHT_ANIMATION_IPC_H
#define LIGHT_ANIMATION_IPC_H

#include "parcel.h"

namespace OHOS {
namespace Sensors {
class LightAnimationIPC : public Parcelable {
public:
    LightAnimationIPC() = default;
    virtual ~LightAnimationIPC() = default;
    int32_t GetMode() const;
    void SetMode(int32_t mode);
    int32_t GetOnTime() const;
    void SetOnTime(int32_t onTime);
    int32_t GetOffTime() const;
    void SetOffTime(int32_t offTime);
    bool ReadFromParcel(Parcel &parcel);
    static std::unique_ptr<LightAnimationIPC> Unmarshalling(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;

private:
    int32_t mode_ = 0;
    int32_t onTime_ = 0;
    int32_t offTime_ = 0;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // LIGHT_ANIMATION_IPC_H
