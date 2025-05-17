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
using namespace ExternalDeviceManager;

class UsbDriverInfoTest : public testing::Test {
public:
    void SetUp() override
    {
        EDM_LOGD(MODULE_BUS_USB, "UsbDriverInfoTest SetUp");
    }
    void TearDown() override
    {
        EDM_LOGD(MODULE_BUS_USB, "UsbDriverInfoTest TearDown");
    }
};

HWTEST_F(UsbDriverInfoTest, SerializeThenUnSerializeTest, TestSize.Level1)
{
    int ret = 0;
    auto usbDrvInfo = make_shared<UsbDriverInfo>();
    usbDrvInfo->pids_.push_back(1234);
    usbDrvInfo->pids_.push_back(5678);
    usbDrvInfo->vids_.push_back(1111);
    usbDrvInfo->vids_.push_back(2222);
    auto drvInfo = make_shared<DriverInfo>();
    drvInfo->bus_ = "USB";
    drvInfo->vendor_ = "TestVendor";
    drvInfo->version_ = "0.1.1";
    drvInfo->driverInfoExt_ = usbDrvInfo;
    string drvInfoStr;
    EDM_LOGD(MODULE_BUS_USB, "begin to Serialize driverInfo");
    ret = drvInfo->Serialize(drvInfoStr);
    ASSERT_EQ(ret, 0);
    EDM_LOGI(MODULE_BUS_USB, "drvStr:%s", drvInfoStr.c_str());

    auto newDriverInfo = make_shared<DriverInfo>();
    ret = newDriverInfo->UnSerialize(drvInfoStr);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(newDriverInfo->bus_, "USB");
    ASSERT_EQ(newDriverInfo->vendor_, "TestVendor");
    ASSERT_EQ(newDriverInfo->version_, "0.1.1");
    UsbDriverInfo* newUsbDriverInfo = static_cast<UsbDriverInfo*>(newDriverInfo->driverInfoExt_.get());
    ASSERT_NE(newUsbDriverInfo, nullptr);
    ASSERT_EQ(newUsbDriverInfo->pids_.size(), (size_t)2);
    ASSERT_EQ(newUsbDriverInfo->vids_.size(), (size_t)2);
    ASSERT_EQ(newUsbDriverInfo->pids_[0], 1234);
    ASSERT_EQ(newUsbDriverInfo->pids_[1], 5678);
    ASSERT_EQ(newUsbDriverInfo->vids_[0], 1111);
    ASSERT_EQ(newUsbDriverInfo->vids_[1], 2222);
}

HWTEST_F(UsbDriverInfoTest, UnSerializeErrorTest, TestSize.Level1)
{
    int ret = 0;
    DriverInfo driverInfo;
    string drvStr;
    // valid json
    drvStr = "\{\"bus\":\"usb\",\"vendor\":\"TestVendor\",\"version\":\"0.0.1\",\
\"ext_info\":\"{\\\"vids\\\":[1111, 2222],\\\"pids\\\":[1234,4567]}\"}";
    ret = driverInfo.UnSerialize(drvStr);
    ASSERT_EQ(ret, 0);

    // invalid json, format error
    drvStr = "\{\"bus\"_\"usb\",\"vendor\":\"TestVendor\",\"version\":\"0.0.1\",\
\"ext_info\":\"{\\\"vids\\\":[1111, 2222],\\\"pids\\\":[1234,4567]}\"}";
    ret = driverInfo.UnSerialize(drvStr);
    ASSERT_NE(ret, 0);

    // invalid json, bus error
    drvStr = "\{\"bus\":\"peci\",\"vendor\":\"TestVendor\",\"version\":\"0.0.1\",\
\"ext_info\":\"{\\\"vids\\\":[1111, 2222],\\\"pids\\\":[1234,4567]}\"}";
    ret = driverInfo.UnSerialize(drvStr);
    ASSERT_NE(ret, 0);

    // invalid json, member error
    drvStr = "\{\"bus\":\"usb\",\"vendorxx\":\"TestVendor\",\"version\":\"0.0.1\",\
\"ext_info\":\"{\\\"vids\\\":[1111, 2222],\\\"pids\\\":[1234,4567]}\"}";
    ret = driverInfo.UnSerialize(drvStr);
    ASSERT_NE(ret, 0);

    // invalid json, member type error
    drvStr = "\{\"bus\":\"usb\",\"vendor\":\"TestVendor\",\"version\":0,\
\"ext_info\":\"{\\\"vids\\\":[1111, 2222],\\\"pids\\\":[1234,4567]}\"}";
    ret = driverInfo.UnSerialize(drvStr);
    ASSERT_NE(ret, 0);

    // invalid json, ext_info format error
    drvStr = "\{\"bus\":\"usb\",\"vendor\":\"TestVendor\",\"version\":\"0.0.1\",\
\"ext_info\":\"{\\\"vids\\\"_[1111, 2222],\\\"pids\\\":[1234,4567]}\"}";
    ret = driverInfo.UnSerialize(drvStr);
    ASSERT_NE(ret, 0);

    // invalid json, ext_info member
    drvStr = "\{\"bus\":\"usb\",\"vendor\":\"TestVendor\",\"version\":\"0.0.1\",\
\"ext_info\":\"{\\\"vid\\\":[1111, 2222],\\\"pids\\\":[1234,4567]}\"}";
    ret = driverInfo.UnSerialize(drvStr);
    ASSERT_NE(ret, 0);

    // invalid json, ext_info member type error
    drvStr = "\{\"bus\":\"usb\",\"vendor\":\"TestVendor\",\"version\":\"0.0.1\",\
\"ext_info\":\"{\\\"vids\\\":\\\"1111\\\",\\\"pids\\\":[1234,4567]}\"}";
    ret = driverInfo.UnSerialize(drvStr);
    ASSERT_NE(ret, 0);

    // invalid json, ext_info vids item type error
    drvStr = "\{\"bus\":\"usb\",\"vendor\":\"TestVendor\",\"version\":\"0.0.1\",\
\"ext_info\":\"{\\\"vids\\\":[\\\"1111\\\", \\\"2222\\\"],\\\"pids\\\":[1234,4567]}\"}";
    ret = driverInfo.UnSerialize(drvStr);
    ASSERT_NE(ret, 0);

    // invalid json, ext_info pid item type error
    drvStr = "\{\"bus\":\"usb\",\"vendor\":\"TestVendor\",\"version\":\"0.0.1\",\
\"ext_info\":\"{\\\"vids\\\":[1111, 2222],\\\"pids\\\":[\\\"1234\\\",\\\"4567\\\"]}\"}";
    ret = driverInfo.UnSerialize(drvStr);
    ASSERT_NE(ret, 0);
}
}
}