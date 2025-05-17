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

#include "hmac_key.h"

#include <stddef.h>
#include <string.h>

#include "adaptor_algorithm.h"
#include "adaptor_log.h"
#include "buffer.h"
#include "defines.h"
#include "sign_param.h"
#include "udid_manager.h"

static Buffer *GenerateHmacKey(const Buffer *peerUdid)
{
    if (!CheckBufferWithSize(peerUdid, UDID_LEN)) {
        LOG_ERROR("param is invalid");
        return NULL;
    }

    uint8_t udid[UDID_LEN] = {};
    Uint8Array localUdidArray = { .data = udid, .len = UDID_LEN };
    if (!GetLocalUdid(&localUdidArray)) {
        LOG_ERROR("GetLocalUdid fail");
        return NULL;
    }
    Buffer localUdidBuf = GetTmpBuffer(localUdidArray.data, localUdidArray.len, localUdidArray.len);

    Buffer *salt = NULL;
    if (memcmp(localUdidBuf.buf, peerUdid->buf, UDID_LEN) < 0) {
        salt = MergeBuffers(&localUdidBuf, peerUdid);
    } else {
        salt = MergeBuffers(peerUdid, &localUdidBuf);
    }
    if (!IsBufferValid(salt)) {
        LOG_ERROR("generate salt failed");
        return NULL;
    }
    Buffer *key = NULL;
    ResultCode result = (ResultCode)GetDistributeKey(peerUdid, salt, &key);
    DestoryBuffer(salt);
    if (result != RESULT_SUCCESS) {
        LOG_ERROR("GetDistributeKey failed");
        return NULL;
    }
    if (!IsBufferValid(key)) {
        DestoryBuffer(key);
        LOG_ERROR("GenerateHmacKey fail");
        return NULL;
    }
    return key;
}

Buffer *HmacSign(const Buffer *data, SignParam signParam)
{
    if (signParam.keyType != KEY_TYPE_CROSS_DEVICE || !IsBufferValid(data)) {
        LOG_ERROR("invalid param");
        return NULL;
    }

    Buffer peerUdidBuf = GetTmpBuffer(signParam.peerUdid.data, signParam.peerUdid.len, signParam.peerUdid.len);
    Buffer *key = GenerateHmacKey(&peerUdidBuf);
    if (!IsBufferValid(key)) {
        LOG_ERROR("GenerateHmacKey failed");
        return NULL;
    }

    Buffer *signature = NULL;
    int32_t ret = HmacSha256(key, data, &signature);
    DestoryBuffer(key);
    if (ret != RESULT_SUCCESS) {
        LOG_ERROR("HmacSha256 failed");
        return NULL;
    }

    if (!CheckBufferWithSize(signature, SHA256_DIGEST_SIZE)) {
        LOG_ERROR("CheckBufferWithSize failed");
        DestoryBuffer(signature);
        return NULL;
    }

    LOG_INFO("HmacSign success");
    return signature;
}

ResultCode HmacVerify(const Buffer *data, const Buffer *sign, SignParam signParam)
{
    if (signParam.keyType != KEY_TYPE_CROSS_DEVICE || !IsBufferValid(data) || !IsBufferValid(sign)) {
        LOG_ERROR("invalid param");
        return RESULT_BAD_PARAM;
    }

    Buffer peerUdidBuf = GetTmpBuffer(signParam.peerUdid.data, signParam.peerUdid.len, signParam.peerUdid.len);
    Buffer *key = GenerateHmacKey(&peerUdidBuf);
    if (!IsBufferValid(key)) {
        LOG_ERROR("GenerateHmacKey failed");
        return RESULT_GENERAL_ERROR;
    }

    Buffer *rightSign = NULL;
    ResultCode ret = HmacSha256(key, data, &rightSign);
    DestoryBuffer(key);
    if (ret != RESULT_SUCCESS) {
        LOG_ERROR("HmacSha256 failed");
        return RESULT_GENERAL_ERROR;
    }
    if (!CompareBuffer(rightSign, sign)) {
        LOG_ERROR("sign compare failed");
        DestoryBuffer(rightSign);
        return RESULT_BAD_SIGN;
    }
    LOG_INFO("HmacVerify success");
    DestoryBuffer(rightSign);
    return RESULT_SUCCESS;
}