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

#include "issupporteffect_fuzzer.h"

#include "securec.h"

#include "vibrator_agent.h"

namespace OHOS {
namespace Sensors {
namespace {
constexpr size_t DATA_MIN_SIZE = 20;
constexpr char END_CHAR = '\0';
constexpr size_t LEN = 20;
} // namespace

template<class T>
size_t GetObject(const uint8_t *data, size_t size, T &object)
{
    size_t objectSize = sizeof(object);
    if (objectSize > size) {
        return 0;
    }
    errno_t ret = memcpy_s(&object, objectSize, data, objectSize);
    if (ret != EOK) {
        return 0;
    }
    return objectSize;
}

void IsSupportEffectFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < DATA_MIN_SIZE) {
        return;
    }
    size_t startPos = 0;
    char effectId[LEN + 1];
    effectId[LEN] = END_CHAR;
    for (size_t i = 0; i < LEN; ++i) {
        startPos += GetObject<char>(data + startPos, size - startPos, effectId[i]);
    }
    bool state { false };
    OHOS::Sensors::IsSupportEffect(effectId, &state);
}
} // namespace Sensors
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::Sensors::IsSupportEffectFuzzTest(data, size);
    return 0;
}