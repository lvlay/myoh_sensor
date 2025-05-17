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

#include "device_cache_manager.h"

#include "common/include/display_interface_utils.h"
#include "hdf_base.h"
#include "hdf_log.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {

std::shared_ptr<DeviceCacheManager> DeviceCacheManager::GetInstance()
{
    static std::shared_ptr<DeviceCacheManager> mgr = nullptr;
    if (mgr == nullptr) {
        mgr = std::make_shared<DeviceCacheManager>();
        if (mgr == nullptr) {
            HDF_LOGE("%{public}s: DeviceCacheManager construct failed", __func__);
            return mgr;
        }

        int32_t ret = mgr->Init();
        if (ret != HDF_SUCCESS) {
            mgr = nullptr;
            HDF_LOGE("%{public}s: DeviceCacheManager init failed", __func__);
        }
    }

    return mgr;
}

DeviceCacheManager::DeviceCacheManager()
{
}

DeviceCacheManager::~DeviceCacheManager()
{
    DestroyCaches();
}

int32_t DeviceCacheManager::Init()
{
    deviceCaches_.reset(new CacheManager<uint32_t, DeviceCache>());
    DISPLAY_CHK_RETURN(deviceCaches_ == nullptr,
        HDF_FAILURE, HDF_LOGE("%{public}s: init deviceCaches failed", __func__));

    return HDF_SUCCESS;
}

int32_t DeviceCacheManager::AddDeviceCache(uint32_t deviceId)
{
    return AddCacheInternal(deviceId, DeviceCache::DEVICE_TYPE_DEVICE);
}

int32_t DeviceCacheManager::RemoveDeviceCache(uint32_t deviceId)
{
    bool ret = deviceCaches_->EraseCache(deviceId);
    DISPLAY_CHK_RETURN(ret != true, HDF_FAILURE, HDF_LOGE("%{public}s: Destroy cache failed", __func__));

    return HDF_SUCCESS;
}

int32_t DeviceCacheManager::CreateVirtualDisplayCache(uint32_t deviceId)
{
    return AddCacheInternal(deviceId, DeviceCache::DEVICE_TYPE_VIRTUAL);
}

int32_t DeviceCacheManager::DestroyVirtualDisplayCache(uint32_t deviceId)
{
    auto cache = deviceCaches_->SearchCache(deviceId);
    DISPLAY_CHK_RETURN((cache == nullptr) || (cache->CacheType() != DeviceCache::DEVICE_TYPE_VIRTUAL),
        HDF_FAILURE, HDF_LOGE("%{public}s: device is not virtual display cache", __func__));

    bool ret = deviceCaches_->EraseCache(deviceId);
    DISPLAY_CHK_RETURN(ret != true, HDF_FAILURE,
        HDF_LOGE("%{public}s: Destroy virtual display cache failed", __func__));

    return HDF_SUCCESS;
}

int32_t DeviceCacheManager::DestroyCaches()
{
    deviceCaches_.reset();
    return HDF_SUCCESS;
}

DeviceCache* DeviceCacheManager::DeviceCacheInstance(uint32_t deviceId) const
{
    auto devCache = deviceCaches_->SearchCache(deviceId);
    DISPLAY_CHK_RETURN(devCache == nullptr, nullptr, HDF_LOGE("%{public}s: Can't find device Cache", __func__));

    return devCache;
}

LayerCache* DeviceCacheManager::LayerCacheInstance(uint32_t deviceId, uint32_t layerId) const
{
    auto devCache = deviceCaches_->SearchCache(deviceId);
    DISPLAY_CHK_RETURN(devCache == nullptr, nullptr, HDF_LOGE("%{public}s: Can't find device Cache", __func__));

    auto layerCache = devCache->LayerCacheInstance(layerId);
    DISPLAY_CHK_RETURN(layerCache == nullptr, nullptr, HDF_LOGE("%{public}s: Can't find layer Cache", __func__));

    return layerCache;
}

void DeviceCacheManager::Dump() const
{
    HDF_LOGE("********************************");
    HDF_LOGE(" Devicecache dump start");
    HDF_LOGE("--------------------------------");

    deviceCaches_->TravelCaches([](int32_t id, const DeviceCache& cache)->void {
        cache.Dump();
    });

    HDF_LOGE("--------------------------------");
    HDF_LOGE("  Devicecache dump end");
    HDF_LOGE("********************************");
}

int32_t DeviceCacheManager::AddCacheInternal(uint32_t deviceId, DeviceCache::DeviceType type)
{
    DeviceCache* device = DeviceCache::Create(deviceId, type);
    DISPLAY_CHK_RETURN(device == nullptr, HDF_FAILURE, HDF_LOGE("%{public}s: Create cache failed", __func__));

    bool ret = deviceCaches_->InsertCache(deviceId, device);
    DISPLAY_CHK_RETURN(ret == false, HDF_FAILURE, HDF_LOGE("%{public}s: insert device cache failed", __func__));

    return HDF_SUCCESS;
}

std::mutex& DeviceCacheManager::GetCacheMgrMutex()
{
    static std::mutex deviceCacheMgr;
    return deviceCacheMgr;
}
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
