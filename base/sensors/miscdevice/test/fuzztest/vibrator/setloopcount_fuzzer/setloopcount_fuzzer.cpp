/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "setloopcount_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

#include "vibrator_agent.h"

namespace OHOS {
bool SetLoopCountFuzzTest(const uint8_t *data, size_t size)
{
    std::string argv(reinterpret_cast<const char *>(data), size);
    int32_t count = static_cast<int32_t>(std::atoi(argv.c_str()));
    bool ret = OHOS::Sensors::SetLoopCount(count);
    if ((count <= 0 && ret == true) || ((count > 0 && ret == false))) {
        return false;
    }
    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::SetLoopCountFuzzTest(data, size);
    return 0;
}