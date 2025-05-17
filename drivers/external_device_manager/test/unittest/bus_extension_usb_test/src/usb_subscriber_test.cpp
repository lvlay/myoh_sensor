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

#include "gtest/gtest.h"
#include "map"
#include "gmock/gmock.h"
#include "usb_impl_mock.h"
#include "usb_ddk_service_mock.h"
#define private public
#include "ibus_extension.h"
#include "usb_bus_extension.h"
#undef private
namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace ExternalDeviceManager;
using namespace OHOS::USB;

class UsbSubscriberTest : public testing::Test {
public:
    void SetUp() override
    {
        cout << "UsbSubscriberTest SetUp" << endl;
        busExt = make_shared<UsbBusExtension>();
        usbBusExt = static_cast<UsbBusExtension*>(busExt.get());
        mockUsb = sptr<UsbImplMock>(new UsbImplMock());
        mockUsbDdk = sptr<UsbDdkServiceMock>(new UsbDdkServiceMock());
        usbBusExt->SetUsbInferface(mockUsb);
        usbBusExt->SetUsbDdk(mockUsbDdk);
    }
    void TearDown() override
    {
        cout << "UsbSubscriberTest TearDown" << endl;
        mockUsb = nullptr;
        mockUsbDdk = nullptr;
    }

    shared_ptr<IBusExtension> busExt;
    sptr<UsbImplMock> mockUsb;
    sptr<UsbDdkServiceMock> mockUsbDdk;
    UsbBusExtension *usbBusExt;
};

class TestDevChangeCallback : public IDevChangeCallback {
public:
    map<uint64_t, shared_ptr<DeviceInfo>> devInfoMap;
    TestDevChangeCallback() { };
    ~TestDevChangeCallback() { };

    int32_t OnDeviceAdd(std::shared_ptr<DeviceInfo> device) override
    {
        this->devInfoMap[device->devInfo_.deviceId] = device;
        return 0;
    };
    int32_t OnDeviceRemove(std::shared_ptr<DeviceInfo> device) override
    {
        this->devInfoMap.erase(device->devInfo_.deviceId);
        return 1;
    };
};

const uint32_t ACT_DEVUP = 0;
const uint32_t ACT_DEVDOWN = 1;

HWTEST_F(UsbSubscriberTest, UsbDevCallbackTest, TestSize.Level1)
{
    auto testCb = make_shared<TestDevChangeCallback>();
    usbBusExt->SetDevChangeCallback(testCb);

    USBDeviceInfo info = {ACT_DEVUP, BUS_NUM_OK, DEV_ADDR_OK};
    EXPECT_EQ(testCb->devInfoMap.size(), (size_t)0);

    auto ret = mockUsb->SubscriberDeviceEvent(info);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(testCb->devInfoMap.size(), (size_t)1);

    // invalid devNum
    info.busNum = BUS_NUM_OK;
    info.devNum = 9999;
    ret = mockUsb->SubscriberDeviceEvent(info);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(testCb->devInfoMap.size(), (size_t)1);

    // valid param
    info.busNum = BUS_NUM_OK;
    info.devNum = DEV_ADDR_OK_2;
    ret = mockUsb->SubscriberDeviceEvent(info);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(testCb->devInfoMap.size(), (size_t)2);

    info.busNum = BUS_NUM_OK_2;
    info.devNum = DEV_ADDR_OK_2;
    ret = mockUsb->SubscriberDeviceEvent(info);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(testCb->devInfoMap.size(), (size_t)3);

    info.status=ACT_DEVDOWN;

    info.busNum = BUS_NUM_OK_2;
    info.devNum = DEV_ADDR_OK_2;
    ret = mockUsb->SubscriberDeviceEvent(info);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(testCb->devInfoMap.size(), (size_t)2);

    info.busNum = BUS_NUM_OK;
    info.devNum = DEV_ADDR_OK_2;
    ret = mockUsb->SubscriberDeviceEvent(info);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(testCb->devInfoMap.size(), (size_t)1);

    info.busNum = BUS_NUM_OK;
    info.devNum = DEV_ADDR_OK;
    ret = mockUsb->SubscriberDeviceEvent(info);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(testCb->devInfoMap.size(), (size_t)0);
}
HWTEST_F(UsbSubscriberTest, UsbDevCallbackErrorTest, TestSize.Level1)
{
    USBDeviceInfo info = {ACT_DEVUP, BUS_NUM_OK, DEV_ADDR_OK};
    int ret = 0;
    usbBusExt->SetDevChangeCallback(nullptr);
    // valid param, but error usb desc
    info.busNum = BUS_NUM_OK;
    info.devNum = DEV_ADDR_OK_ERR_DESC;
    ret = mockUsb->SubscriberDeviceEvent(info);
    EXPECT_NE(ret, 0);
    // valid param, but null usb desc
    info.busNum = BUS_NUM_OK;
    info.devNum = DEV_ADDR_OK_NULL_DESC;
    ret = mockUsb->SubscriberDeviceEvent(info);
    EXPECT_NE(ret, 0);

    // valid param, but null usb desc, action not support
    info.busNum = BUS_NUM_OK;
    info.devNum = DEV_ADDR_OK_NULL_DESC;
    info.status = 99;
    ret = mockUsb->SubscriberDeviceEvent(info);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(UsbSubscriberTest, PortChangEeventTest, TestSize.Level1)
{
    int ret = 0;
    usbBusExt->SetDevChangeCallback(nullptr);
    ret = mockUsb->SetPortRole(0, 0, 0);
    ASSERT_EQ(ret, 0);
}
}
}