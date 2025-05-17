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

#ifndef HDI_LIGHT_CONNECTION_H
#define HDI_LIGHT_CONNECTION_H

#include "death_recipient_template.h"
#include "i_light_hdi_connection.h"
#include "v1_0/light_interface_proxy.h"

namespace OHOS {
namespace Sensors {
using OHOS::HDI::Light::V1_0::ILightInterface;
class HdiLightConnection : public ILightHdiConnection {
public:
    HdiLightConnection() = default;
    virtual ~HdiLightConnection() {};
    int32_t ConnectHdi() override;
    int32_t GetLightList(std::vector<LightInfoIPC> &lightList) const override;
    int32_t TurnOn(int32_t lightId, const LightColor &color, const LightAnimationIPC &animation) override;
    int32_t TurnOff(int32_t lightId) override;
    int32_t DestroyHdiConnection() override;
    void ProcessDeathObserver(const wptr<IRemoteObject> &object);

private:
    DISALLOW_COPY_AND_MOVE(HdiLightConnection);
    sptr<IRemoteObject::DeathRecipient> hdiDeathObserver_ = nullptr;
    sptr<ILightInterface> lightInterface_ = nullptr;
    void RegisterHdiDeathRecipient();
    void UnregisterHdiDeathRecipient();
    void Reconnect();
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // HDI_LIGHT_CONNECTION_H