/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_HDI_DISPLAY_V1_2_DISPLAY_CMD_UTILS_H
#define OHOS_HDI_DISPLAY_V1_2_DISPLAY_CMD_UTILS_H

#include "v1_1/display_command/display_cmd_utils.h"
#include "v1_2/display_composer_type.h"

#undef LOG_TAG
#define LOG_TAG "DISP_CMD"
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002515

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_2 {

class DisplayCmdUtils : public V1_1::DisplayCmdUtils {
public:
    #define SWITCHCASE(x) case (x): {return #x;}
    static const char *CommandToString(int32_t cmdId)
    {
        switch (cmdId) {
            /* request cmd */
            SWITCHCASE(REQUEST_CMD_PREPARE_DISPLAY_LAYERS);
            SWITCHCASE(REQUEST_CMD_SET_DISPLAY_CLIENT_BUFFER);
            SWITCHCASE(REQUEST_CMD_SET_DISPLAY_CLIENT_DAMAGE);
            SWITCHCASE(REQUEST_CMD_COMMIT);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_ALPHA);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_REGION);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_CROP);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_ZORDER);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_PREMULTI);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_TRANSFORM_MODE);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_DIRTY_REGION);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_VISIBLE_REGION);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_BUFFER);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_COMPOSITION_TYPE);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_BLEND_TYPE);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_VISIBLE);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_MASK_INFO);
            SWITCHCASE(REQUEST_CMD_SET_LAYER_COLOR);
            SWITCHCASE(REQUEST_CMD_COMMIT_AND_GET_RELEASE_FENCE);
            SWITCHCASE(REQUEST_CMD_SET_DISPLAY_CONSTRAINT);
            /* reply cmd */
            SWITCHCASE(REPLY_CMD_SET_ERROR);
            SWITCHCASE(REPLY_CMD_PREPARE_DISPLAY_LAYERS);
            SWITCHCASE(REPLY_CMD_COMMIT);
            SWITCHCASE(REPLY_CMD_COMMIT_AND_GET_RELEASE_FENCE);
            /* pack control cmd */
            SWITCHCASE(CONTROL_CMD_REQUEST_BEGIN);
            SWITCHCASE(CONTROL_CMD_REPLY_BEGIN);
            SWITCHCASE(CONTROL_CMD_REQUEST_END);
            SWITCHCASE(CONTROL_CMD_REPLY_END);
            default:
                return "unknow command id.";
        }
    }
};
using CmdUtils = DisplayCmdUtils;
} // namespace V1_2
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_2_DISPLAY_CMD_UTILS_H