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

#ifndef USB_DEVICE_INFO_H
#define USB_DEVICE_INFO_H
#include "ibus_extension.h"
#include "usb_ddk_types.h"
namespace OHOS {
namespace ExternalDeviceManager {
class UsbDeviceInfo : public DeviceInfo {
public:
    UsbDeviceInfo(uint32_t busDeviceId, const std::string &description = "")
        : DeviceInfo(busDeviceId, BusType::BUS_TYPE_USB, description) { }
    ~UsbDeviceInfo() = default;

    uint16_t GetVendorId() const
    {
        return idVendor_;
    }

    uint16_t GetProductId() const
    {
        return idProduct_;
    }

    std::vector<UsbInterfaceDescriptor> interfaceDescList_;

private:
    friend class UsbBusExtension;
    friend class UsbDevSubscriber;
    uint16_t bcdUSB_ = 0;
    uint8_t  deviceClass_ = 0;
    uint16_t idVendor_ = 0;
    uint16_t idProduct_ = 0;
};
}
}
#endif