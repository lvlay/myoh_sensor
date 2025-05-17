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

#include <dlfcn.h>
#include <sstream>

#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "string_ex.h"
#include "bus_extension_core.h"

#ifdef __aarch64__
static constexpr const char *BUS_EXTENSION_SO_PATH = "/system/lib64";
#else
static constexpr const char *BUS_EXTENSION_SO_PATH = "/system/lib";
#endif
static constexpr const char *HDI_SO_SUFFIX = ".z.so";
static constexpr const char *HDI_SO_PREFIX = "lib";
static constexpr const char *USB_BUS_EXTENSION = "driver_extension_usb_bus";

namespace OHOS {
namespace ExternalDeviceManager {
IMPLEMENT_SINGLE_INSTANCE(BusExtensionCore);

std::unordered_map<std::string, BusType> BusExtensionCore::busTypeMap_ = {
    {"usb", BusType::BUS_TYPE_USB}
};

void BusExtensionCore::LoadBusExtensionLibs()
{
    for (BusType i = BUS_TYPE_USB; i < BUS_TYPE_MAX; i = (BusType)(i + 1)) {
        std::ostringstream libPath;
        libPath << BUS_EXTENSION_SO_PATH << "/" << HDI_SO_PREFIX;
        switch (i) {
            case BUS_TYPE_USB:
                libPath << USB_BUS_EXTENSION;
                break;
            default:
                EDM_LOGE(MODULE_DEV_MGR, "invalid bus type");
                continue;
        }
        libPath << HDI_SO_SUFFIX;
        char realPath[PATH_MAX + 1] = {0};
        if (realpath(libPath.str().c_str(), realPath) == nullptr) {
            EDM_LOGE(MODULE_DEV_MGR, "invalid so path %{public}s", realPath);
            continue;
        }
        void *handler = dlopen(realPath, RTLD_LAZY);
        if (handler == nullptr) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to dlopen  %{public}s, %{public}s", realPath, dlerror());
            continue;
        }
    }
}

int32_t BusExtensionCore::Init(std::shared_ptr<IDevChangeCallback> callback)
{
    int ret = EDM_OK;
    for (auto &iter : busExtensions_) {
        if (iter.second->SetDevChangeCallback(callback) != EDM_OK) {
            ret = EDM_NOK;
            EDM_LOGE(MODULE_DEV_MGR, "busExtension init failed, busType is %{public}d", iter.first);
        }
        EDM_LOGD(MODULE_DEV_MGR, "busExtension init successfully, busType is %{public}d", iter.first);
    }
    return ret;
}

int32_t BusExtensionCore::Register(BusType busType, std::shared_ptr<IBusExtension> busExtension)
{
    if (busExtensions_.size() == MAX_BUS_EXTENSIONS) {
        EDM_LOGE(MODULE_DEV_MGR, "the maximum number of busextension registrations has been reached");
        return EDM_NOK;
    }
    if (busExtensions_.find(busType) != busExtensions_.end()) {
        EDM_LOGI(MODULE_DEV_MGR, "busType %{public}d has been registered", busType);
        return EDM_OK;
    }
    busExtensions_.emplace(busType, busExtension);
    EDM_LOGD(MODULE_DEV_MGR, "busType %{public}d register successfully", busType);
    return EDM_OK;
}

BusType BusExtensionCore::GetBusTypeByName(const std::string &busName)
{
    auto iterMap = busTypeMap_.find(LowerStr(busName));
    if (iterMap == busTypeMap_.end()) {
        EDM_LOGE(MODULE_DEV_MGR, "invalid bus name: %{public}s", busName.c_str());
        return BusType::BUS_TYPE_INVALID;
    }
    return iterMap->second;
}

std::shared_ptr<IBusExtension> BusExtensionCore::GetBusExtensionByName(std::string busName)
{
    BusType busType = GetBusTypeByName(busName);
    if (busType <= BusType::BUS_TYPE_INVALID || busType >= BusType::BUS_TYPE_MAX) {
        return nullptr;
    }
    auto iterExtension = busExtensions_.find(busType);
    if (iterExtension == busExtensions_.end()) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s bus extension not found", busName.c_str());
        return nullptr;
    }
    return busExtensions_[busType];
}
} // namespace ExternalDeviceManager
} // namespace OHOS