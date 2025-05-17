/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef USER_IDM_FUNCS_H
#define USER_IDM_FUNCS_H

#include "idm_database.h"
#include "idm_session.h"
#include "user_sign_centre.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t token[AUTH_TOKEN_LEN];
    int32_t userId;
    uint32_t authType;
    uint32_t executorSensorHint;
    int32_t userType;
} PermissionCheckParam;

typedef struct {
    uint8_t token[AUTH_TOKEN_LEN];
    int32_t userId;
    uint64_t credentialId;
} CredentialDeleteParam;

typedef struct {
    uint64_t credentialId;
    CredentialInfoHal deletedCredential;
    Buffer *oldRootSecret;
    Buffer *rootSecret;
    Buffer *authToken;
} UpdateCredentialOutput;

ResultCode CheckEnrollPermission(PermissionCheckParam param, uint64_t *scheduleId);
ResultCode CheckUpdatePermission(PermissionCheckParam param, uint64_t *scheduleId);
ResultCode AddCredentialFunc(int32_t userId, const Buffer *scheduleResult, uint64_t *credentialId, Buffer **rootSecret,
    Buffer **authToken);
ResultCode DeleteCredentialFunc(CredentialDeleteParam param, CredentialInfoHal *credentialInfo);
ResultCode QueryCredentialFunc(int32_t userId, uint32_t authType, LinkedList **creds);
ResultCode GetUserInfoFunc(int32_t userId, uint64_t *secureUid, uint64_t *pinSubType,
    EnrolledInfoHal **enrolledInfoArray, uint32_t *enrolledNum);
ResultCode UpdateCredentialFunc(int32_t userId, const Buffer *scheduleResult, UpdateCredentialOutput *output);
ResultCode QueryAllExtUserInfoFunc(UserInfoResult *userInfos, uint32_t userInfolen, uint32_t *userInfoCount);

#ifdef __cplusplus
}
#endif

#endif // USER_IDM_FUNCS_H
