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

#include "stopvibrator_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

#include "vibrator_agent.h"

namespace OHOS {
bool StopVibratorFuzzTest(const uint8_t *data, size_t size)
{
    std::string argv(reinterpret_cast<const char *>(data), size);
    int32_t ret = OHOS::Sensors::StopVibrator(argv.c_str());
    int32_t ret2 = strcmp(argv.c_str(), "time") != 0 && strcmp(argv.c_str(), "preset");
    if ((ret2 != 0 && ret == 0) || (ret2 == 0 && ret != 0)) {
        return false;
    }
    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::StopVibratorFuzzTest(data, size);
    return 0;
}