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

#include <dlfcn.h>

#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "iostream"
#define private public
#include "driver_pkg_manager.h"
#include "ibus_extension.h"
#include "usb_device_info.h"
#include "dev_change_callback.h"
#include "bus_extension_core.h"
#undef private

constexpr const char *START_TEXT = "Begin to loop and listen pkg event:\n\
enter q to exit.\n\
enter p to print QueryMatchDriver.";
using namespace OHOS::ExternalDeviceManager;
using namespace std;
static void PrintQueryMatchDriver()
{
    cout << "------------------" << endl;
    DriverPkgManager &drvPkgMgrInstance = DriverPkgManager::GetInstance();

    auto deviceInfo = make_shared<UsbDeviceInfo>(0);
    deviceInfo->devInfo_.devBusInfo.busType = BusType::BUS_TYPE_USB;
    deviceInfo->idProduct_ = 0x8835;
    deviceInfo->idVendor_ = 0x0B57;
    deviceInfo->deviceClass_ = 0;
    deviceInfo->bcdUSB_ = 0x1122;
    std::shared_ptr<BundleInfoNames> bundle = drvPkgMgrInstance.QueryMatchDriver(deviceInfo);

    if (bundle != nullptr) {
        cout << "Query Success" << endl;
    }
    cout << "------------------" << endl;
}

int main(int argc, char **argv)
{
    cout << START_TEXT << endl;
    std::shared_ptr<DevChangeCallback> callback = std::make_shared<DevChangeCallback>();
    BusExtensionCore::GetInstance().LoadBusExtensionLibs();
    bool ret = BusExtensionCore::GetInstance().Init(callback);
    if (ret != EDM_OK) {
        cout << "BusExtensionCore init failed" << endl;
        return ret;
    }
    DriverPkgManager &drvPkgMgrInstance = DriverPkgManager::GetInstance();
    ret = drvPkgMgrInstance.Init();
    if (ret != EDM_OK) {
        cout << "drvPkgMgrInstance init failed" << endl;
        return ret;
    }
    while (true) {
        std::string in;
        cin >> in;
        if (in == "q") {
            break;
        } else if (in == "p") {
            PrintQueryMatchDriver();
        } else {
            cout << "invalid param" << endl;
        }
    }
    cout << "exit!" << endl;
    return ret;
}