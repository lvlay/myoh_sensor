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

#ifndef OHOS_HDI_DISPLAY_V1_0_DEVICE_CACHE_MANAGER_H
#define OHOS_HDI_DISPLAY_V1_0_DEVICE_CACHE_MANAGER_H

#include <mutex>
#include <memory>
#include <unordered_map>
#include "cache_manager.h"
#include "device_cache.h"
#include "nocopyable.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {

class DeviceCacheManager : public NoCopyable {
public:
    DeviceCacheManager();
    ~DeviceCacheManager();
    static std::shared_ptr<DeviceCacheManager> GetInstance();
    int32_t AddDeviceCache(uint32_t deviceId);
    int32_t RemoveDeviceCache(uint32_t deviceId);
    int32_t CreateVirtualDisplayCache(uint32_t deviceId);
    int32_t DestroyVirtualDisplayCache(uint32_t deviceId);
    int32_t DestroyCaches();
    DeviceCache* DeviceCacheInstance(uint32_t deviceId) const;
    LayerCache* LayerCacheInstance(uint32_t deviceId, uint32_t layerId) const;
    void Dump() const;
    static std::mutex& GetCacheMgrMutex();
private:
    int32_t Init();
    int32_t AddCacheInternal(uint32_t deviceId, DeviceCache::DeviceType type);
    std::unique_ptr<CacheManager<uint32_t, DeviceCache>> deviceCaches_;
};
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_0_DEVICE_CACHE_MANAGER_H
