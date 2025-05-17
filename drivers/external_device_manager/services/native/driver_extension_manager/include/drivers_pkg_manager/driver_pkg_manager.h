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

#ifndef DRIVER_PKG_MANAGER_H
#define DRIVER_PKG_MANAGER_H

#include <iostream>
#include <stdint.h>

#include "bundle_monitor.h"
#include "drv_bundle_state_callback.h"
#include "ibus_extension.h"
#include "single_instance.h"
#include "ext_object.h"
#include <future>
namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::ExternalDeviceManager;

enum {
    ERR_DRV_PKG_MGR_ERROR = 1,
};

struct BundleInfoNames {
    string bundleName;
    string abilityName;
    string driverUid;
};

class DriverPkgManager {
    DECLARE_SINGLE_INSTANCE_BASE(DriverPkgManager);

public:
    void PrintTest();
    /**
     * @brief Called at first before Monitor DriverExtension Package.
     * @param bundleName Indicates the bundle name of the application.
     * @param launcherShortcutInfo List of LauncherShortcutInfo objects if obtained.
     * @return Returns true if the function is successfully called; returns false otherwise.
     */
    int32_t Init();
    int32_t Init(shared_future<int32_t> bmsFuture, shared_future<int32_t> accountFuture,
        shared_future<int32_t> commEventFuture);
    shared_ptr<BundleInfoNames> QueryMatchDriver(shared_ptr<DeviceInfo> devInfo);
    int32_t QueryDriverInfo(vector<shared_ptr<DriverInfo>> &driverInfos,
        bool isByDriverUid = false, const std::string &driverUid = "");
    int32_t RegisterOnBundleUpdate(PCALLBACKFUN pFun);
    int32_t RegisterBundleCallback(std::shared_ptr<IBundleUpdateCallback> callback);
    int32_t UnRegisterOnBundleUpdate();
    int32_t RegisterBundleStatusCallback();
    bool SubscribeOsAccountSwitch();
    ~DriverPkgManager();

private:
    shared_ptr<BundleMonitor> bundleMonitor_ = nullptr;
    sptr<DrvBundleStateCallback> bundleStateCallback_ = nullptr;
    BundleInfoNames bundleInfoName_;

    shared_future<int32_t> bmsFuture_;
    shared_future<int32_t> accountFuture_;
    shared_future<int32_t> commEventFuture_;

    int32_t UnRegisterBundleStatusCallback();
    DriverPkgManager();
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // DRIVER_PKG_MANAGER_H