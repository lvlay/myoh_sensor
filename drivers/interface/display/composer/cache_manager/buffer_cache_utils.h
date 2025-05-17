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

#ifndef OHOS_HDI_DISPLAY_V1_0_BUFFER_CACHE_UTILS_H
#define OHOS_HDI_DISPLAY_V1_0_BUFFER_CACHE_UTILS_H

#include <functional>
#include <memory>
#include <unordered_map>
#include "base/native_buffer.h"
#include "cache_manager.h"
#include "common/include/display_interface_utils.h"
#include "hdf_log.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {

#define DEBUG_BUFFER_CACHE_UTILS
class BufferCacheUtils {
public:
    static BufferHandle* NativeBufferCache(const std::shared_ptr<CacheManager<uint32_t, NativeBuffer>>& cacheMgr,
        BufferHandle* buffer, uint32_t seqNo, uint32_t callerId, bool &needFreeBuffer)
    {
        BufferHandle* handle = nullptr;
        needFreeBuffer = false;
        if (buffer == nullptr && seqNo != UINT32_MAX) {
            // Fetch buffer from caches indexed by seqNo
            NativeBuffer* nativeBuffer = cacheMgr->SearchCache(seqNo);
            DISPLAY_CHK_RETURN(nativeBuffer == nullptr, handle,
                HDF_LOGE("%{public}s: Get Native buffer cache is not found, callerId=%{public}u, seqNo=%{public}u",
                    __func__, callerId, seqNo));

            handle = nativeBuffer->GetBufferHandle();
            HDF_LOGD("apply %{public}s: callerId=%{public}u, seqNo=%{public}u", __func__, callerId, seqNo);
            DISPLAY_CHK_RETURN(handle == nullptr, handle,
                HDF_LOGE("%{public}s: Get buffer cache is nullptr, callerId=%{public}u, seqNo=%{public}u",
                    __func__, callerId, seqNo));
        } else if (buffer != nullptr && seqNo != UINT32_MAX) {
            // Insert buffer to caches
            NativeBuffer* nativeBuffer = new NativeBuffer();
            DISPLAY_CHK_RETURN(nativeBuffer == nullptr, nullptr,
                HDF_LOGE("%{public}s: new nativeBuffer fail", __func__));
            nativeBuffer->SetBufferHandle(buffer, true, nullptr);

            auto retBool = cacheMgr->InsertCache(seqNo, nativeBuffer);
            if (retBool == false) {
                // if InsertCache failed, remove BufferHandle ownership
                handle = nativeBuffer->Move();
                delete nativeBuffer;
                HDF_LOGE("%{public}s: Set buffer cache fail, callerId=%{public}u, seqNo=%{public}u",
                    __func__, callerId, seqNo);
                needFreeBuffer = true;
            } else {
                handle = buffer;
            }
            HDF_LOGD("insert %{public}s: callerId=%{public}u, seqNo=%{public}u", __func__, callerId, seqNo);
        } else if (buffer != nullptr && seqNo == UINT32_MAX) {
            // Caches not used
            HDF_LOGI("%{public}s: buffer cache passthrough", __func__);
            handle = buffer;
            needFreeBuffer = true;
        } else {
            // Input arguments error
            DISPLAY_CHK_RETURN(((buffer == nullptr)), nullptr,
                HDF_LOGE("%{public}s: Inputs args check error", __func__));
        }

        return handle;
    }
};
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_0_BUFFER_CACHE_UTILS_H
