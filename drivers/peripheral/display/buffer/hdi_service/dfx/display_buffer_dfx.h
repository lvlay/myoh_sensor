/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_HDI_DISPLAY_BUFFER_DFX_H
#define OHOS_HDI_DISPLAY_BUFFER_DFX_H
#include <string>
#include <sys/time.h>
#ifdef DISPLAY_HICOLLIE_ENABLE
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#endif
namespace OHOS {
namespace HDI {
namespace Display {
namespace Buffer {
namespace V1_0 {
class DisplayBufferDfx {
public:
    explicit DisplayBufferDfx(std::string name);
    ~DisplayBufferDfx();
    void SetTimer();
    void CancelTimer();
    void StartTimeStamp();
    void StopTimeStamp();
private:
    std::string dfxName_;
    int32_t timeId_;
    bool flag_;
    struct timeval startTimeStamp;
    struct timeval stopTimeStamp;
};
} // namespace V1_0
} // namespace Buffer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_BUFFER_DFX_H