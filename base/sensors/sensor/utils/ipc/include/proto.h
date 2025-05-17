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

#ifndef PROTO_H
#define PROTO_H

#include <sys/types.h>

namespace OHOS {
namespace Sensors {
static constexpr size_t SEND_RETRY_LIMIT = 32;
static constexpr size_t SEND_RETRY_SLEEP_TIME = 10000;
static constexpr size_t MAX_VECTOR_SIZE = 10;
static constexpr size_t MAX_SESSON_ALARM = 100;
static constexpr size_t MAX_RECV_LIMIT = 13;
static constexpr size_t MAX_STREAM_BUF_SIZE = 256;
static constexpr size_t MAX_PACKET_BUF_SIZE = 256;
static constexpr size_t ONCE_PROCESS_NETPACKET_LIMIT = 100;

enum class MessageId : int32_t {
    INVALID,
    ACTIVE_INFO,
};
} // namespace Sensors
} // namespace OHOS
#endif // PROTO_H