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

#ifndef OHOS_HDI_DISPLAY_V1_0_DEVICE_CACHE_H
#define OHOS_HDI_DISPLAY_V1_0_DEVICE_CACHE_H

#include <functional>
#include <memory>
#include <unordered_map>
#include "base/native_buffer.h"
#include "cache_manager.h"
#include "layer_cache.h"
#include "nocopyable.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
using namespace OHOS::HDI::Base;
class DeviceCache : public NoCopyable {
public:
    enum DeviceType {
        DEVICE_TYPE_DEVICE = 0,
        DEVICE_TYPE_VIRTUAL,
        DEVICE_TYPE_BUTT
    };

    ~DeviceCache();
    static DeviceCache* Create(uint32_t id, DeviceType type);
    int32_t SetClientBufferCacheCount(uint32_t bufferCacheCount);
    LayerCache* LayerCacheInstance(uint32_t layerId) const;
    int32_t AddLayerCache(uint32_t id, uint32_t bufferCacheCount);
    int32_t RemoveLayerCache(uint32_t id);
    int32_t ClearClientCache();
    int32_t ClearLayerBuffer(uint32_t layerId);
    int32_t SetDisplayClientBuffer(BufferHandle*& buffer, uint32_t seqNo, bool &needFreeBuffer,
        std::function<int32_t (const BufferHandle&)> realFunc);
    int32_t SetVirtualDisplayBuffer(BufferHandle*& buffer, uint32_t seqNo, bool &needFreeBuffer,
        std::function<int32_t (const BufferHandle&)> realFunc);
    DeviceType CacheType() const;
    void Dump() const;
private:
    DeviceCache(uint32_t id, DeviceType type);
    int32_t Init();
    uint32_t deviceId_;
    DeviceType cacheType_;
    std::shared_ptr<CacheManager<uint32_t, NativeBuffer>> clientBufferCaches_;
    std::shared_ptr<CacheManager<uint32_t, NativeBuffer>> outputBufferCaches_;
    std::shared_ptr<CacheManager<uint32_t, LayerCache>> layerCaches_;
};
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_0_DEVICE_CACHE_H
