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

#ifndef NET_PACKET_H
#define NET_PACKET_H

#include "proto.h"
#include "stream_buffer.h"

#undef LOG_TAG
#define LOG_TAG "NetPacket"

#pragma pack(1)
using PACKHEAD = struct PackHead {
    OHOS::Sensors::MessageId idMsg;
    size_t size;
};

#pragma pack()
namespace OHOS {
namespace Sensors {
class NetPacket : public StreamBuffer {
public:
    explicit NetPacket(MessageId msgId);
    NetPacket(const NetPacket &pkt);
    NetPacket &operator = (const NetPacket &pkt);
    ~NetPacket() = default;
    void MakeData(StreamBuffer &buf) const;
    size_t GetSize() const;
    size_t GetPacketLength() const;
    const char *GetData() const;
    MessageId GetMsgId() const;
    DISALLOW_MOVE(NetPacket);

protected:
    MessageId msgId_ = MessageId::INVALID;
};
} // namespace Sensors
} // namespace OHOS
#endif // NET_PACKET_H