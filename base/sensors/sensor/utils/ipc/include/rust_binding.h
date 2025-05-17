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

#ifndef RUST_BINDING_H
#define RUST_BINDING_H
#include <stdint.h>
#include <string.h>
#include "proto.h"

extern "C" {
    struct RustStreamSocket;
    struct RustStreamSession;
    struct RustStreamBuffer;
    struct RustNetPacket {
        OHOS::Sensors::MessageId msgId { OHOS::Sensors::MessageId::INVALID };
        struct RustStreamBuffer *streamBuffer;
    };
    RustStreamSocket *StreamSocketCreate(void);
    void StreamSocketDelete(RustStreamSocket *raw);
    int32_t StreamSocketGetFd(const RustStreamSocket *rustStreamSocket);
    int32_t StreamSocketClose(RustStreamSocket *rustStreamSocket);
    int32_t StreamSocketSetFd(RustStreamSocket *rustStreamSocket, int32_t fd);
    RustStreamSession *StreamSessionCreate(void);
    void StreamSessionDelete(RustStreamSession *raw);
    void StreamSessionSetUid(RustStreamSession *rustStreamSession, int32_t uid);
    void StreamSessionSetPid(RustStreamSession *rustStreamSession, int32_t pid);
    void StreamSessionSetFd(RustStreamSession *rustStreamSession, int32_t fd);
    void StreamSessionClose(RustStreamSession *rustStreamSession);
    bool StreamSessionSendMsg(const RustStreamSession *rustStreamSession, const char *buf, size_t size);
    int32_t StreamSessionGetUid(const RustStreamSession *rustStreamSession);
    int32_t StreamSessionGetPid(const RustStreamSession *rustStreamSession);
    int32_t StreamSessionGetFd(const RustStreamSession *rustStreamSession);
    void StreamSessionSetTokenType(RustStreamSession *rustStreamSession, int32_t type);
    int32_t StreamSessionGetTokenType(const RustStreamSession *rustStreamSession);
    int32_t StreamSessionGetModuleType(const RustStreamSession *rustStreamSession);
    RustStreamBuffer *StreamBufferCreate(void);
    void StreamBufferDelete(RustStreamBuffer *raw);
    int32_t StreamBufferGetWcount(const RustStreamBuffer *rustStreamBuffer);
    int32_t StreamBufferGetRcount(const RustStreamBuffer *rustStreamBuffer);
    int32_t StreamBufferGetWpos(const RustStreamBuffer *rustStreamBuffer);
    int32_t StreamBufferGetRpos(const RustStreamBuffer *rustStreamBuffer);
    const char *StreamBufferGetSzBuff(const RustStreamBuffer *rustStreamBuffer);
    int32_t StreamBufferSetRwErrStatus(RustStreamBuffer *rustStreamBuffer, int32_t rwErrStatus);
    int32_t StreamBufferSetRpos(RustStreamBuffer *rustStreamBuffer, int32_t rPos);
    void StreamBufferReset(RustStreamBuffer *rustStreamBuffer);
    void StreamBufferClean(RustStreamBuffer *rustStreamBuffer);
    bool StreamBufferRead(RustStreamBuffer *rustStreamBuffer1, RustStreamBuffer *rustStreamBuffer2);
    bool StreamBufferWrite(RustStreamBuffer *rustStreamBuffer1, const RustStreamBuffer *rustStreamBuffer2);
    const char *StreamBufferReadBuf(const RustStreamBuffer *rustStreamBuffer);
    bool StreamBufferReadChar(RustStreamBuffer *rustStreamBuffer, char *buf, size_t size);
    bool StreamBufferWriteChar(RustStreamBuffer *rustStreamBuffer, const char *buf, size_t size);
    const char *StreamBufferData(const RustStreamBuffer *rustStreamBuffer);
    size_t StreamBufferSize(const RustStreamBuffer *rustStreamBuffer);
    const char *StreamBufferGetErrorStatusRemark(const RustStreamBuffer *rustStreamBuffer);
    bool StreamBufferChkRWError(const RustStreamBuffer *rustStreamBuffer);
    bool StreamBufferCheckWrite(RustStreamBuffer *rustStreamBuffer, size_t size);
    bool CircleStreamBufferWrite(RustStreamBuffer *rustStreamBuffer, const char *buf, size_t size);
    void CircleStreamBufferCopyDataToBegin(RustStreamBuffer *rustStreamBuffer);
}
#endif // RUST_BINDING_H