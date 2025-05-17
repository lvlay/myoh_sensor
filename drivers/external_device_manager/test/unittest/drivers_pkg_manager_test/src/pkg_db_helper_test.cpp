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

#include <gtest/gtest.h>
#include <iostream>
#include "driver_ext_mgr_callback_stub.h"
#include "driver_ext_mgr_client.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "system_ability_load_callback_stub.h"
#include "ext_object.h"
#define private public
#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "pkg_db_helper.h"
#include "ibus_extension.h"
#include "usb_device_info.h"
#undef private

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing::ext;
using namespace OHOS::ExternalDeviceManager;

class PkgDbHelperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    static bool LoadSaService();
    static bool UnLoadSaService();

    void SetUp() override {}
    void TearDown() override {}
private:
    class LoadCallback : public SystemAbilityLoadCallbackStub {
    public:
        void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject) override;
        void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
    };
};

enum class LoadStatus {
    LOAD_SUCCESS,
    LOAD_FAILED,
    ALREADY_EXISTS,
};

static std::string g_bundleName = "testBundleName";
static std::string g_Ability = "testAbility";
static size_t g_expect_size = 1;
static LoadStatus g_loadStatus = LoadStatus::LOAD_FAILED;
static sptr<IRemoteObject> g_saObject = nullptr;
static constexpr uint64_t START_SA_SERVICE_WAIT_TIME = 3;
bool g_loadSaServiceSucc = false;
bool g_unLoadSaServiceSucc = false;

void PkgDbHelperTest::SetUpTestCase()
{
    int32_t ret = 0;
    g_loadSaServiceSucc = LoadSaService();
    if (g_loadSaServiceSucc) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s load hdf_ext_devmgr successlfully", __func__);
    } else {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s load hdf_ext_devmgr failed", __func__);
    }

    std::shared_ptr<PkgDbHelper> helper = PkgDbHelper::GetInstance();
    bool isUpdate = false;
    ret = helper->CheckIfNeedUpdateEx(isUpdate, g_Ability);
    if (!isUpdate) {
        return;
    }
    ret = helper->DeleteRightRecord(g_bundleName);
    if (ret != PKG_OK) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s delete pkg from db fail", __func__);
    }
}

void PkgDbHelperTest::TearDownTestCase()
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    bool isUpdate = false;
    int32_t ret = helper->CheckIfNeedUpdateEx(isUpdate, g_Ability);
    if (!isUpdate) {
        return;
    }
    ret = helper->DeleteRightRecord(g_bundleName);
    if (ret != PKG_OK) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s delete pkg from db fail", __func__);
    }

    g_unLoadSaServiceSucc = UnLoadSaService();
    if (g_unLoadSaServiceSucc) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s unload hdf_ext_devmgr successlfully", __func__);
    } else {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s unload hdf_ext_devmgr failed", __func__);
    }
}

bool PkgDbHelperTest::LoadSaService()
{
    int32_t ret = 0;
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s get samgr failed", __func__);
        g_loadStatus = LoadStatus::LOAD_FAILED;
        return false;
    }
    auto saObj = samgr->CheckSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
    if (saObj != nullptr) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s CheckSystemAbility failed", __func__);
        g_saObject = saObj;
        g_loadStatus = LoadStatus::ALREADY_EXISTS;
        EDM_LOGI(EDM_MODULE_TEST, " hdf_ext_devmgr is exits");
        return true;
    }
    sptr<LoadCallback> loadCallback_ = new LoadCallback();
    ret = samgr->LoadSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID, loadCallback_);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s load hdf_ext_devmgr failed", __func__);
        g_loadStatus = LoadStatus::LOAD_FAILED;
        return false;
    }
    if (g_loadStatus != LoadStatus::ALREADY_EXISTS) {
        std::this_thread::sleep_for(std::chrono::seconds(START_SA_SERVICE_WAIT_TIME));
    }
    if (g_saObject != nullptr) {
        return true;
    } else {
        return false;
    }
}

bool PkgDbHelperTest::UnLoadSaService()
{
    if (g_loadStatus == LoadStatus::LOAD_FAILED || g_loadStatus == LoadStatus::ALREADY_EXISTS) {
        EDM_LOGE(EDM_MODULE_TEST, "g_loadStatus = %{public}d ", g_loadStatus);
        return false;
    }

    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s get samgr failed", __func__);
        return false;
    }

    int32_t ret = samgr->UnloadSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
    if (ret != UsbErrCode::EDM_OK) {
        return false;
    } else {
        return true;
    }
}

void PkgDbHelperTest::LoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    EDM_LOGI(EDM_MODULE_TEST, "enter test: OnLoadSystemAbilitySuccess ");
    std::cout << "load success: systemAbilityId:" << systemAbilityId
              << " IRemoteObject result:" << ((remoteObject != nullptr) ? "succeed" : "failed") << std::endl;
    g_loadStatus = LoadStatus::LOAD_SUCCESS;
    g_saObject = remoteObject;
}

void PkgDbHelperTest::LoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    EDM_LOGI(EDM_MODULE_TEST, "enter test: OnLoadSystemAbilityFail ");
    std::cout << "load failed: systemAbilityId:" << systemAbilityId << std::endl;
    g_loadStatus = LoadStatus::LOAD_FAILED;
    g_saObject = nullptr;
}

HWTEST_F(PkgDbHelperTest, PkgDb_CheckIfNeedUpdateEx_Test, TestSize.Level1)
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    bool isUpdate = false;
    int32_t ret = helper->CheckIfNeedUpdateEx(isUpdate, g_Ability);
    EXPECT_EQ(false, isUpdate);
    EXPECT_EQ(0, ret);
    cout << "PkgDb_CheckIfNeedUpdateEx_Test" << endl;
}

HWTEST_F(PkgDbHelperTest, PkgDb_AddOrUpdateRightRecord_Test, TestSize.Level1)
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    string driverInfo = "{}";
    int32_t ret = helper->AddOrUpdateRightRecord(g_bundleName, g_Ability, driverInfo);
    EXPECT_EQ(0, ret);
    cout << "PkgDb_AddOrUpdateRightRecord_Test" << endl;
}

HWTEST_F(PkgDbHelperTest, PkgDb_QueryAllSize_Test, TestSize.Level1)
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    std::vector<std::string> allBundleAbilityNames;
    int32_t ret = helper->QueryAllSize(allBundleAbilityNames);
    EXPECT_LE(1, ret);
    EXPECT_LE(g_expect_size, allBundleAbilityNames.size());
    cout << "PkgDb_QueryAllSize_Test" << endl;
}

HWTEST_F(PkgDbHelperTest, PkgDb_QueryAllBundleAbilityNames_Test, TestSize.Level1)
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    std::vector<std::string> bundleAbilityNames;
    int32_t ret = helper->QueryAllBundleAbilityNames(g_bundleName, bundleAbilityNames);
    EXPECT_EQ(1, ret);
    EXPECT_EQ(g_expect_size, bundleAbilityNames.size());
    cout << "PkgDb_QueryAllBundleAbilityNames_Test" << endl;
}

HWTEST_F(PkgDbHelperTest, PkgDb_QueryAllDriverInfos_Test, TestSize.Level1)
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    std::vector<std::string> driverInfos;
    int32_t ret = helper->QueryAllDriverInfos(driverInfos);
    EXPECT_LE(1, ret);
    EXPECT_LE(g_expect_size, driverInfos.size());
    cout << "PkgDb_QueryAllDriverInfos_Test" << endl;
}

HWTEST_F(PkgDbHelperTest, PkgDb_QueryBundleInfoNames_Test, TestSize.Level1)
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    string driverInfo = "{}";
    string bundleName = helper->QueryBundleInfoNames(driverInfo);
    EXPECT_EQ("testAbility", bundleName);
    cout << "PkgDb_QueryBundleInfoNames_Test" << endl;
}

HWTEST_F(PkgDbHelperTest, PkgDb_QueryBundleInfoNames1_Test, TestSize.Level1)
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    string driverInfo = "{test}";
    string bundleName = helper->QueryBundleInfoNames(driverInfo);
    EXPECT_EQ("", bundleName);
    cout << "PkgDb_QueryBundleInfoNames1_Test" << endl;
}

HWTEST_F(PkgDbHelperTest, PkgDb_DeleteRightRecord_Test, TestSize.Level1)
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    int32_t ret = helper->DeleteRightRecord(g_bundleName);
    EXPECT_EQ(0, ret);
    cout << "PkgDb_DeleteRightRecord_Test" << endl;
}
}
}