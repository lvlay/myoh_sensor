/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "iostream"
#include "string"
#include "securec.h"
#include "usb_bus_extension.h"
#include "usb_driver_info.h"
#include "hilog_wrapper.h"
#include "usbextension_fuzzer.h"
namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;

bool UsbDriverInfoUnSerializeFuzzer(const uint8_t *data, size_t size)
{
    string drvInfoStr(reinterpret_cast<const char *>(data));
    if (drvInfoStr.size() == 0) {
        return false;
    }

    UsbDriverInfo usbDevInfo;
    usbDevInfo.UnSerialize(drvInfoStr);
    return true;
}
bool ParseDriverInfoTest(const uint8_t *data, size_t size)
{
    auto bus = make_shared<UsbBusExtension>();
    string str(reinterpret_cast<const char *>(data));
    map<string, string> metadata = {
        {"vid", str},
        {"pid", str}
    };
    auto ret = bus->ParseDriverInfo(metadata);
    return true;
}

using TestFuncDef = bool (*)(const uint8_t *data, size_t size);

TestFuncDef g_allTestFunc[] = {
    UsbDriverInfoUnSerializeFuzzer,
    ParseDriverInfoTest,
};

bool DoSomethingInterestingWithMyAPI(const uint8_t *rawData, size_t size)
{
    if (size < sizeof(int)) {
        return false;
    }
    const int index = *(static_cast<const uint8_t *>(rawData));
    rawData += sizeof(int);
    size -= sizeof(int);
    int funcCount = sizeof(g_allTestFunc) / sizeof(g_allTestFunc[0]);

    auto func = g_allTestFunc[index % funcCount];
    if (func != nullptr) {
        auto ret = func(rawData, size);
        return ret;
    }
    return false;
}
} // namespace ExtDevMgr
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::ExternalDeviceManager::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}