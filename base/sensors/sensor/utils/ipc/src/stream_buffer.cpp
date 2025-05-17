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

#include "stream_buffer.h"

namespace OHOS {
namespace Sensors {
StreamBuffer::StreamBuffer(const StreamBuffer &buf)
{
    Clone(buf);
}

StreamBuffer &StreamBuffer::operator=(const StreamBuffer &other)
{
    Clone(other);
    return *this;
}

void StreamBuffer::Reset()
{
#ifdef OHOS_BUILD_ENABLE_RUST
    StreamBufferReset(streamBufferPtr_.get());
#else
    rPos_ = 0;
    wPos_ = 0;
    rCount_ = 0;
    wCount_ = 0;
    rwErrorStatus_ = ErrorStatus::ERROR_STATUS_OK;
#endif // OHOS_BUILD_ENABLE_RUST
}

void StreamBuffer::Clean()
{
#ifdef OHOS_BUILD_ENABLE_RUST
    StreamBufferClean(streamBufferPtr_.get());
#else
    Reset();
    errno_t ret = memset_sp(&szBuff_, sizeof(szBuff_), 0, sizeof(szBuff_));
    if (ret != EOK) {
        SEN_HILOGE("Call memset_s fail");
        return;
    }
#endif // OHOS_BUILD_ENABLE_RUST
}

bool StreamBuffer::Read(std::string &buf)
{
#ifdef OHOS_BUILD_ENABLE_RUST
    const int32_t ERROR_STATUS_READ = 1;
    if (StreamBufferGetRpos(streamBufferPtr_.get()) == StreamBufferGetWpos(streamBufferPtr_.get())) {
        SEN_HILOGE("Not enough memory to read, errCode:%{public}d", STREAM_BUF_READ_FAIL);
        StreamBufferSetRwErrStatus(streamBufferPtr_.get(), ERROR_STATUS_READ);
        return false;
    }
    buf = ReadBuf();
    StreamBufferSetRpos(streamBufferPtr_.get(),
        StreamBufferGetRpos(streamBufferPtr_.get()) + static_cast<int32_t>(buf.length()) + 1);
    return (buf.length() > 0);
#else
    if (rPos_ == wPos_) {
        SEN_HILOGE("Not enough memory to read");
        rwErrorStatus_ = ErrorStatus::ERROR_STATUS_READ;
        return false;
    }
    buf = ReadBuf();
    rPos_ += buf.length() + 1;
    return (buf.length() > 0);
#endif // OHOS_BUILD_ENABLE_RUST
}

bool StreamBuffer::Write(const std::string &buf)
{
    return Write(buf.c_str(), buf.length()+1);
}

bool StreamBuffer::Read(StreamBuffer &buf)
{
#ifdef OHOS_BUILD_ENABLE_RUST
    return StreamBufferRead(streamBufferPtr_.get(), buf.streamBufferPtr_.get());
#else
    return buf.Write(Data(), Size());
#endif // OHOS_BUILD_ENABLE_RUST
}

bool StreamBuffer::Write(const StreamBuffer &buf)
{
#ifdef OHOS_BUILD_ENABLE_RUST
    return StreamBufferWrite(streamBufferPtr_.get(), buf.streamBufferPtr_.get());
#else
    return Write(buf.Data(), buf.Size());
#endif // OHOS_BUILD_ENABLE_RUST
}

bool StreamBuffer::Read(char *buf, size_t size)
{
#ifdef OHOS_BUILD_ENABLE_RUST
    return StreamBufferReadChar(streamBufferPtr_.get(), buf, size);
#else
    if (ChkRWError()) {
        return false;
    }
    if (buf == nullptr) {
        SEN_HILOGE("Invalid input parameter, buf is nullptr");
        rwErrorStatus_ = ErrorStatus::ERROR_STATUS_READ;
        return false;
    }
    if (size == 0) {
        SEN_HILOGE("Invalid input parameter, size:%{public}zu", size);
        rwErrorStatus_ = ErrorStatus::ERROR_STATUS_READ;
        return false;
    }
    if (rPos_ + size > wPos_) {
        SEN_HILOGE("Memory out of bounds on read");
        rwErrorStatus_ = ErrorStatus::ERROR_STATUS_READ;
        return false;
    }
    errno_t ret = memcpy_sp(buf, size, ReadBuf(), size);
    if (ret != EOK) {
        SEN_HILOGE("Failed to call memcpy_sp");
        rwErrorStatus_ = ErrorStatus::ERROR_STATUS_READ;
        return false;
    }
    rPos_ += size;
    ++rCount_;
    return true;
#endif // OHOS_BUILD_ENABLE_RUST
}

bool StreamBuffer::Write(const char *buf, size_t size)
{
#ifdef OHOS_BUILD_ENABLE_RUST
    return StreamBufferWriteChar(streamBufferPtr_.get(), buf, size);
#else
    if (ChkRWError()) {
        return false;
    }
    if (buf == nullptr) {
        SEN_HILOGE("Invalid input parameter, buf is nullptr");
        rwErrorStatus_ = ErrorStatus::ERROR_STATUS_WRITE;
        return false;
    }
    if (size == 0) {
        SEN_HILOGE("Invalid input parameter, size:%{public}zu", size);
        rwErrorStatus_ = ErrorStatus::ERROR_STATUS_WRITE;
        return false;
    }
    if (wPos_ + size > MAX_STREAM_BUF_SIZE) {
        SEN_HILOGE("The write length exceeds buffer. wPos:%{public}zu, size:%{public}zu, maxBufSize:%{public}zu",
                   wPos_, size, MAX_STREAM_BUF_SIZE);
        rwErrorStatus_ = ErrorStatus::ERROR_STATUS_WRITE;
        return false;
    }
    errno_t ret = memcpy_sp(&szBuff_[wPos_], GetAvailableBufSize(), buf, size);
    if (ret != EOK) {
        SEN_HILOGE("Failed to call memcpy_sp");
        rwErrorStatus_ = ErrorStatus::ERROR_STATUS_WRITE;
        return false;
    }
    wPos_ += size;
    ++wCount_;
    return true;
#endif // OHOS_BUILD_ENABLE_RUST
}

const char *StreamBuffer::ReadBuf() const
{
#ifdef OHOS_BUILD_ENABLE_RUST
    return StreamBufferReadBuf(streamBufferPtr_.get());
#else
    return &szBuff_[rPos_];
#endif // OHOS_BUILD_ENABLE_RUST
}

bool StreamBuffer::Clone(const StreamBuffer &buf)
{
    Clean();
#ifdef OHOS_BUILD_ENABLE_RUST
    return Write(StreamBufferData(buf.streamBufferPtr_.get()), StreamBufferSize(buf.streamBufferPtr_.get()));
#else
    return Write(buf.Data(), buf.Size());
#endif // OHOS_BUILD_ENABLE_RUST
}
#ifndef OHOS_BUILD_ENABLE_RUST

bool StreamBuffer::ChkRWError() const
{
    return (rwErrorStatus_ != ErrorStatus::ERROR_STATUS_OK);
}

bool StreamBuffer::SeekReadPos(size_t n)
{
    size_t pos = rPos_ + n;
    if (pos > wPos_) {
        SEN_HILOGE("The position in the calculation is not as expected. pos:%{public}zu, [0, %{public}zu]", pos, wPos_);
        return false;
    }
    rPos_ = pos;
    return true;
}

bool StreamBuffer::IsEmpty() const
{
    return (rPos_ == wPos_);
}

size_t StreamBuffer::Size() const
{
#ifdef OHOS_BUILD_ENABLE_RUST
    return StreamBufferSize(&rustStreamBuffer_);
#else
    return wPos_;
#endif // OHOS_BUILD_ENABLE_RUST
}

size_t StreamBuffer::UnreadSize() const
{
    return ((wPos_ <= rPos_) ? 0 : (wPos_ - rPos_));
}

size_t StreamBuffer::GetAvailableBufSize() const
{
    return ((wPos_ >= MAX_STREAM_BUF_SIZE) ? 0 : (MAX_STREAM_BUF_SIZE - wPos_));
}

const std::string &StreamBuffer::GetErrorStatusRemark() const
{
#ifdef OHOS_BUILD_ENABLE_RUST
    return StreamBufferGetErrorStatusRemark(streamBufferPtr_.get());
#else
    static const std::vector<std::pair<ErrorStatus, std::string>> remark {
        {ErrorStatus::ERROR_STATUS_OK, "OK"},
        {ErrorStatus::ERROR_STATUS_READ, "READ_ERROR"},
        {ErrorStatus::ERROR_STATUS_WRITE, "WRITE_ERROR"},
    };
    static const std::string invalidStatus { "UNKNOWN" };
    auto tIter = std::find_if(remark.cbegin(), remark.cend(), [this](const auto &item) {
        return (item.first == rwErrorStatus_);
    });
    return (tIter != remark.cend() ? tIter->second : invalidStatus);
#endif // OHOS_BUILD_ENABLE_RUST
}

const char *StreamBuffer::Data() const
{
#ifdef OHOS_BUILD_ENABLE_RUST
    return StreamBufferData(&rustStreamBuffer_);
#else
    return &szBuff_[0];
#endif // OHOS_BUILD_ENABLE_RUST
}

const char *StreamBuffer::WriteBuf() const
{
    return &szBuff_[wPos_];
}
#endif // OHOS_BUILD_ENABLE_RUST
} // namespace Sensors
} // namespace OHOS
