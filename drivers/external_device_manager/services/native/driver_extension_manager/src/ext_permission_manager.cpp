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
#include "ext_permission_manager.h"

#include "accesstoken_kit.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace OHOS::Security::AccessToken;

bool ExtPermissionManager::VerifyPermission(std::string permissionName)
{
    AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    int result = AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (result == PERMISSION_GRANTED) {
        EDM_LOGI(MODULE_DEV_MGR, "%{public}s VerifyAccessToken: %{public}d", __func__, result);
        return true;
    }
    return false;
}

bool ExtPermissionManager::IsSystemApp()
{
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        return true;
    }
    AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    ATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(callerToken);
    return tokenType != ATokenTypeEnum::TOKEN_HAP;
}
} // namespace ExternalDeviceManager
} // namespace OHOS