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
#include "memory_guard.h"
#include <cstdint>
#include <malloc.h>
#include "intell_voice_log.h"

#undef HDF_LOG_TAG
#define HDF_LOG_TAG "MemoryGaurd"

namespace OHOS {
namespace IntelligentVoice {
namespace Utils {
MemoryGuard::MemoryGuard()
{
#ifdef CONFIG_USE_JEMALLOC_DFX_INTF
    // 0 indicates success
    int32_t ret1 = mallopt(M_SET_THREAD_CACHE, M_THREAD_CACHE_DISABLE);
    int32_t ret2 = mallopt(M_DELAYED_FREE, M_DELAYED_FREE_DISABLE);
    INTELLIGENT_VOICE_LOGI("disable tcache and delay free, result:%{public}d, %{public}d", ret1, ret2);
#endif
}

MemoryGuard::~MemoryGuard()
{
#ifdef CONFIG_USE_JEMALLOC_DFX_INTF
    int32_t err = mallopt(M_FLUSH_THREAD_CACHE, 0);
    INTELLIGENT_VOICE_LOGI("flush cache, result: %{public}d", err);
#endif
}
}
}
}