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

#ifndef OHOS_HDI_DISPLAY_V1_2_IDISPLAY_COMPOSER_INTERFACE_H
#define OHOS_HDI_DISPLAY_V1_2_IDISPLAY_COMPOSER_INTERFACE_H

#include "v1_1/include/idisplay_composer_interface.h"
#include "v1_2/display_composer_type.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_2 {

class IDisplayComposerInterface : public V1_1::IDisplayComposerInterface {
public:
    /**
     * @brief Obtains all interfaces of IDisplayComposerInterface.
     *
     * @return Returns <b>IDisplayComposerInterface*</b> if the operation is successful;
     * returns an null point otherwise.
     * @since 5.0
     * @version 1.2
     */
    static IDisplayComposerInterface* Get(bool needSMQ = true);

    /**
     * @brief Commits the request for composition and display.
     *        Obtains the fences of the display layers after the commit operation.
     * If there is a hardware composition layer, the composition is performed and the composition result is sent to
     * the hardware for display.
     *
     * @param devId Indicates the ID of the display device.
     * @param fence Indicates the pointer to the start address of the fence.
     * @param skipState Indicates the state of the skip validate feature.
     * @param needFlush Indicates the pointer that specifies whether the graphics service needs to reset the display
     * framebuffer by using <b>SetDisplayClientBuffer</b> before the commit operation.
     * The value <b>true</b> means that the framebuffer needs to be reset, and <b>false</b> means the opposite.
     *
     * @return Returns <b>0</b> if the operation is successful; returns an error code defined
     * in {@link DispErrCode} otherwise.
     * @since 5.0
     * @version 1.2
     */
    virtual int32_t CommitAndGetReleaseFence(uint32_t devId, int32_t& fence, int32_t& skipState,
        bool& needFlush, std::vector<uint32_t>& layers, std::vector<int32_t>& fences, bool isValidated) = 0;

    virtual int32_t RegDisplayVBlankIdleCallback(VBlankIdleCallback cb, void* data) = 0;

    virtual int32_t SetDisplayConstraint(uint32_t devId, uint64_t frameID, uint64_t ns, uint32_t type) = 0;

    virtual int32_t ClearClientBuffer(uint32_t devId) = 0;

    virtual int32_t ClearLayerBuffer(uint32_t devId, uint32_t layerId) = 0;

    virtual int32_t SetHardwareCursorPosition(uint32_t devId, int32_t x, int32_t y) = 0;

    virtual int32_t EnableHardwareCursorStats(uint32_t devId, bool enable) = 0;

    virtual int32_t GetHardwareCursorStats(uint32_t devId, uint32_t& frameCount, uint32_t& vsyncCount) = 0;
};
} // V1_2
} // Composer
} // Display
} // HDI
} // OHOS
#endif // OHOS_HDI_DISPLAY_V1_2_IDISPLAY_COMPOSER_INTERFACE_H
