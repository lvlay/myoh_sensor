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
#include "cinttypes"
#include "usb_bus_extension.h"
#include "iostream"
#include "hilog_wrapper.h"
namespace OHOS {
namespace ExternalDeviceManager {
constexpr const char *HELP_TEXT = "usb_bus_extension_mt help:\n\
Before run this tool, please make sure the selinux is disabled.\n\
use commond below to disable selinux:\n\
setenfore 0\n\
use command below to check: (Permissive means selinux is disabled)\n\
getenfore\n\
------------------------------------------------------------------\n\
\n";
constexpr const char *START_TEXT = "Begin to loop and listen usb event:\n\
enter q to exit.\n\
enter p to print all usb device.";
constexpr const char *EXIT_TEXT = "Exit now!";
using namespace std;
class BusExtensionUsbMt {
public:
/*
运行前要先关闭selinux，
1、临时方法：运行 setenforce 0，重启还会开启
验证：运行 getenforce  应该打印Permissive
2、修改 /etc/selinux/config文件中的SELINUX值改为disable。（开启为enforcing）
*/
    int Entry(void)
    {
        cout << HELP_TEXT << endl;
        auto usbExt = std::make_shared<UsbBusExtension>();
        auto cb = make_shared<TestDevChangeCallback>();
        cout << START_TEXT << endl;
        usbExt->SetDevChangeCallback(cb);
        while (true) {
            string in;
            cin >> in;
            if (in == "q") {
                break;
            } else if (in == "p") {
                cb->PrintAllDevice();
            } else {
                cout << in;
            }
        }
        cout << EXIT_TEXT << endl;
        return 0;
    };
    class TestDevChangeCallback : public IDevChangeCallback {
    public:
        map<uint64_t, shared_ptr<DeviceInfo>> devInfoMap_;
        TestDevChangeCallback() { };
        int32_t OnDeviceAdd(shared_ptr<DeviceInfo> device) override
        {
            devInfoMap_[device->GetDeviceId()] = device;
            EDM_LOGI(MODULE_COMMON, "OnDeviceAdd: Id = %{public}016" PRIx64 ", Desc = %{public}s",
                device->GetDeviceId(), device->GetDeviceDescription().c_str());
            PrintAllDevice();
            return 0;
        };
        int32_t OnDeviceRemove(shared_ptr<DeviceInfo> device) override
        {
            devInfoMap_.erase(device->GetDeviceId());
            EDM_LOGI(MODULE_COMMON, "OnDeviceRemove: Id = %{public}016" PRIx64 ", Desc = %{public}s",
                device->GetDeviceId(), device->GetDeviceDescription().c_str());
            PrintAllDevice();
            return 0;
        };
        void PrintAllDevice(void)
        {
            cout << "++++++++, all usb device, count = " << devInfoMap_.size() << " detail:" << endl;
            for (auto &devItem : devInfoMap_) {
                cout << devItem.second->GetDeviceDescription().c_str() << endl;
            }
            cout << "--------" << endl;
        }
    };
};
}
}
int main(int argc, char **argv)
{
    int ret = OHOS::ExternalDeviceManager::BusExtensionUsbMt().Entry();
    return ret;
}