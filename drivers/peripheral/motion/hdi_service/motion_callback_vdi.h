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

#ifndef HDI_MOTION_CALLBACK_VDI_H
#define HDI_MOTION_CALLBACK_VDI_H

#include "v1_1/imotion_interface.h"
#include "imotion_callback_vdi.h"
#include "motion_uhdf_log.h"
#include "refbase.h"

namespace OHOS {
namespace HDI {
namespace Motion {
namespace V1_1 {
class MotionCallbackVdi : public IMotionCallbackVdi {
public:
    MotionCallbackVdi() = default;
    ~MotionCallbackVdi() = default;
    explicit MotionCallbackVdi(sptr<IMotionCallback> motionCallback) : motionCallback_(motionCallback) {}
    int32_t OnDataEventVdi(const HdfMotionEventVdi& eventVdi) override;
    sptr<IRemoteObject> HandleCallbackDeath() override;
private:
    sptr<IMotionCallback> motionCallback_;
};
} // V1_1
} // Motion
} // HDI
} // OHOS

#endif // HDI_MOTION_CALLBACK_VDI_H
