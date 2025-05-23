/*
 * Copyright (C) 2022-2023 Huawei Device Co., Ltd.
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

#include "token_key.h"
#include "securec.h"

#define HKS_DEFAULT_USER_AT_MAC_KEY "huks_default_user_auth_token_mac"
#define HKS_DEFAULT_USER_AT_CIPHER_KEY "huks_default_user_auth_cipherkey"

/*
 * The key here is only for example.
 * The real scene key needs to be obtained from huks, and the key life cycle is consistent with huks key.
 */
ResultCode GetTokenKey(HksAuthTokenKey *key)
{
    if (memcpy_s(key->macKey, HKS_DEFAULT_USER_AT_KEY_LEN, HKS_DEFAULT_USER_AT_MAC_KEY,
        HKS_DEFAULT_USER_AT_KEY_LEN) != EOK) {
        LOG_ERROR("macKey copy error");
        return RESULT_BAD_COPY;
    }

    if (memcpy_s(key->cipherKey, HKS_DEFAULT_USER_AT_KEY_LEN, HKS_DEFAULT_USER_AT_CIPHER_KEY,
        HKS_DEFAULT_USER_AT_KEY_LEN) != EOK) {
        LOG_ERROR("cipherKey copy error");
        return RESULT_BAD_COPY;
    }

    return RESULT_SUCCESS;
}
