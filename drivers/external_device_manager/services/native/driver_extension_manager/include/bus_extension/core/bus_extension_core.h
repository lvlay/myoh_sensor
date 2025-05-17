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
#ifndef BUS_EXTENSION_CORE_H
#define BUS_EXTENSION_CORE_H

#include <memory>
#include <unordered_map>
#include "ext_object.h"
#include "idev_change_callback.h"
#include "single_instance.h"
#include "ibus_extension.h"

namespace OHOS {
namespace ExternalDeviceManager {
class BusExtensionCore {
    DECLARE_SINGLE_INSTANCE_BASE(BusExtensionCore);

public:
    ~BusExtensionCore() = default;
    int32_t Init(std::shared_ptr<IDevChangeCallback> callback);
    int32_t Register(BusType busType, std::shared_ptr<IBusExtension> busExtension);
    std::shared_ptr<IBusExtension> GetBusExtensionByName(std::string busName);
    static BusType GetBusTypeByName(const std::string &busName);
    void LoadBusExtensionLibs();

private:
    BusExtensionCore() = default;
    std::unordered_map<BusType, std::shared_ptr<IBusExtension>> busExtensions_;
    const uint32_t MAX_BUS_EXTENSIONS = 100;
    static std::unordered_map<std::string, BusType> busTypeMap_;
};

// bus extension should register by __attribute__ ((constructor)) when loading so
template <typename BusExtension>
void RegisterBusExtension(BusType busType)
{
    BusExtensionCore::GetInstance().Register(busType, std::make_shared<BusExtension>());
}
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // BUS_EXTENSION_CORE_H