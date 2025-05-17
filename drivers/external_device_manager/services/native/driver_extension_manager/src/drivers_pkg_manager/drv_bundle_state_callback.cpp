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

#include "drv_bundle_state_callback.h"

#include <want.h>
#include <element_name.h>

#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "bundle_constants.h"
#include "os_account_manager.h"
#include  "pkg_db_helper.h"

#include "hdf_log.h"
#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "bus_extension_core.h"
#include "accesstoken_kit.h"
#include <unordered_map>
#include <pthread.h>
#include <thread>
namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::ExternalDeviceManager;
using namespace OHOS::Security::AccessToken;

constexpr uint64_t LABEL = HITRACE_TAG_OHOS;
const string DRV_INFO_BUS = "bus";
const string DRV_INFO_VENDOR = "vendor";
const string DRV_INFO_DESC = "description";

static constexpr const char *BUNDLE_RESET_TASK_NAME = "DRIVER_INFO_RESET";
static constexpr const char *BUNDLE_UPDATE_TASK_NAME = "DRIVER_INFO_UPDATE";
static constexpr const char *GET_DRIVERINFO_TASK_NAME = "GET_DRIVERINFO_ASYNC";

std::string DrvBundleStateCallback::GetBundleSize(const std::string &bundleName)
{
    std::string bundleSize = "";
    auto iBundleMgr = GetBundleMgrProxy();
    if (iBundleMgr == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "Can not get iBundleMgr");
        return bundleSize;
    }
    int32_t userId = GetCurrentActiveUserId();
    std::vector<int64_t> bundleStats;
    if (!iBundleMgr->GetBundleStats(bundleName, userId, bundleStats)) {
        EDM_LOGE(MODULE_PKG_MGR, "GetBundleStats failed");
        return bundleSize;
    }
    if (!bundleStats.empty()) {
        bundleSize.append(std::to_string(bundleStats[0]));
    }
    return bundleSize;
}

void DrvBundleStateCallback::ChangeValue(DriverInfo &tmpDrvInfo, const map<string, string> &metadata)
{
    for (auto data : metadata) {
        if (data.first == DRV_INFO_BUS) {
            tmpDrvInfo.bus_ = data.second;
        }
        if (data.first == DRV_INFO_VENDOR) {
            tmpDrvInfo.vendor_ = data.second;
        }
        if (data.first == DRV_INFO_DESC) {
            tmpDrvInfo.description_ = data.second;
        }
    }
}

void DrvBundleStateCallback::ParseToPkgInfoTables(const std::vector<ExtensionAbilityInfo> &driverInfos,
    std::vector<PkgInfoTable> &pkgInfoTables)
{
    std::unordered_map<std::string, std::string> bundlesSize;
    shared_ptr<IBusExtension> extInstance = nullptr;
    for (const auto &driverInfo : driverInfos) {
        if (driverInfo.type != ExtensionAbilityType::DRIVER || driverInfo.metadata.empty()) {
            continue;
        }
        DriverInfo tmpDrvInfo(driverInfo.bundleName, driverInfo.name);
        map<string, string> metaMap;
        for (auto meta : driverInfo.metadata) {
            metaMap.emplace(meta.name, meta.value);
        }
        ChangeValue(tmpDrvInfo, metaMap);
        extInstance = BusExtensionCore::GetInstance().GetBusExtensionByName(tmpDrvInfo.GetBusName());
        if (extInstance == nullptr) {
            EDM_LOGE(MODULE_PKG_MGR, "GetBusExtensionByName failed, bus:%{public}s", tmpDrvInfo.bus_.c_str());
            continue;
        }
        tmpDrvInfo.driverInfoExt_ = extInstance->ParseDriverInfo(metaMap);
        if (tmpDrvInfo.driverInfoExt_ == nullptr) {
            EDM_LOGE(MODULE_PKG_MGR, "ParseDriverInfo null");
            continue;
        }

        if (bundlesSize.find(driverInfo.bundleName) == bundlesSize.end()) {
            std::string bundleSize = GetBundleSize(driverInfo.bundleName);
            bundlesSize.emplace(driverInfo.bundleName, bundleSize);
        }

        tmpDrvInfo.driverSize_ = bundlesSize[driverInfo.bundleName];
        tmpDrvInfo.version_ = driverInfo.applicationInfo.versionName;
        string driverInfoStr;
        if (tmpDrvInfo.Serialize(driverInfoStr) != EDM_OK) {
            EDM_LOGE(MODULE_PKG_MGR, "Serialize driverInfo faild");
            continue;
        }

        PkgInfoTable pkgInfo = CreatePkgInfoTable(driverInfo, driverInfoStr);
        pkgInfoTables.emplace_back(pkgInfo);
    }
}

PkgInfoTable DrvBundleStateCallback::CreatePkgInfoTable(const ExtensionAbilityInfo &driverInfo, string driverInfoStr)
{
    PkgInfoTable pkgInfo = {
        .driverUid = driverInfo.name + "-" + std::to_string(driverInfo.applicationInfo.accessTokenId),
        .bundleAbility = driverInfo.bundleName + "-" + driverInfo.name,
        .bundleName = driverInfo.bundleName,
        .driverName = driverInfo.name,
        .driverInfo = driverInfoStr
    };
    HapTokenInfo hapTokenInfo;
    if (AccessTokenKit::GetHapTokenInfo(driverInfo.applicationInfo.accessTokenId, hapTokenInfo)
        == AccessTokenKitRet::RET_SUCCESS) {
        pkgInfo.userId = hapTokenInfo.userID;
        pkgInfo.appIndex = hapTokenInfo.instIndex;
        EDM_LOGD(MODULE_PKG_MGR, "userId:%{public}d, appIndex:%{public}d",
            hapTokenInfo.userID, hapTokenInfo.instIndex);
    }
    return pkgInfo;
}

DrvBundleStateCallback::DrvBundleStateCallback()
{
    stiching.clear();
    stiching += "-";
};

DrvBundleStateCallback::DrvBundleStateCallback(shared_future<int32_t> bmsFuture, shared_future<int32_t> accountFuture,
    shared_future<int32_t> commEventFuture): DrvBundleStateCallback()
{
    bmsFuture_ = bmsFuture;
    accountFuture_ = accountFuture;
    commEventFuture_ = commEventFuture;
}

DrvBundleStateCallback::~DrvBundleStateCallback()
{
    return;
};

void DrvBundleStateCallback::PrintTest()
{
    std::vector<std::string> allBundleAbilityNames;
    std::shared_ptr<PkgDbHelper> helper = PkgDbHelper::GetInstance();
    int32_t ret = helper->QueryAllSize(allBundleAbilityNames);
    if (ret <= 0) {
        EDM_LOGE(MODULE_PKG_MGR, "QueryAllSize failed");
        return;
    }
    cout << "allBundleAbilityNames_ size = " << allBundleAbilityNames.size() << endl;
}

void DrvBundleStateCallback::OnBundleStateChanged(const uint8_t installType, const int32_t resultCode,
    const std::string &resultMsg, const std::string &bundleName)
{
    EDM_LOGI(MODULE_PKG_MGR, "OnBundleStateChanged");
    return;
};

/**
    * @brief Called when a new application package has been installed on the device.
    * @param bundleName Indicates the name of the bundle whose state has been installed.
    * @param userId Indicates the id of the bundle whose state has been installed.
    */
void DrvBundleStateCallback::OnBundleAdded(const std::string &bundleName, const int userId)
{
    EDM_LOGI(MODULE_PKG_MGR, "OnBundleAdded");
    StartTrace(LABEL, "OnBundleAdded");
    if (!IsCurrentUserId(userId)) {
        return;
    }
    std::vector<ExtensionAbilityInfo> driverInfos;
    if (!QueryDriverInfos(bundleName, userId, driverInfos) || driverInfos.empty()) {
        return;
    }

    if (!UpdateToRdb(driverInfos, bundleName)) {
        EDM_LOGE(MODULE_PKG_MGR, "OnBundleAdded error");
    }
    FinishTrace(LABEL);
}
/**
    * @brief Called when a new application package has been Updated on the device.
    * @param bundleName Indicates the name of the bundle whose state has been Updated.
    * @param userId Indicates the id of the bundle whose state has been Updated.
    */
void DrvBundleStateCallback::OnBundleUpdated(const std::string &bundleName, const int userId)
{
    EDM_LOGI(MODULE_PKG_MGR, "OnBundleUpdated");
    StartTrace(LABEL, "OnBundleUpdated");
    if (!IsCurrentUserId(userId)) {
        return;
    }
    std::vector<ExtensionAbilityInfo> driverInfos;
    if (!QueryDriverInfos(bundleName, userId, driverInfos)) {
        return;
    }

    if (driverInfos.empty()) {
        OnBundleDrvRemoved(bundleName);
        return;
    }

    if (!UpdateToRdb(driverInfos, bundleName)) {
        EDM_LOGE(MODULE_PKG_MGR, "OnBundleUpdated error");
    }
    FinishTrace(LABEL);
}

/**
    * @brief Called when a new application package has been Removed on the device.
    * @param bundleName Indicates the name of the bundle whose state has been Removed.
    * @param userId Indicates the id of the bundle whose state has been Removed.
    */
void DrvBundleStateCallback::OnBundleRemoved(const std::string &bundleName, const int userId)
{
    EDM_LOGI(MODULE_PKG_MGR, "OnBundleRemoved");
    StartTrace(LABEL, "OnBundleRemoved");
    if (!IsCurrentUserId(userId)) {
        return;
    }
    OnBundleDrvRemoved(bundleName);
    FinishTrace(LABEL);
}

sptr<IRemoteObject> DrvBundleStateCallback::AsObject()
{
    return nullptr;
}

void DrvBundleStateCallback::ResetInitOnce()
{
    std::lock_guard<std::mutex> lock(initOnceMutex_);
    initOnce = false;
}

void DrvBundleStateCallback::ResetMatchedBundles(const int32_t userId)
{
    if (bundleUpdateCallback_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "DrvBundleStateCallback::ResetMatchedBundles bundleUpdateCallback_ is null");
        return;
    }
    std::thread taskThread([userId, this]() {
        bundleUpdateCallback_->OnBundlesReseted(userId);
    });
    pthread_setname_np(taskThread.native_handle(), BUNDLE_RESET_TASK_NAME);
    taskThread.detach();
}

bool DrvBundleStateCallback::GetAllDriverInfos(bool isExecCallback)
{
    std::lock_guard<std::mutex> lock(initOnceMutex_);
    if (initOnce) {
        EDM_LOGI(MODULE_PKG_MGR, "GetAllDriverInfos has inited");
        return true;
    }
    // query history bundle
    auto iBundleMgr = GetBundleMgrProxy();
    if (iBundleMgr == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "Can not get iBundleMgr");
        return false;
    }
    std::vector<ExtensionAbilityInfo> driverInfos;
    int32_t userId = GetCurrentActiveUserId();
    if (userId == Constants::INVALID_USERID) {
        EDM_LOGI(MODULE_PKG_MGR, "GetCurrentActiveUserId userId is invalid");
        return false;
    }
    EDM_LOGI(MODULE_PKG_MGR, "QueryExtensionAbilityInfos userId:%{public}d", userId);
    iBundleMgr->QueryExtensionAbilityInfos(ExtensionAbilityType::DRIVER, userId, driverInfos);
    if (!UpdateToRdb(driverInfos, "", isExecCallback)) {
        EDM_LOGE(MODULE_PKG_MGR, "UpdateToRdb failed");
        return false;
    }
    initOnce = true;
    return true;
}

void DrvBundleStateCallback::GetAllDriverInfosAsync(bool isExecCallback)
{
    EDM_LOGI(MODULE_PKG_MGR, "GetAllDriverInfosAsync enter");
    std::thread taskThread([isExecCallback, this]() {
        bmsFuture_.wait();
        accountFuture_.wait();
        if (!GetAllDriverInfos(isExecCallback)) {
            EDM_LOGE(MODULE_PKG_MGR, "GetAllDriverInfos failed");
        }
    });
    pthread_setname_np(taskThread.native_handle(), GET_DRIVERINFO_TASK_NAME);
    taskThread.detach();
}

string DrvBundleStateCallback::GetStiching()
{
    return stiching;
}

bool DrvBundleStateCallback::CheckBundleMgrProxyPermission()
{
    // check permission
    auto iBundleMgr = GetBundleMgrProxy();
    if (iBundleMgr == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "Can not get iBundleMgr");
        return false;
    }
    if (!iBundleMgr->VerifySystemApi(Constants::INVALID_API_VERSION)) {
        EDM_LOGE(MODULE_PKG_MGR, "non-system app calling system api");
        return false;
    }
    if (!iBundleMgr->VerifyCallingPermission(Constants::LISTEN_BUNDLE_CHANGE)) {
        EDM_LOGE(MODULE_PKG_MGR, "register bundle status callback failed due to lack of permission");
        return false;
    }
    return true;
}

bool DrvBundleStateCallback::QueryDriverInfos(const std::string &bundleName, const int userId,
    std::vector<ExtensionAbilityInfo> &driverInfos)
{
    if (bundleName.empty()) {
        EDM_LOGE(MODULE_PKG_MGR, "BundleName empty");
        return false;
    }

    if (bundleMgr_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "BundleMgr_ nullptr");
        return false;
    }
 
    BundleInfo tmpBundleInfo;
    int32_t flags = static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) + \
                    static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) + \
                    static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA);
    if (!(bundleMgr_->GetBundleInfo(bundleName, flags, tmpBundleInfo, userId))) {
        EDM_LOGE(MODULE_PKG_MGR, "GetBundleInfo err");
        return false;
    }

    for (auto &extensionInfo : tmpBundleInfo.extensionInfos) {
        if (extensionInfo.type == ExtensionAbilityType::DRIVER) {
            extensionInfo.applicationInfo = tmpBundleInfo.applicationInfo;
            driverInfos.emplace_back(extensionInfo);
        }
    }
    return true;
}

void DrvBundleStateCallback::ClearDriverInfo(DriverInfo &tmpDrvInfo)
{
    tmpDrvInfo.bus_.clear();
    tmpDrvInfo.vendor_.clear();
    tmpDrvInfo.version_.clear();
    tmpDrvInfo.driverInfoExt_ = nullptr;
}

bool DrvBundleStateCallback::UpdateToRdb(const std::vector<ExtensionAbilityInfo> &driverInfos,
    const std::string &bundleName, bool isExecCallback)
{
    std::vector<PkgInfoTable> pkgInfoTables;
    ParseToPkgInfoTables(driverInfos, pkgInfoTables);
    std::shared_ptr<PkgDbHelper> helper = PkgDbHelper::GetInstance();
    if (helper->AddOrUpdatePkgInfo(pkgInfoTables, bundleName) < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "add or update failed,bundleName:%{public}s", bundleName.c_str());
        return false;
    }

    if (isExecCallback && bundleUpdateCallback_ != nullptr) {
        std::thread taskThread([bundleName, this]() {
            bundleUpdateCallback_->OnBundlesUpdated(bundleName);
        });
        pthread_setname_np(taskThread.native_handle(), BUNDLE_UPDATE_TASK_NAME);
        taskThread.detach();
    }
    return true;
}

int32_t DrvBundleStateCallback::GetCurrentActiveUserId()
{
    int32_t localId;
    int32_t ret = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(localId);
    if (ret != 0) {
        EDM_LOGE(MODULE_PKG_MGR, "GetForegroundOsAccountLocalId failed ret:%{public}d", ret);
        return Constants::INVALID_USERID;
    }
    return localId;
}

bool DrvBundleStateCallback::IsCurrentUserId(const int userId)
{
    return GetCurrentActiveUserId() == userId;
}

sptr<OHOS::AppExecFwk::IBundleMgr> DrvBundleStateCallback::GetBundleMgrProxy()
{
    if (bundleMgr_ == nullptr) {
        std::lock_guard<std::mutex> lock(bundleMgrMutex_);
        if (bundleMgr_ == nullptr) {
            auto systemAbilityManager = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (systemAbilityManager == nullptr) {
                EDM_LOGE(MODULE_PKG_MGR, "GetBundleMgr GetSystemAbilityManager is null");
                return nullptr;
            }
            auto bundleMgrSa = systemAbilityManager->GetSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
            if (bundleMgrSa == nullptr) {
                EDM_LOGE(MODULE_PKG_MGR, "GetBundleMgr GetSystemAbility is null");
                return nullptr;
            }
            auto bundleMgr = OHOS::iface_cast<IBundleMgr>(bundleMgrSa);
            if (bundleMgr == nullptr) {
                EDM_LOGE(MODULE_PKG_MGR, "GetBundleMgr iface_cast get null");
            }
            bundleMgr_ = bundleMgr;
        }
    }
    return bundleMgr_;
}

void DrvBundleStateCallback::OnBundleDrvRemoved(const std::string &bundleName)
{
    std::shared_ptr<PkgDbHelper> helper = PkgDbHelper::GetInstance();

    int32_t ret = helper->DeleteRightRecord(bundleName);
    if (ret < 0) {
        EDM_LOGE(MODULE_PKG_MGR, "delete failed: %{public}s", bundleName.c_str());
        return;
    }
    if (bundleUpdateCallback_ != nullptr) {
        std::thread taskThread([bundleName, this]() {
            bundleUpdateCallback_->OnBundlesUpdated(bundleName);
        });
        pthread_setname_np(taskThread.native_handle(), BUNDLE_UPDATE_TASK_NAME);
        taskThread.detach();
    }
}
}
}