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

#ifndef CIRCLE_STREAM_BUFFER_H
#define CIRCLE_STREAM_BUFFER_H

#include "stream_buffer.h"

#undef LOG_TAG
#define LOG_TAG "CircleStreamBuffer"

namespace OHOS {
namespace Sensors {
class CircleStreamBuffer : public StreamBuffer {
public:
    CircleStreamBuffer() = default;
    ~CircleStreamBuffer() = default;
    bool CheckWrite(size_t size);
    bool Write(const char *buf, size_t size) override;
    DISALLOW_MOVE(CircleStreamBuffer);

protected:
    void CopyDataToBegin();
};
} // namespace Sensors
} // namespace OHOS
#endif // CIRCLE_STREAM_BUFFER_H