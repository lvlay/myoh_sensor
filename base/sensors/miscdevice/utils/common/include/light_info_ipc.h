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

#ifndef LIGHT_INFO_IPC_H
#define LIGHT_INFO_IPC_H

#include <string>

#include "parcel.h"

namespace OHOS {
namespace Sensors {
class LightInfoIPC : public Parcelable {
public:
    LightInfoIPC() = default;
    virtual ~LightInfoIPC() = default;
    std::string GetLightName() const;
    void SetLightName(const std::string &lightName);
    int32_t GetLightId() const;
    void SetLightId(int32_t lightId);
    int32_t GetLightNumber() const;
    void SetLightNumber(int32_t lightNumber);
    int32_t GetLightType() const;
    void SetLightType(int32_t lightType);
    bool ReadFromParcel(Parcel &parcel);
    static std::unique_ptr<LightInfoIPC> Unmarshalling(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;

private:
    std::string lightName_ = "";
    int32_t lightId_ = 0;
    int32_t lightNumber_ = 0;
    int32_t lightType_ = 0;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // LIGHT_INFO_IPC_H
