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

#include "fd_listener.h"

#include <cinttypes>

#include "sensor_errors.h"
#include "stream_socket.h"

#undef LOG_TAG
#define LOG_TAG "FdListener"

namespace OHOS {
namespace Sensors {
using namespace OHOS::AppExecFwk;
namespace {
constexpr int32_t MAX_DATA_BUF_SIZE = 256;
}

void FdListener::SetChannel(SensorDataChannel *channel)
{
    channel_ = channel;
}

void FdListener::OnReadable(int32_t fd)
{
    if (fd < 0) {
        SEN_HILOGE("Invalid fd, fd:%{public}d", fd);
        return;
    }
    char szBuf[MAX_DATA_BUF_SIZE] = {};
    for (size_t i = 0; i < MAX_RECV_LIMIT; i++) {
        ssize_t size = recv(fd, szBuf, MAX_DATA_BUF_SIZE, MSG_DONTWAIT | MSG_NOSIGNAL);
        if (size > 0) {
            CHKPV(channel_);
            ReceiveMessageFun receiveMessage = channel_->GetReceiveMessageFun();
            receiveMessage(szBuf, size);
            if (size < MAX_DATA_BUF_SIZE) {
                break;
            }
        } else if (size < 0) {
            if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {
                SEN_HILOGW("Continue for errno EAGAIN|EINTR|EWOULDBLOCK, size:%{public}zu, errno:%{public}d",
                    size, errno);
                continue;
            }
            SEN_HILOGE("Recv return %{public}zu, errno:%{public}d", size, errno);
            break;
        } else {
            SEN_HILOGD("The service side disconnect with the client. size:0, count:%{public}zu, errno:%{public}d",
                i, errno);
            break;
        }
    }
}

void FdListener::OnShutdown(int32_t fd)
{
    CALL_LOG_ENTER;
    if (fd < 0) {
        SEN_HILOGE("Invalid fd, fd:%{public}d", fd);
    }
    CHKPV(channel_);
    DisconnectFun disconnect = channel_->GetDisconnectFun();
    disconnect();
}

void FdListener::OnException(int32_t fd)
{
    CALL_LOG_ENTER;
    if (fd < 0) {
        SEN_HILOGE("Invalid fd, fd:%{public}d", fd);
    }
    CHKPV(channel_);
    DisconnectFun disconnect = channel_->GetDisconnectFun();
    disconnect();
}
}  // namespace Sensors
}  // namespace OHOS
