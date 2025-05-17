/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef APP_THREAD_INFO_H
#define APP_THREAD_INFO_H

#include "accesstoken_kit.h"

namespace OHOS {
namespace Sensors {
using Security::AccessToken::AccessTokenID;
struct AppThreadInfo {
    int32_t pid { 0 };
    int32_t uid { 0 };
    AccessTokenID callerToken { 0 };
    AppThreadInfo() {};
    AppThreadInfo(int32_t pid, int32_t uid, AccessTokenID callerToken)
        : pid(pid), uid(uid), callerToken(callerToken) {};
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // APP_THREAD_INFO_H
