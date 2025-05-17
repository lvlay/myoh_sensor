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

#include "display_buffer_hdi_impl.h"
namespace OHOS {
namespace HDI {
namespace Display {
namespace Buffer {
namespace V1_2  {
using namespace OHOS::HDI::Display::Buffer::V1_2;
IDisplayBuffer *IDisplayBuffer::Get()
{
    IDisplayBuffer *instance = new V1_2::HdiDisplayBufferImpl();
    if (instance == nullptr) {
        return nullptr;
    }
    return instance;
}

} // namespace V1_2
} // namespace Buffer
} // namespace Display
} // namespace HDI
} // namespace OHOS
