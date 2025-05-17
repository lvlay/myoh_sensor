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

#ifndef LIGHT_HDI_CONNECTION_H
#define LIGHT_HDI_CONNECTION_H

#include <vector>

#include "i_light_hdi_connection.h"
#include "singleton.h"

namespace OHOS {
namespace Sensors {
class LightHdiConnection : public ILightHdiConnection, public Singleton<LightHdiConnection> {
public:
    LightHdiConnection() = default;
    virtual ~LightHdiConnection() {}
    int32_t ConnectHdi() override;
    int32_t GetLightList(std::vector<LightInfoIPC> &lightList) const override;
    int32_t TurnOn(int32_t lightId, const LightColor &color, const LightAnimationIPC &animation) override;
    int32_t TurnOff(int32_t lightId) override;
    int32_t DestroyHdiConnection() override;

private:
    DISALLOW_COPY_AND_MOVE(LightHdiConnection);
    int32_t ConnectHdiService();
    std::unique_ptr<ILightHdiConnection> iLightHdiConnection_;
    std::vector<LightInfoIPC> lightInfoList_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // LIGHT_HDI_CONNECTION_H
