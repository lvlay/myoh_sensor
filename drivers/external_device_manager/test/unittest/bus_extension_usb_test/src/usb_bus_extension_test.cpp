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
#include "json/json.h"
#include "hilog_wrapper.h"
#define private public
#include "ibus_extension.h"
#include "usb_driver_info.h"
#include "usb_device_info.h"
#include "usb_bus_extension.h"
#undef private

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing::ext;

class UsbBusExtensionTest : public testing::Test {
public:
    void SetUp() override
    {
        EDM_LOGD(MODULE_BUS_USB, "UsbBusExtensionTest SetUp");
    }
    void TearDown() override
    {
        EDM_LOGD(MODULE_BUS_USB, "UsbBusExtensionTest TearDown");
    }
};

static const map<string, string> g_testMetaDatas = {
    {"bus", "usb"},
    {"desc", "test usb driver extension"},
    {"vendor", "testVendor"},
    {"pid", "0x1234,0x5678"},
    {"vid", "0x1111,0x2222"}
};
HWTEST_F(UsbBusExtensionTest, GetExtensionInstanceTest, TestSize.Level1)
{
    auto usbBus = make_shared<UsbBusExtension>();
    ASSERT_NE(usbBus, nullptr);
}

HWTEST_F(UsbBusExtensionTest, SetDevChangeCallbackTest, TestSize.Level1)
{
    auto usbBus = make_shared<UsbBusExtension>();
    ASSERT_NE(usbBus, nullptr);
    auto ret = usbBus->SetDevChangeCallback(nullptr);
    ASSERT_EQ(ret, 0);
}

HWTEST_F(UsbBusExtensionTest, ParseDriverInfoTest, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "PraseDriverInfoTest Start");
    auto usbBus = make_shared<UsbBusExtension>();
    ASSERT_NE(usbBus, nullptr);
    auto driverInfoExt = usbBus->ParseDriverInfo(g_testMetaDatas);
    EDM_LOGD(MODULE_BUS_USB, "parse driver info done");
    ASSERT_NE(driverInfoExt, nullptr);
    UsbDriverInfo *usbDriverinfo = static_cast<UsbDriverInfo*>(driverInfoExt.get());
    ASSERT_NE(usbDriverinfo, nullptr);
    ASSERT_EQ(usbDriverinfo->pids_.size(), (size_t)2);
    ASSERT_EQ(usbDriverinfo->vids_.size(), (size_t)2);
    ASSERT_EQ(usbDriverinfo->pids_[0], 0x1234);
    ASSERT_EQ(usbDriverinfo->pids_[1], 0x5678);
    ASSERT_EQ(usbDriverinfo->vids_[0], 0x1111);
    ASSERT_EQ(usbDriverinfo->vids_[1], 0x2222);
}

HWTEST_F(UsbBusExtensionTest, MatchDriverTest, TestSize.Level1)
{
    auto usbDrvInfo = make_shared<UsbDriverInfo>();
    usbDrvInfo->pids_.push_back(0x1234);
    usbDrvInfo->pids_.push_back(0x5678);
    usbDrvInfo->vids_.push_back(0x1111);
    usbDrvInfo->vids_.push_back(0x2222);
    auto drvInfo = make_shared<DriverInfo>();
    drvInfo->bus_ = "USB";
    drvInfo->vendor_ = "TestVendor";
    drvInfo->version_ = "0.1.1";
    drvInfo->driverInfoExt_ = usbDrvInfo;
    string drvInfoStr;
    EDM_LOGD(MODULE_BUS_USB, "build driverInfo Done");

    auto deviceInfo = make_shared<UsbDeviceInfo>(0);
    deviceInfo->devInfo_.devBusInfo.busType = BusType::BUS_TYPE_USB;
    deviceInfo->idProduct_ = 0x1234;
    deviceInfo->idVendor_ = 0x1111;
    deviceInfo->deviceClass_ = 0;
    deviceInfo->bcdUSB_ = 0x1122;

    auto usbBus = make_shared<UsbBusExtension>();
    bool isMatched = usbBus->MatchDriver(*drvInfo, *deviceInfo);
    ASSERT_EQ(isMatched, true);

    UsbDeviceInfo deviceInfo2 = *deviceInfo;
    deviceInfo2.idProduct_ = 0x9999;
    isMatched = usbBus->MatchDriver(*drvInfo, deviceInfo2);
    ASSERT_EQ(isMatched, false);

    UsbDeviceInfo deviceInfo3 = *deviceInfo;
    deviceInfo3.idVendor_ = 0x9999;
    isMatched = usbBus->MatchDriver(*drvInfo, deviceInfo3);
    ASSERT_EQ(isMatched, false);

    UsbDeviceInfo deviceInfo4 = *deviceInfo;
    deviceInfo4.devInfo_.devBusInfo.busType = BusType::BUS_TYPE_INVALID;
    isMatched = usbBus->MatchDriver(*drvInfo, deviceInfo4);
    ASSERT_EQ(isMatched, false);

    drvInfo->bus_ = BusType::BUS_TYPE_INVALID;
    isMatched = usbBus->MatchDriver(*drvInfo, *deviceInfo);
    ASSERT_EQ(isMatched, false);
}
}
}
