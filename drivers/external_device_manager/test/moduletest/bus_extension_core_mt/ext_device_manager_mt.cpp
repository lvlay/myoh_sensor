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
#include "bus_extension_core.h"
#include "dev_change_callback.h"
#include "driver_pkg_manager.h"
#include "etx_device_mgr.h"

#undef private

constexpr const char *START_TEXT = "Begin to loop and listen usb event:\n\
enter q to exit.\n\
enter p to print all usb device.";
using namespace OHOS::ExternalDeviceManager;
using namespace std;
static void PrintAllDevice()
{
    ExtDeviceManager &devmgr = ExtDeviceManager::GetInstance();
    cout << "------------------" << endl;
    std::vector<shared_ptr<DeviceInfo>> devicesInfo = devmgr.QueryDevice(BUS_TYPE_USB);
    cout << "usb device size: " << devicesInfo.size() << endl;
    for (auto &iter :devicesInfo) {
        cout << "description: " << iter->GetDeviceDescription().c_str() << endl;
        cout << "deviceId: " << std::hex << iter->GetDeviceId() << endl;
    }
    std::unordered_map<string, unordered_set<uint64_t>> &bundleMatchMap = devmgr.bundleMatchMap_;
    cout << "bundleMatchMap size:" << bundleMatchMap.size() << endl;
    for (auto &iter : bundleMatchMap) {
        cout << "bundleInfo [" << iter.first << "]: ";
        for (auto &devId : iter.second) {
            cout << std::hex << devId << " ";
        }
        cout << endl;
    }
    cout << "------------------" << endl;
}

int main(int argc, char **argv)
{
    cout << START_TEXT << endl;
    BusExtensionCore::GetInstance().LoadBusExtensionLibs();
    int32_t ret = DriverPkgManager::GetInstance().Init();
    if (ret != EDM_OK) {
        cout << "DriverPkgManager Init failed, ret = " << ret << endl;
        return -1;
    }
    ret = ExtDeviceManager::GetInstance().Init();
    if (ret != EDM_OK) {
        cout << "ExtDeviceManager Init failed, ret = " << ret << endl;
        return -1;
    }
    std::shared_ptr<DevChangeCallback> callback = std::make_shared<DevChangeCallback>();
    ret = BusExtensionCore::GetInstance().Init(callback);
    if (ret != EDM_OK) {
        cout << "BusExtensionCore Init failed, ret = " << ret << endl;
        return -1;
    }

    while (true) {
        std::string in;
        cin >> in;
        if (in == "q") {
            break;
        } else if (in == "p") {
            PrintAllDevice();
        } else {
            cout << in;
        }
    }
    cout << "exit!" << endl;
    return ret;
}