/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef I_LIGHT_HDI_CONNECTION_H
#define I_LIGHT_HDI_CONNECTION_H

#include <cstdint>
#include <string>
#include <nocopyable.h>

#include "light_agent_type.h"
#include "light_animation_ipc.h"
#include "light_info_ipc.h"

namespace OHOS {
namespace Sensors {
class ILightHdiConnection {
public:
    ILightHdiConnection() = default;
    virtual ~ILightHdiConnection() = default;
    virtual int32_t ConnectHdi() = 0;
    virtual int32_t GetLightList(std::vector<LightInfoIPC> &lightList) const = 0;
    virtual int32_t TurnOn(int32_t lightId, const LightColor &color, const LightAnimationIPC &animation) = 0;
    virtual int32_t TurnOff(int32_t lightId) = 0;
    virtual int32_t DestroyHdiConnection() = 0;

private:
    DISALLOW_COPY_AND_MOVE(ILightHdiConnection);
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // I_LIGHT_HDI_CONNECTION_H
