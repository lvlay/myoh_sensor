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

/**
 * @file light_agent.h
 *
 * @brief Declares the functions for starting or stopping a light.
 * @since 10
 */

#ifndef LIGHT_AGENT_H
#define LIGHT_AGENT_H

#include "light_agent_type.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace OHOS {
namespace Sensors {
/**
 * @brief Obtains information about all the lights in the system.
 *
 * @param lightInfo Indicates the list of the light information.
 * @param count Indicates the count of all the lights in the system.
 * @return Returns <b>0</b> if the operation is successful.
 * @return Returns a negative value if the operation fails.
 * @since 10
 */
int32_t GetLightList(LightInfo **lightInfo, int32_t &count);

/**
 * @brief Turns on available lights in the list based on the specified light id.
 *
 * @param lightId Indicates the light id.
 * @param color：Color corresponding to the light.
 * @param animation Indicates the blinking parameters.
 * @return Returns <b>0</b> if the operation is successful.
 * @return Returns a negative value if the operation fails.
 * @since 10
 */
int32_t TurnOn(int32_t lightId, const LightColor &color, const LightAnimation &animation);

/**
 * @brief Turns off available lights in the list based on the specified light id.
 *
 * @param lightId Indicates the light id.
 * @return Returns <b>0</b> if the operation is successful.
 * @return Returns a negative value if the operation fails.
 * @since 10
 */
int32_t TurnOff(int32_t lightId);
} // namespace Sensors
} // namespace OHOS
#ifdef __cplusplus
};
#endif
/** @} */
#endif // endif LIGHT_AGENT_H
