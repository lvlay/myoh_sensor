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

#include "dev_change_callback.h"
#include "edm_errors.h"
#include "etx_device_mgr.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace ExternalDeviceManager {
int32_t DevChangeCallback::OnDeviceAdd(std::shared_ptr<DeviceInfo> device)
{
    EDM_LOGD(MODULE_DEV_MGR, "OnDeviceAdd start");
    return ExtDeviceManager::GetInstance().RegisterDevice(device);
}

int32_t DevChangeCallback::OnDeviceRemove(std::shared_ptr<DeviceInfo> device)
{
    EDM_LOGD(MODULE_DEV_MGR, "OnDeviceRemove start");
    return ExtDeviceManager::GetInstance().UnRegisterDevice(device);
}
} // namespace ExternalDeviceManager
} // namespace OHOS