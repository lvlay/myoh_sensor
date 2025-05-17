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

#ifndef STREAM_BUFFER_H
#define STREAM_BUFFER_H

#include <string>
#include <vector>

#include "nocopyable.h"
#include "securec.h"

#include "proto.h"
#include "sensor_errors.h"

#ifdef OHOS_BUILD_ENABLE_RUST
#include "rust_binding.h"
#endif // OHOS_BUILD_ENABLE_RUST

#undef LOG_TAG
#define LOG_TAG "StreamBuffer"

namespace OHOS {
namespace Sensors {
class StreamBuffer {
public:
    StreamBuffer() = default;
    explicit StreamBuffer(const StreamBuffer &buf);
    virtual StreamBuffer &operator=(const StreamBuffer &other);
    virtual ~StreamBuffer() = default;
    void Reset();
    void Clean();
    bool Read(std::string &buf);
    bool Read(StreamBuffer &buf);
    bool Read(char *buf, size_t size);
    bool Write(const std::string &buf);
    bool Write(const StreamBuffer &buf);
    virtual bool Write(const char *buf, size_t size);
    const std::string &GetErrorStatusRemark() const;
    bool ChkRWError() const;
#ifndef OHOS_BUILD_ENABLE_RUST
    bool SeekReadPos(size_t n);
    bool IsEmpty() const;
    size_t Size() const;
    size_t UnreadSize() const;
    size_t GetAvailableBufSize() const;
    const char *Data() const;
    const char *WriteBuf() const;
#endif // OHOS_BUILD_ENABLE_RUST
    template<typename T>
    bool Read(T &data);
    template<typename T>
    bool Write(const T &data);
    template<typename T>
    bool Read(std::vector<T> &data);
    template<typename T>
    bool Write(const std::vector<T> &data);
    const char *ReadBuf() const;

    template<typename T>
    StreamBuffer &operator >> (T &data);
    template<typename T>
    StreamBuffer &operator << (const T &data);
    DISALLOW_MOVE(StreamBuffer);

protected:
    bool Clone(const StreamBuffer &buf);
#ifdef OHOS_BUILD_ENABLE_RUST
public:
    std::unique_ptr<RustStreamBuffer, void(*)(RustStreamBuffer*)> streamBufferPtr_ { StreamBufferCreate(),
        StreamBufferDelete };
#else
    enum class ErrorStatus {
        ERROR_STATUS_OK,
        ERROR_STATUS_READ,
        ERROR_STATUS_WRITE,
    };
    ErrorStatus rwErrorStatus_ = ErrorStatus::ERROR_STATUS_OK;
    size_t rCount_ { 0 };
    size_t wCount_ { 0 };
    size_t rPos_ { 0 };
    size_t wPos_ { 0 };
    char szBuff_[MAX_STREAM_BUF_SIZE + 1] = {};
#endif // OHOS_BUILD_ENABLE_RUST
};

template<typename T>
bool StreamBuffer::Read(T &data)
{
    if (!Read(reinterpret_cast<char *>(&data), sizeof(data))) {
#ifdef OHOS_BUILD_ENABLE_RUST
        const char *s = StreamBufferGetErrorStatusRemark(streamBufferPtr_.get());
        SEN_HILOGE("[%{public}s] size:%{public}zu count:%{public}d",
            s, sizeof(data), StreamBufferGetRcount(streamBufferPtr_.get()) + 1);
#else
        SEN_HILOGE("%{public}s, size:%{public}zu, count:%{public}zu",
            GetErrorStatusRemark().c_str(), sizeof(data), rCount_ + 1);
#endif // OHOS_BUILD_ENABLE_RUST
        return false;
    }
    return true;
}

template<typename T>
bool StreamBuffer::Write(const T &data)
{
    if (!Write(reinterpret_cast<const char *>(&data), sizeof(data))) {
#ifdef OHOS_BUILD_ENABLE_RUST
        const char *s = StreamBufferGetErrorStatusRemark(streamBufferPtr_.get());
        SEN_HILOGE("[%{public}s] size:%{public}zu,count:%{public}d",
            s, sizeof(data), StreamBufferGetWcount(streamBufferPtr_.get()) + 1);
#else
        SEN_HILOGE("%{public}s, size:%{public}zu, count:%{public}zu",
            GetErrorStatusRemark().c_str(), sizeof(data), wCount_ + 1);
#endif // OHOS_BUILD_ENABLE_RUST
        return false;
    }
    return true;
}

template<typename T>
bool StreamBuffer::Read(std::vector<T> &data)
{
    size_t size = 0;
    if (!Read(size)) {
        SEN_HILOGE("Read vector size failed");
        return false;
    }
    if (size > MAX_VECTOR_SIZE) {
        SEN_HILOGE("Vector size is invalid, size:%{public}zu", size);
        return false;
    }
    for (size_t i = 0; i < size; i++) {
        T val;
        if (!Read(val)) {
            SEN_HILOGE("Read vector data failed");
            return false;
        }
        data.push_back(val);
    }
    return true;
}

template<typename T>
bool StreamBuffer::Write(const std::vector<T> &data)
{
    if (data.size() > INT32_MAX) {
        SEN_HILOGE("Vector exceeds the max range");
        return false;
    }
    size_t size = data.size();
    if (!Write(size)) {
        SEN_HILOGE("Write vector size failed");
        return false;
    }
    for (const auto &item : data) {
        if (!Write(item)) {
            SEN_HILOGE("Write vector data failed");
            return false;
        }
    }
    return true;
}

template<typename T>
StreamBuffer &StreamBuffer::operator>>(T &data)
{
    if (!Read(data)) {
        SEN_HILOGW("Read data failed");
    }
    return *this;
}

template<typename T>
StreamBuffer &StreamBuffer::operator<<(const T &data)
{
    if (!Write(data)) {
        SEN_HILOGW("Write data failed");
    }
    return *this;
}
} // namespace Sensors
} // namespace OHOS
#endif // STREAM_BUFFER_H