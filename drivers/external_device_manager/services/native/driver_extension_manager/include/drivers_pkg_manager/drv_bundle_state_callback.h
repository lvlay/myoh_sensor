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

#ifndef DRIVER_BUNDLE_STATUS_CALLBACK_H
#define DRIVER_BUNDLE_STATUS_CALLBACK_H

#include <stdint.h>
#include <vector>
#include <map>
#include <iostream>

#include "bundle_info.h"
#include "bundle_mgr_proxy.h"
#include "extension_ability_info.h"
#include "ibus_extension.h"
#include "pkg_tables.h"
#include "ibundle_update_callback.h"
#include <future>
namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::ExternalDeviceManager;

enum {
    ERR_DRV_STATUS_CALLBACK_ERROR = 1,
};

enum ON_BUNDLE_STATUS {
    BUNDLE_NULL,
    BUNDLE_ADDED = 1,
    BUNDLE_UPDATED,
    BUNDLE_REMOVED,
};

typedef int32_t(*PCALLBACKFUN)(int, int, const string &, const string &);

class DrvBundleStateCallback : public IBundleStatusCallback {
public:
    DrvBundleStateCallback();
    DrvBundleStateCallback(shared_future<int32_t> bmsFuture, shared_future<int32_t> accountFuture,
        shared_future<int32_t> commEventFuture);
    ~DrvBundleStateCallback();

    void PrintTest();

    virtual void OnBundleStateChanged(const uint8_t installType, const int32_t resultCode,
        const std::string &resultMsg, const std::string &bundleName) override;

    /**
     * @brief Called when a new application package has been installed on the device.
     * @param bundleName Indicates the name of the bundle whose state has been installed.
     * @param userId Indicates the id of the bundle whose state has been installed.
     */
    virtual void OnBundleAdded(const std::string &bundleName, const int userId) override;
    /**
     * @brief Called when a new application package has been Updated on the device.
     * @param bundleName Indicates the name of the bundle whose state has been Updated.
     * @param userId Indicates the id of the bundle whose state has been Updated.
     */
    virtual void OnBundleUpdated(const std::string &bundleName, const int userId) override;
    /**
     * @brief Called when a new application package has been Removed on the device.
     * @param bundleName Indicates the name of the bundle whose state has been Removed.
     * @param userId Indicates the id of the bundle whose state has been Removed.
     */
    virtual void OnBundleRemoved(const std::string &bundleName, const int userId) override;
    
    virtual sptr<IRemoteObject> AsObject() override;

    bool GetAllDriverInfos(bool isExecCallback = true);
    void GetAllDriverInfosAsync(bool isExecCallback = true);

    bool CheckBundleMgrProxyPermission();

    string GetStiching();

    PCALLBACKFUN m_pFun = nullptr;
    std::shared_ptr<IBundleUpdateCallback> bundleUpdateCallback_ = nullptr;

    void ResetInitOnce();
    void ResetMatchedBundles(const int32_t userId);

private:
    std::mutex bundleMgrMutex_;
    std::mutex initOnceMutex_;
    sptr<IBundleMgr> bundleMgr_ = nullptr;
    string stiching = "This is used for Name Stiching";
    bool initOnce = false;

    shared_future<int32_t> bmsFuture_;
    shared_future<int32_t> accountFuture_;
    shared_future<int32_t> commEventFuture_;

    bool QueryDriverInfos(const std::string &bundleName, const int userId,
        std::vector<ExtensionAbilityInfo> &driverInfos);
    bool UpdateToRdb(const std::vector<ExtensionAbilityInfo> &driverInfos, const std::string &bundleName = "",
        bool isExecCallback = true);
    void ClearDriverInfo(DriverInfo &tmpDrvInfo);
    sptr<OHOS::AppExecFwk::IBundleMgr> GetBundleMgrProxy();
    int32_t GetCurrentActiveUserId();
    void ChangeValue(DriverInfo &tmpDrvInfo, const map<string, string> &metadata);
    std::string GetBundleSize(const std::string &bundleName);
    void ParseToPkgInfoTables(
        const std::vector<ExtensionAbilityInfo> &driverInfos, std::vector<PkgInfoTable> &pkgInfoTables);
    PkgInfoTable CreatePkgInfoTable(const ExtensionAbilityInfo &driverInfo, string driverInfoStr);
    bool IsCurrentUserId(const int userId);

    void OnBundleDrvAdded(int bundleStatus);
    void OnBundleDrvUpdated(int bundleStatus);
    void OnBundleDrvRemoved(const std::string &bundleName);
};
} // namespace
}
#endif // DRIVER_BUNDLE_STATUS_CALLBACK_H