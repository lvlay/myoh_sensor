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
#include "edm_errors.h"
#include "hilog_wrapper.h"
#define private public
#include "dev_change_callback.h"
#include "etx_device_mgr.h"
#include "ibus_extension.h"
#include "usb_bus_extension.h"
#include "bus_extension_core.h"
#include "driver_pkg_manager.h"
#undef private

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing::ext;

class DeviceManagerTest : public testing::Test {
public:
    void SetUp() override
    {
        EDM_LOGD(MODULE_DEV_MGR, "DeviceManagerTest SetUp");
    }
    void TearDown() override
    {
        EDM_LOGD(MODULE_DEV_MGR, "DeviceManagerTest TearDown");
    }
};

static void clearDeviceMap(ExtDeviceManager &instance)
{
    unordered_map<BusType, unordered_map<uint64_t, shared_ptr<Device>>> map;
    instance.deviceMap_ = map;
}

static size_t getDeviceNum(unordered_map<uint64_t, shared_ptr<Device>> map)
{
    size_t num = 0;
    for (auto &[_, device] : map) {
        if (!device->IsUnRegisted()) {
            num++;
        }
    }
    return num;
}

HWTEST_F(DeviceManagerTest, BusExtensionRegisterTest, TestSize.Level1)
{
    BusExtensionCore &core = BusExtensionCore::GetInstance();
    int32_t ret = core.Register(BusType::BUS_TYPE_USB, std::make_shared<UsbBusExtension>());
    ASSERT_EQ(ret, EDM_OK);
    ASSERT_NE(core.busExtensions_[BusType::BUS_TYPE_USB], nullptr);
}

HWTEST_F(DeviceManagerTest, InitTest, TestSize.Level1)
{
    DriverPkgManager::GetInstance().Init();
    int32_t ret = ExtDeviceManager::GetInstance().Init();
    ASSERT_EQ(ret, EDM_OK);
}

// test OnDeviceAdd and OnDeviceRemove
HWTEST_F(DeviceManagerTest, OnDeviceAddRemoveTest001, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    clearDeviceMap(extMgr);
    std::shared_ptr<DevChangeCallback> callback = std::make_shared<DevChangeCallback>();
    std::shared_ptr<DeviceInfo> device = std::make_shared<DeviceInfo>(0);
    device->devInfo_.devBusInfo.busType = BusType::BUS_TYPE_TEST;
    device->devInfo_.devBusInfo.busDeviceId = 1;
    int32_t ret = callback->OnDeviceAdd(device);
    ASSERT_EQ(ret, EDM_OK);
    ASSERT_EQ(getDeviceNum(extMgr.deviceMap_[BusType::BUS_TYPE_TEST]), 1);
    ret = callback->OnDeviceRemove(device);
    ASSERT_EQ(ret, EDM_OK);
    ASSERT_EQ(getDeviceNum(extMgr.deviceMap_[BusType::BUS_TYPE_TEST]), 0);
}

// test adding device repeatedly
HWTEST_F(DeviceManagerTest, OnDeviceAddRemoveTest002, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    clearDeviceMap(extMgr);
    std::shared_ptr<DevChangeCallback> callback = std::make_shared<DevChangeCallback>();
    std::shared_ptr<DeviceInfo> device = std::make_shared<DeviceInfo>(0);
    device->devInfo_.devBusInfo.busType = BusType::BUS_TYPE_TEST;
    device->devInfo_.devBusInfo.busDeviceId = 1;
    int32_t ret = callback->OnDeviceAdd(device);
    ASSERT_EQ(ret, EDM_OK);
    ret = callback->OnDeviceAdd(device);
    ASSERT_EQ(ret, EDM_OK);
    ASSERT_EQ(getDeviceNum(extMgr.deviceMap_[BusType::BUS_TYPE_TEST]), 1);
    ret = callback->OnDeviceRemove(device);
    ASSERT_EQ(ret, EDM_OK);
    ASSERT_EQ(getDeviceNum(extMgr.deviceMap_[BusType::BUS_TYPE_TEST]), 0);
    ret = callback->OnDeviceRemove(device);
    ASSERT_EQ(ret, EDM_OK);
}

HWTEST_F(DeviceManagerTest, OnDeviceAddRemoveTest003, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    clearDeviceMap(extMgr);
    std::shared_ptr<DevChangeCallback> callback = std::make_shared<DevChangeCallback>();
    std::shared_ptr<DeviceInfo> device0 = std::make_shared<DeviceInfo>(0);
    device0->devInfo_.devBusInfo.busType = BusType::BUS_TYPE_TEST;
    device0->devInfo_.devBusInfo.busDeviceId = 1;
    int32_t ret = callback->OnDeviceAdd(device0);
    ASSERT_EQ(ret, EDM_OK);
    std::shared_ptr<DeviceInfo> device1 = std::make_shared<DeviceInfo>(0);
    device1->devInfo_.devBusInfo.busType = BusType::BUS_TYPE_TEST;
    device1->devInfo_.devBusInfo.busDeviceId = 2;
    ret = callback->OnDeviceAdd(device1);
    ASSERT_EQ(ret, EDM_OK);
    ASSERT_EQ(getDeviceNum(extMgr.deviceMap_[BusType::BUS_TYPE_TEST]), 2);
    ret = callback->OnDeviceRemove(device1);
    ASSERT_EQ(ret, EDM_OK);
    ASSERT_EQ(getDeviceNum(extMgr.deviceMap_[BusType::BUS_TYPE_TEST]), 1);
    ret = callback->OnDeviceRemove(device0);
    ASSERT_EQ(ret, EDM_OK);
    ASSERT_EQ(getDeviceNum(extMgr.deviceMap_[BusType::BUS_TYPE_TEST]), 0);
}

HWTEST_F(DeviceManagerTest, QueryDeviceTest, TestSize.Level1)
{
    ExtDeviceManager &extMgr = ExtDeviceManager::GetInstance();
    clearDeviceMap(extMgr);
    std::vector<std::shared_ptr<DeviceInfo>> devVec = extMgr.QueryDevice(BUS_TYPE_TEST);
    ASSERT_EQ(devVec.size(), 0);
    std::shared_ptr<DevChangeCallback> callback = std::make_shared<DevChangeCallback>();
    std::shared_ptr<DeviceInfo> device0 = std::make_shared<DeviceInfo>(0);
    device0->devInfo_.devBusInfo.busType = BusType::BUS_TYPE_TEST;
    device0->devInfo_.devBusInfo.busDeviceId = 1;
    int32_t ret = callback->OnDeviceAdd(device0);
    ASSERT_EQ(ret, EDM_OK);
    std::shared_ptr<DeviceInfo> device1 = std::make_shared<DeviceInfo>(0);
    device1->devInfo_.devBusInfo.busType = BusType::BUS_TYPE_TEST;
    device1->devInfo_.devBusInfo.busDeviceId = 2;
    ret = callback->OnDeviceAdd(device1);
    ASSERT_EQ(ret, EDM_OK);
    devVec = extMgr.QueryDevice(BUS_TYPE_TEST);
    ASSERT_EQ(devVec.size(), 2);
    ret = callback->OnDeviceRemove(device0);
    ret = callback->OnDeviceRemove(device1);
    ASSERT_EQ(getDeviceNum(extMgr.deviceMap_[BusType::BUS_TYPE_TEST]), 0);
}

HWTEST_F(DeviceManagerTest, GetBusExtensionByNameTest, TestSize.Level1)
{
    BusExtensionCore &core = BusExtensionCore::GetInstance();
    ASSERT_NE(core.busExtensions_[BusType::BUS_TYPE_USB], nullptr);
    std::shared_ptr<IBusExtension> extension = core.GetBusExtensionByName("HDMI");
    ASSERT_EQ(extension, nullptr);
    extension = core.GetBusExtensionByName("USB");
    ASSERT_NE(extension, nullptr);
    core.busExtensions_.erase(BusType::BUS_TYPE_USB);
    extension = core.GetBusExtensionByName("USB");
    ASSERT_EQ(extension, nullptr);
}
} // namespace ExternalDeviceManager
} // namespace OHOS
