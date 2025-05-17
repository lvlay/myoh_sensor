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

#ifndef COMPATIBLE_LIGHT_CONNECTION_H
#define COMPATIBLE_LIGHT_CONNECTION_H

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include "i_light_hdi_connection.h"
namespace OHOS {
namespace Sensors {
class CompatibleLightConnection : public ILightHdiConnection {
public:
    CompatibleLightConnection() = default;
    virtual ~CompatibleLightConnection() = default;
    int32_t ConnectHdi() override;
    int32_t GetLightList(std::vector<LightInfoIPC> &lightList) const override;
    int32_t TurnOn(int32_t lightId, const LightColor &color, const LightAnimationIPC &animation) override;
    int32_t TurnOff(int32_t lightId) override;
    int32_t DestroyHdiConnection() override;

private:
    std::mutex turnOnLightsMutex_;
    std::vector<int32_t> turnOnLights_;
    static std::atomic_bool isStop_;
    DISALLOW_COPY_AND_MOVE(CompatibleLightConnection);
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // COMPATIBLE_LIGHT_CONNECTION_H