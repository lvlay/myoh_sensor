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
#include <thread>
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "system_ability_load_callback_stub.h"
#include "hilog_wrapper.h"
#define private public
#include "driver_pkg_manager.h"
#include "ibus_extension.h"
#include "usb_device_info.h"
#undef private

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing::ext;
using namespace OHOS::ExternalDeviceManager;

class DriverPkgManagerTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
    static void SetUpTestCase();
    static void TearDownTestCase();
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

static LoadStatus g_loadStatus = LoadStatus::LOAD_FAILED;
static sptr<IRemoteObject> g_saObject = nullptr;
static constexpr uint64_t START_SA_SERVICE_WAIT_TIME = 3;

void DriverPkgManagerTest::SetUpTestCase()
{
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s get samgr failed", __func__);
        g_loadStatus = LoadStatus::LOAD_FAILED;
        return;
    }
    auto saObj = samgr->CheckSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
    if (saObj != nullptr) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s CheckSystemAbility failed", __func__);
        g_saObject = saObj;
        g_loadStatus = LoadStatus::ALREADY_EXISTS;
        EDM_LOGI(EDM_MODULE_TEST, " hdf_ext_devmgr is exits");
        return;
    }
    sptr<LoadCallback> loadCallback_ = new LoadCallback();
    int32_t ret = samgr->LoadSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID, loadCallback_);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s load hdf_ext_devmgr failed", __func__);
        g_loadStatus = LoadStatus::LOAD_FAILED;
        return;
    }
    if (g_loadStatus != LoadStatus::ALREADY_EXISTS) {
        std::this_thread::sleep_for(std::chrono::seconds(START_SA_SERVICE_WAIT_TIME));
    }
}

void DriverPkgManagerTest::TearDownTestCase()
{
    if (g_loadStatus == LoadStatus::LOAD_FAILED || g_loadStatus == LoadStatus::ALREADY_EXISTS) {
        EDM_LOGE(EDM_MODULE_TEST, "g_loadStatus = %{public}d ", g_loadStatus);
        return;
    }

    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s get samgr failed", __func__);
        return;
    }

    int32_t ret = samgr->UnloadSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
    if (ret == UsbErrCode::EDM_OK) {
        EDM_LOGI(EDM_MODULE_TEST, "UnloadSystemAbility success");
    } else {
        EDM_LOGI(EDM_MODULE_TEST, "UnloadSystemAbility failed");
    }
}

void DriverPkgManagerTest::LoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    EDM_LOGI(EDM_MODULE_TEST, "enter test: OnLoadSystemAbilitySuccess ");
    std::cout << "load success: systemAbilityId:" << systemAbilityId
              << " IRemoteObject result:" << ((remoteObject != nullptr) ? "succeed" : "failed") << std::endl;
    g_loadStatus = LoadStatus::LOAD_SUCCESS;
    g_saObject = remoteObject;
}

void DriverPkgManagerTest::LoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    EDM_LOGI(EDM_MODULE_TEST, "enter test: OnLoadSystemAbilityFail ");
    std::cout << "load failed: systemAbilityId:" << systemAbilityId << std::endl;
    g_loadStatus = LoadStatus::LOAD_FAILED;
    g_saObject = nullptr;
}

HWTEST_F(DriverPkgManagerTest, DrvExt_Init_Test, TestSize.Level1)
{
    DriverPkgManager &drvPkgMgrInstance = DriverPkgManager::GetInstance();
    bool ret = drvPkgMgrInstance.Init();
    EXPECT_EQ(0, ret);
    cout << "DrvExt_Init_New_BundleTest" << endl;
}

int32_t Fun(int a, int b, const string & c, const string & d)
{
    cout << a << endl;
    cout << b << endl;
    cout << c << endl;
    cout << d << endl;
    return EDM_OK;
}

HWTEST_F(DriverPkgManagerTest, DrvExt_RegisterOnBundleUpdate_Init_Test, TestSize.Level1)
{
    DriverPkgManager &drvPkgMgrInstance = DriverPkgManager::GetInstance();
    int32_t regist = drvPkgMgrInstance.RegisterOnBundleUpdate(Fun);
    EXPECT_EQ(0, regist);
    cout << "DrvExt_RegisterOnBundleUpdate_Init_Test" << endl;
}

HWTEST_F(DriverPkgManagerTest, DrvExt_QueryMatch_Illegal_Bus_Test, TestSize.Level1)
{
    DriverPkgManager &drvPkgMgrInstance = DriverPkgManager::GetInstance();
    std::shared_ptr<DeviceInfo> devInfo = std::make_shared<DeviceInfo>(1);
    devInfo->devInfo_.devBusInfo.busType = BUS_TYPE_INVALID;
    std::shared_ptr<BundleInfoNames> bundle = drvPkgMgrInstance.QueryMatchDriver(devInfo);
    EXPECT_EQ(nullptr, bundle);
    cout << "DrvExt_QueryMatch_Illegal_Bus_Test" << endl;
}

class DriverPkgManagerPtrTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(DriverPkgManagerPtrTest, DrvExt_QueryMatch_Before_Init_Test, TestSize.Level1)
{
    DriverPkgManager &drvPkgMgrInstance = DriverPkgManager::GetInstance();
    std::shared_ptr<DeviceInfo> devInfo = std::make_shared<DeviceInfo>(0);
    std::shared_ptr<BundleInfoNames> bundle = drvPkgMgrInstance.QueryMatchDriver(devInfo);
    EXPECT_EQ(nullptr, bundle);
    cout << "DrvExt_QueryMatch_Before_Init_Test" << endl;
}

HWTEST_F(DriverPkgManagerPtrTest, DrvExt_New_Test, TestSize.Level1)
{
    DriverPkgManager &drvPkgMgrInstance = DriverPkgManager::GetInstance();
    bool ret = drvPkgMgrInstance.Init();
    if (ret != 0) {
        EXPECT_EQ(0, ret);
        return;
    }
    EXPECT_NE(nullptr, &drvPkgMgrInstance);
    cout << "DrvExt_New_Test" << endl;
}

HWTEST_F(DriverPkgManagerPtrTest, DrvExt_Delete_Test, TestSize.Level1)
{
    auto drvPkgMgrInstance = &(DriverPkgManager::GetInstance());
    if (drvPkgMgrInstance != nullptr) {
        delete drvPkgMgrInstance;
        drvPkgMgrInstance = nullptr;
        EXPECT_EQ(nullptr, drvPkgMgrInstance);
    }
    cout << "DrvExt_Delete_Test" << endl;
}
}
}