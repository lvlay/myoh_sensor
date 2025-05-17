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

#include "stream_socket.h"

#include <cinttypes>

#include "sensor_errors.h"

namespace OHOS {
namespace Sensors {
#ifndef OHOS_BUILD_ENABLE_RUST
#undef LOG_TAG
#define LOG_TAG "StreamSocket"
#endif // OHOS_BUILD_ENABLE_RUST

StreamSocket::StreamSocket() {}

StreamSocket::~StreamSocket()
{
#ifdef OHOS_BUILD_ENABLE_RUST
    StreamSocketClose(streamSocketPtr_.get());
#else
    Close();
#endif // OHOS_BUILD_ENABLE_RUST
}

#ifndef OHOS_BUILD_ENABLE_RUST
void StreamSocket::OnReadPackets(CircleStreamBuffer &circBuf, StreamSocket::PacketCallBackFun callbackFun)
{
    constexpr size_t headSize = sizeof(PackHead);
    for (size_t i = 0; i < ONCE_PROCESS_NETPACKET_LIMIT; ++i) {
        const size_t unreadSize = circBuf.UnreadSize();
        if (unreadSize < headSize) {
            break;
        }
        size_t dataSize = unreadSize - headSize;
        char *buf = const_cast<char *>(circBuf.ReadBuf());
        CHKPB(buf);
        PackHead *head = reinterpret_cast<PackHead *>(buf);
        CHKPB(head);
        if (head->size < 0 || head->size > MAX_PACKET_BUF_SIZE) {
            SEN_HILOGE("Packet header parsing error, and this error cannot be recovered. The buffer will be reset."
                " head->size:%{public}zu, unreadSize:%{public}zu", head->size, unreadSize);
            circBuf.Reset();
            break;
        }
        if (head->size > dataSize) {
            break;
        }
        NetPacket pkt(head->idMsg);
        if ((head->size > 0) && (!pkt.Write(&buf[headSize], head->size))) {
            SEN_HILOGW("Error writing data in the NetPacket. It will be retried next time. messageid:%{public}d,"
                "size:%{public}zu", head->idMsg, head->size);
            break;
        }
        if (!circBuf.SeekReadPos(pkt.GetPacketLength())) {
            SEN_HILOGW("Set read position error, and this error cannot be recovered, and the buffer will be reset."
                " packetSize:%{public}zu, unreadSize:%{public}zu", pkt.GetPacketLength(), unreadSize);
            circBuf.Reset();
            break;
        }
        callbackFun(pkt);
        if (circBuf.IsEmpty()) {
            circBuf.Reset();
            break;
        }
    }
}
#endif // OHOS_BUILD_ENABLE_RUST

void StreamSocket::Close()
{
#ifdef OHOS_BUILD_ENABLE_RUST
    StreamSocketClose(streamSocketPtr_.get());
#else
    if (fd_ >= 0) {
        auto rf = close(fd_);
        if (rf != 0) {
            SEN_HILOGE("Socket close failed, rf:%{public}d", rf);
        }
    }
    fd_ = -1;
#endif // OHOS_BUILD_ENABLE_RUST
}

int32_t StreamSocket::GetFd() const
{
#ifdef OHOS_BUILD_ENABLE_RUST
    return StreamSocketGetFd(streamSocketPtr_.get());
#else
    return fd_;
#endif // OHOS_BUILD_ENABLE_RUST
}
} // namespace Sensors
} // namespace OHOS