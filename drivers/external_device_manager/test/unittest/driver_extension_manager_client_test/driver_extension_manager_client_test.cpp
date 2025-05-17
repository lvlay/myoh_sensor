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

#include <chrono>
#include <cinttypes>
#include <iostream>
#include <thread>
#include <gtest/gtest.h>
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "driver_ext_mgr_callback_stub.h"
#include "driver_ext_mgr_client.h"
#include "edm_errors.h"
#include "ext_object.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace testing::ext;

class DrvExtMgrClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

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

static LoadStatus g_loadStatus_ = LoadStatus::LOAD_FAILED;
static sptr<IRemoteObject> g_saObject = nullptr;
static constexpr int32_t ERROR_CODE_WITH_INVALID_CODE = 305;
static constexpr uint64_t START_SA_SERVICE_WAIT_TIME = 3;

void GetNativeToken()
{
    uint64_t tokenId;
    const char **perms = new const char *[1];
    perms[0] = "ohos.permission.ACCESS_EXTENSIONAL_DEVICE_DRIVER";

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .aplStr = "system_core",
    };

    infoInstance.processName = "TestCase";
    tokenId = GetAccessTokenId(&infoInstance);
    EXPECT_EQ(0, SetSelfTokenID(tokenId));
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;
}

void DrvExtMgrClientTest::SetUpTestCase()
{
    GetNativeToken();
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s get samgr failed", __func__);
        g_loadStatus_ = LoadStatus::LOAD_FAILED;
        return;
    }

    auto saObj = samgr->CheckSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
    if (saObj != nullptr) {
        g_saObject = saObj;
        g_loadStatus_ = LoadStatus::ALREADY_EXISTS;
        return;
    }

    sptr<LoadCallback> loadCallback_ = new LoadCallback();
    int32_t ret = samgr->LoadSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID, loadCallback_);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s load hdf_ext_devmgr failed", __func__);
        g_loadStatus_ = LoadStatus::LOAD_FAILED;
        return;
    }
}

void DrvExtMgrClientTest::TearDownTestCase()
{
    if (g_loadStatus_ == LoadStatus::LOAD_FAILED || g_loadStatus_ == LoadStatus::ALREADY_EXISTS) {
        return;
    }

    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s get samgr failed", __func__);
        return;
    }

    int32_t ret = samgr->UnloadSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s unload hdf_ext_devmgr failed, ret:%{public}d", __func__, ret);
    } else {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s unload hdf_ext_devmgr successlfuly", __func__);
    }
}

void DrvExtMgrClientTest::LoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    std::cout << "load success: systemAbilityId:" << systemAbilityId
              << " IRemoteObject result:" << ((remoteObject != nullptr) ? "succeed" : "failed") << std::endl;
    g_loadStatus_ = LoadStatus::LOAD_SUCCESS;
    g_saObject = remoteObject;
}

void DrvExtMgrClientTest::LoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    std::cout << "load failed: systemAbilityId:" << systemAbilityId << std::endl;
    g_loadStatus_ = LoadStatus::LOAD_FAILED;
    g_saObject = nullptr;
}

HWTEST_F(DrvExtMgrClientTest, CheckSAServiceLoad001, TestSize.Level1)
{
    if (g_loadStatus_ != LoadStatus::ALREADY_EXISTS) {
        std::this_thread::sleep_for(std::chrono::seconds(START_SA_SERVICE_WAIT_TIME));
    }

    ASSERT_NE(g_saObject, nullptr);
}

HWTEST_F(DrvExtMgrClientTest, QueryDevice001, TestSize.Level1)
{
    uint32_t busType = static_cast<uint32_t>(BusType::BUS_TYPE_INVALID);
    std::vector<std::shared_ptr<DeviceData>> devices;
    UsbErrCode ret = DriverExtMgrClient::GetInstance().QueryDevice(busType, devices);
    ASSERT_EQ(ret, UsbErrCode::EDM_ERR_INVALID_PARAM);
    ASSERT_TRUE(devices.empty());
}

HWTEST_F(DrvExtMgrClientTest, QueryDevice002, TestSize.Level1)
{
    uint32_t busType = static_cast<uint32_t>(BusType::BUS_TYPE_USB);
    std::vector<std::shared_ptr<DeviceData>> devices;
    UsbErrCode ret = DriverExtMgrClient::GetInstance().QueryDevice(busType, devices);
    ASSERT_EQ(ret, UsbErrCode::EDM_OK);
    std::cout << "size of devices:" << devices.size() << std::endl;
    for (const auto &device : devices) {
        std::cout << device->Dump() << std::endl;
    }
}

HWTEST_F(DrvExtMgrClientTest, BindDevice001, TestSize.Level1)
{
    uint64_t deviceId = 0;
    sptr<IDriverExtMgrCallback> connectCallback = nullptr;
    UsbErrCode ret = DriverExtMgrClient::GetInstance().BindDevice(deviceId, connectCallback);
    ASSERT_EQ(ret, UsbErrCode::EDM_ERR_INVALID_PARAM);
}

class DriverExtMgrCallbackTest : public DriverExtMgrCallbackStub {
public:
    void OnConnect(uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj, const ErrMsg &errMsg) override;

    void OnDisconnect(uint64_t deviceId, const ErrMsg &errMsg) override;

    void OnUnBind(uint64_t deviceId, const ErrMsg &errMsg) override;
};

void DriverExtMgrCallbackTest::OnConnect(uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj, const ErrMsg &errMsg)
{
    EDM_LOGE(EDM_MODULE_TEST, "ErrMsg:%{public}d:%{public}s, deviceId:%{public}016" PRIX64 "",
        static_cast<UsbErrCode>(errMsg.errCode), errMsg.msg.c_str(), deviceId);
    std::cout << "OnConnect {errCode:" << static_cast<UsbErrCode>(errMsg.errCode) << ", ";
    std::cout << "msg:" << errMsg.msg << ", ";
    std::cout << "deviceId:" << deviceId << "}" << std::endl;
}

void DriverExtMgrCallbackTest::OnDisconnect(uint64_t deviceId, const ErrMsg &errMsg)
{
    EDM_LOGE(EDM_MODULE_TEST, "ErrMsg:%{public}d:%{public}s, deviceId:%{public}016" PRIX64 "",
        static_cast<UsbErrCode>(errMsg.errCode), errMsg.msg.c_str(), deviceId);
    std::cout << "OnDisconnect {errCode:" << static_cast<UsbErrCode>(errMsg.errCode) << ", ";
    std::cout << "msg:" << errMsg.msg << ", ";
    std::cout << "deviceId:" << deviceId << "}" << std::endl;
}

void DriverExtMgrCallbackTest::OnUnBind(uint64_t deviceId, const ErrMsg &errMsg)
{
    EDM_LOGE(EDM_MODULE_TEST, "ErrMsg:%{public}d:%{public}s, deviceId:%{public}016" PRIX64 "",
        static_cast<UsbErrCode>(errMsg.errCode), errMsg.msg.c_str(), deviceId);
    std::cout << "OnUnBind {errCode:" << static_cast<UsbErrCode>(errMsg.errCode) << ", ";
    std::cout << "msg:" << errMsg.msg << ", ";
    std::cout << "deviceId:" << deviceId << "}" << std::endl;
}

HWTEST_F(DrvExtMgrClientTest, BindDevice002, TestSize.Level1)
{
    uint64_t deviceId = 0;
    sptr<IDriverExtMgrCallback> connectCallback = new DriverExtMgrCallbackTest {};
    UsbErrCode ret = DriverExtMgrClient::GetInstance().BindDevice(deviceId, connectCallback);
    ASSERT_EQ(ret, UsbErrCode::EDM_NOK);
}

HWTEST_F(DrvExtMgrClientTest, UnBindDevice001, TestSize.Level1)
{
    uint64_t deviceId = 0;
    UsbErrCode ret = DriverExtMgrClient::GetInstance().UnBindDevice(deviceId);
    ASSERT_EQ(ret, UsbErrCode::EDM_NOK);
}

HWTEST_F(DrvExtMgrClientTest, InvalidCode001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(samgr, nullptr);

    auto saObj = samgr->CheckSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
    ASSERT_NE(saObj, nullptr);

    uint32_t invalid_code = static_cast<uint32_t>(DriverExtMgrInterfaceCode::INVALID_CODE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ASSERT_TRUE(data.WriteInterfaceToken(IDriverExtMgr::GetDescriptor()));
    int32_t ret = saObj->SendRequest(invalid_code, data, reply, option);
    ASSERT_EQ(ret, ERROR_CODE_WITH_INVALID_CODE);
}
} // namespace ExternalDeviceManager
} // namespace OHOS