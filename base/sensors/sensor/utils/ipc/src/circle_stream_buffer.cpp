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

#include "circle_stream_buffer.h"

#include "sensor_errors.h"

namespace OHOS {
namespace Sensors {
bool CircleStreamBuffer::CheckWrite(size_t size)
{
#ifdef OHOS_BUILD_ENABLE_RUST
    return StreamBufferCheckWrite(streamBufferPtr_.get(), size);
#else
    size_t availSize = GetAvailableBufSize();
    if (size > availSize && rPos_ > 0) {
        CopyDataToBegin();
        availSize = GetAvailableBufSize();
    }
    return (availSize >= size);
#endif // OHOS_BUILD_ENABLE_RUST
}

bool CircleStreamBuffer::Write(const char *buf, size_t size)
{
#ifdef OHOS_BUILD_ENABLE_RUST
    return CircleStreamBufferWrite(streamBufferPtr_.get(), buf, size);
#else
    if (!CheckWrite(size)) {
        SEN_HILOGE("Buffer is overflow, availableSize:%{public}zu, size:%{public}zu,"
                   "unreadSize:%{public}zu, rPos:%{public}zu, wPos:%{public}zu",
                   GetAvailableBufSize(), size, UnreadSize(), rPos_, wPos_);
        return false;
    }
    return StreamBuffer::Write(buf, size);
#endif // OHOS_BUILD_ENABLE_RUST
}

void CircleStreamBuffer::CopyDataToBegin()
{
#ifdef OHOS_BUILD_ENABLE_RUST
    CircleStreamBufferCopyDataToBegin(streamBufferPtr_.get());
#else
    size_t unreadSize = UnreadSize();
    if (unreadSize > 0 && rPos_ > 0) {
        size_t pos = 0;
        for (size_t i = rPos_; i <= wPos_;) {
            szBuff_[pos++] = szBuff_[i++];
        }
    }
    SEN_HILOGD("UnreadSize:%{public}zu, rPos:%{public}zu, wPos:%{public}zu", unreadSize, rPos_, wPos_);
    rPos_ = 0;
    wPos_ = unreadSize;
#endif // OHOS_BUILD_ENABLE_RUST
}
} // namespace Sensors
} // namespace OHOS