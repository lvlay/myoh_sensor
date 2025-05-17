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

#include "stream_session.h"

#include <cinttypes>
#include <sstream>

#include "proto.h"
#include "sensor_errors.h"
#include "stream_socket.h"

#undef LOG_TAG
#define LOG_TAG "StreamSession"

namespace OHOS {
namespace Sensors {

StreamSession::StreamSession(const std::string &programName, const int32_t fd, const int32_t uid, const int32_t pid)
    : programName_(programName)
#ifdef OHOS_BUILD_ENABLE_RUST
{
    StreamSessionSetFd(streamSessionPtr_.get(), fd);
    StreamSessionSetUid(streamSessionPtr_.get(), uid);
    StreamSessionSetPid(streamSessionPtr_.get(), pid);
    UpdateDescript();
}
#else
,
      fd_(fd),
      uid_(uid),
      pid_(pid)
{
    UpdateDescript();
}
#endif // OHOS_BUILD_ENABLE_RUST


bool StreamSession::SendMsg(const char *buf, size_t size) const
{
#ifdef OHOS_BUILD_ENABLE_RUST
    return StreamSessionSendMsg(streamSessionPtr_.get(), buf, size);
#else
    CHKPF(buf);
    if ((size == 0) || (size > MAX_PACKET_BUF_SIZE)) {
        SEN_HILOGE("buf size:%{public}zu", size);
        return false;
    }
    if (fd_ < 0) {
        SEN_HILOGE("The fd_ is less than 0");
        return false;
    }
    size_t idx = 0;
    size_t retryCount = 0;
    size_t remSize = size;
    while (remSize > 0 && retryCount < SEND_RETRY_LIMIT) {
        ++retryCount;
        auto count = send(fd_, &buf[idx], remSize, MSG_DONTWAIT | MSG_NOSIGNAL);
        if (count < 0) {
            if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {
#ifdef OHOS_BUILD_ENABLE_RUST
                sleep(Duration::from_micros(SEND_RETRY_SLEEP_TIME));
#else
                usleep(SEND_RETRY_SLEEP_TIME);
#endif
                SEN_HILOGW("Continue for errno EAGAIN|EINTR|EWOULDBLOCK, errno:%{public}d", errno);
                continue;
            }
            SEN_HILOGE("Send return failed, error:%{public}d, fd:%{public}d", errno, fd_);
            return false;
        }
        idx += static_cast<size_t>(count);
        remSize -= static_cast<size_t>(count);
        if (remSize > 0) {
#ifdef OHOS_BUILD_ENABLE_RUST
            sleep(Duration::from_micros(SEND_RETRY_SLEEP_TIME));
#else
            usleep(SEND_RETRY_SLEEP_TIME);
#endif
        }
    }
    if (retryCount >= SEND_RETRY_LIMIT || remSize != 0) {
        SEN_HILOGE("Send too many times:%{public}zu/%{public}zu, size:%{public}zu/%{public}zu, fd:%{public}d",
            retryCount, SEND_RETRY_LIMIT, idx, size, fd_);
        return false;
    }
    return true;
#endif // OHOS_BUILD_ENABLE_RUST
}

void StreamSession::Close()
{
#ifdef OHOS_BUILD_ENABLE_RUST
    StreamSessionClose(streamSessionPtr_.get());
    UpdateDescript();
#else
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
        UpdateDescript();
    }
#endif // OHOS_BUILD_ENABLE_RUST
}

void StreamSession::UpdateDescript()
{
#ifdef OHOS_BUILD_ENABLE_RUST
    std::ostringstream oss;
    oss << "fd = " << StreamSessionGetFd(streamSessionPtr_.get())
        << ", programName = " << programName_
        << ", moduleType = " << StreamSessionGetModuleType(streamSessionPtr_.get())
        << ((StreamSessionGetFd(streamSessionPtr_.get()) < 0) ? ", closed" : ", opened")
        << ", uid = " << StreamSessionGetUid(streamSessionPtr_.get())
        << ", pid = " << StreamSessionGetPid(streamSessionPtr_.get())
        << ", tokenType = " << StreamSessionGetTokenType(streamSessionPtr_.get())
        << std::endl;
    descript_ = oss.str().c_str();
#else
    std::ostringstream oss;
    oss << "fd = " << fd_
        << ", programName = " << programName_
        << ((fd_ < 0) ? ", closed" : ", opened")
        << ", uid = " << uid_
        << ", pid = " << pid_
        << ", tokenType = " << tokenType_
        << std::endl;
    descript_ = oss.str().c_str();
#endif // OHOS_BUILD_ENABLE_RUST
}

bool StreamSession::SendMsg(const NetPacket &pkt) const
{
#ifdef OHOS_BUILD_ENABLE_RUST
    if (StreamBufferChkRWError(pkt.streamBufferPtr_.get())) {
        SEN_HILOGE("Read and write status is error");
        return false;
    }
    StreamBuffer buf;
    pkt.MakeData(buf);
    return SendMsg(StreamBufferData(buf.streamBufferPtr_.get()), StreamBufferSize(buf.streamBufferPtr_.get()));
#else
    if (pkt.ChkRWError()) {
        SEN_HILOGE("Read and write status failed");
        return false;
    }
    StreamBuffer buf;
    pkt.MakeData(buf);
    return SendMsg(buf.Data(), buf.Size());
#endif // OHOS_BUILD_ENABLE_RUST
}

int32_t StreamSession::GetUid() const
{
#ifdef OHOS_BUILD_ENABLE_RUST
    return StreamSessionGetUid(streamSessionPtr_.get());
#else
    return uid_;
#endif // OHOS_BUILD_ENABLE_RUST
}

int32_t StreamSession::GetPid() const
{
#ifdef OHOS_BUILD_ENABLE_RUST
    return StreamSessionGetPid(streamSessionPtr_.get());
#else
    return pid_;
#endif // OHOS_BUILD_ENABLE_RUST
}

SessionPtr StreamSession::GetSharedPtr()
{
    return shared_from_this();
}

int32_t StreamSession::GetFd() const
{
#ifdef OHOS_BUILD_ENABLE_RUST
    return StreamSessionGetFd(streamSessionPtr_.get());
#else
    return fd_;
#endif // OHOS_BUILD_ENABLE_RUST
}

const std::string &StreamSession::GetDescript() const
{
    return descript_;
}

const std::string StreamSession::GetProgramName() const
{
    return programName_;
}

void StreamSession::SetTokenType(int32_t type)
{
#ifdef OHOS_BUILD_ENABLE_RUST
    StreamSessionSetTokenType(streamSessionPtr_.get(), type);
#else
    tokenType_ = type;
#endif // OHOS_BUILD_ENABLE_RUST
}

int32_t StreamSession::GetTokenType() const
{
#ifdef OHOS_BUILD_ENABLE_RUST
    return StreamSessionGetTokenType(streamSessionPtr_.get());
#else
    return tokenType_;
#endif // OHOS_BUILD_ENABLE_RUST
}
} // namespace Sensors
} // namespace OHOS