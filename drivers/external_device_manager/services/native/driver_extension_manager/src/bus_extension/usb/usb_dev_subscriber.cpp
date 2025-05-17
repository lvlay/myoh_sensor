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

#include "string"
#include "securec.h"
#include "hilog_wrapper.h"
#include "edm_errors.h"
#include "usb_dev_subscriber.h"
namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
constexpr uint32_t MAX_DEV_ID_SIZE = 100;
constexpr uint32_t ACT_DEVUP       = 0;
constexpr uint32_t ACT_DEVDOWN     = 1;
constexpr uint32_t SHIFT_16        = 16;
constexpr uint32_t SHIFT_32        = 32;
constexpr uint32_t USB_DEV_DESC_SIZE = 0x12;
struct UsbDevDescLite {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
} __attribute__((packed));

static string ToDeviceDesc(const UsbDev& usbDev, const UsbDevDescLite& desc)
{
    char buffer[MAX_DEV_ID_SIZE];
    auto ret = sprintf_s(buffer, sizeof(buffer), "USB&BUS_%02X&DEV_%02X&PID_%04X&VID_%04X&CLASS_%02X",\
        usbDev.busNum, usbDev.devAddr, desc.idProduct, desc.idVendor, desc.bDeviceClass);
    if (ret < 0) {
        EDM_LOGE(MODULE_BUS_USB,  "ToDeivceDesc sprintf_s error. ret = %{public}d", ret);
        return string();
    }
    return string(buffer);
}

static uint32_t ToBusDeivceId(const UsbDev& usbDev)
{
    uint32_t devId = (usbDev.busNum << SHIFT_16) + usbDev.devAddr;
    return devId;
}

static uint64_t ToDdkDeviceId(const UsbDev& usbDev)
{
    return ((uint64_t)usbDev.busNum << SHIFT_32) + usbDev.devAddr;
}


void UsbDevSubscriber::Init(shared_ptr<IDevChangeCallback> callback, sptr<IUsbInterface> iusb, sptr<IUsbDdk> iUsbDdk)
{
    this->iusb_ = iusb;
    this->callback_ = callback;
    this->iUsbDdk_ = iUsbDdk;
};

int32_t UsbDevSubscriber::GetInterfaceDescriptor(const UsbDev &usbDev,
    std::vector<UsbInterfaceDescriptor> &interfaceList)
{
    int32_t ret = EDM_NOK;
    if (this->iusb_ == nullptr || this->iUsbDdk_ == nullptr) {
        return EDM_ERR_INVALID_OBJECT;
    }
    uint8_t configIndex;
    ret = this->iusb_->GetConfig(usbDev, configIndex);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_BUS_USB,  "GetConfig fail, ret = %{public}d", ret);
        return ret;
    }

    uint64_t ddkDeviceId = ToDdkDeviceId(usbDev);
    std::vector<uint8_t> descriptor;
    ret = this->iUsbDdk_->GetConfigDescriptor(ddkDeviceId, configIndex, descriptor);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_BUS_USB, "GetConfigDescriptor fail, ret = %{public}d", ret);
        return ret;
    }
    UsbDdkConfigDescriptor *config = nullptr;
    ret = ParseUsbConfigDescriptor(descriptor, &config);
    if (ret != EDM_OK || config == nullptr) {
        FreeUsbConfigDescriptor(config);
        EDM_LOGE(MODULE_BUS_USB, "ParseUsbConfigDescriptor fail, ret = %{public}d", ret);
        return ret;
    }
    if (config->interface == nullptr) {
        FreeUsbConfigDescriptor(config);
        EDM_LOGE(MODULE_BUS_USB,  "UsbDdkInterface is null");
        return EDM_ERR_INVALID_OBJECT;
    }
    for (uint8_t i = 0; i < config->configDescriptor.bNumInterfaces; i++) {
        UsbDdkInterfaceDescriptor *interfaceDesc = config->interface[i].altsetting;
        if (interfaceDesc == nullptr) {
            FreeUsbConfigDescriptor(config);
            EDM_LOGE(MODULE_BUS_USB,  "UsbDdkInterfaceDescriptor is null");
            return EDM_ERR_INVALID_OBJECT;
        }
        interfaceList.push_back(interfaceDesc->interfaceDescriptor);
    }
    FreeUsbConfigDescriptor(config);
    return EDM_OK;
}

int32_t UsbDevSubscriber::OnDeviceConnect(const UsbDev &usbDev)
{
    EDM_LOGD(MODULE_BUS_USB,  "OnDeviceConnect enter");
    int32_t ret = 0;
    if (this->iusb_ == nullptr) {
        return EDM_ERR_INVALID_OBJECT;
    }
    vector<uint8_t> descData;
    ret = this->iusb_->GetDeviceDescriptor(usbDev, descData);
    if (ret != 0) {
        EDM_LOGE(MODULE_BUS_USB,  "GetDeviceDescriptor fail, ret = %{public}d\n", ret);
        return EDM_ERR_IO;
    }
    uint8_t *buffer = descData.data();
    uint32_t length = descData.size();
    if (length == 0) {
        EDM_LOGE(MODULE_BUS_USB,  "GetRawDescriptor failed len=%{public}d busNum:%{public}d devAddr:%{public}d",\
            length, usbDev.busNum, usbDev.devAddr);
        return EDM_ERR_USB_ERR;
    }
    UsbDevDescLite deviceDescriptor = *(reinterpret_cast<const UsbDevDescLite *>(buffer));
    if (deviceDescriptor.bLength != USB_DEV_DESC_SIZE) {
        EDM_LOGE(MODULE_BUS_USB,  "UsbdDeviceDescriptor size error");
        return EDM_ERR_USB_ERR;
    }
    string desc = ToDeviceDesc(usbDev, deviceDescriptor);
    uint32_t busDevId = ToBusDeivceId(usbDev);
    auto usbDevInfo = make_shared<UsbDeviceInfo>(busDevId, desc);

    usbDevInfo->bcdUSB_ = deviceDescriptor.bcdUSB;
    usbDevInfo->idProduct_ = deviceDescriptor.idProduct;
    usbDevInfo->idVendor_ = deviceDescriptor.idVendor;
    usbDevInfo->deviceClass_ = deviceDescriptor.bDeviceClass;

    ret = GetInterfaceDescriptor(usbDev, usbDevInfo->interfaceDescList_);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_BUS_USB,  "GetInterfaceDescriptor fail, ret = %{public}d", ret);
        return ret;
    }

    this->deviceInfos_[busDevId] = usbDevInfo;
    if (this->callback_ != nullptr) {
        this->callback_->OnDeviceAdd(usbDevInfo);
    }
    EDM_LOGD(MODULE_BUS_USB,  "OnDeviceConnect:");
    EDM_LOGD(MODULE_BUS_USB,  "%{public}s", desc.c_str());
    return EDM_OK;
};

int32_t UsbDevSubscriber::OnDeviceDisconnect(const UsbDev &usbDev)
{
    EDM_LOGD(MODULE_BUS_USB,  "OnDeviceDisconnect enter");
    uint32_t busDevId = ToBusDeivceId(usbDev);
    if (this->callback_ != nullptr) {
        auto deviceInfo = this->deviceInfos_[busDevId];
        if (deviceInfo != nullptr) {
            this->callback_->OnDeviceRemove(deviceInfo);
        } else {
            EDM_LOGW(MODULE_BUS_USB,  "no dev in map, busDevId=%{public}08X \n", busDevId);
        }
    }
    this->deviceInfos_.erase(busDevId);
    return 0;
}

int32_t UsbDevSubscriber::DeviceEvent(const USBDeviceInfo &usbDevInfo)
{
    EDM_LOGD(MODULE_BUS_USB,  "DeviceEvent enter");
    UsbDev usbDev = {usbDevInfo.busNum, usbDevInfo.devNum};
    int32_t ret = 0;
    if (usbDevInfo.status == ACT_DEVUP) {
        ret = this->OnDeviceConnect(usbDev);
    } else if (usbDevInfo.status == ACT_DEVDOWN) {
        ret = this->OnDeviceDisconnect(usbDev);
    } else {
        EDM_LOGW(MODULE_BUS_USB,  "status not support, %{public}d \n", usbDevInfo.status);
    }
    EDM_LOGD(MODULE_BUS_USB,  "ret = %{public}d, %{public}s", ret, this->ToString().c_str());
    return ret;
}

int32_t UsbDevSubscriber::PortChangedEvent(const PortInfo &usbDevInfo)
{
    return 0;
}

string UsbDevSubscriber::ToString(void)
{
    string str = "DeviceInfos: Device count:" + to_string(this->deviceInfos_.size()) + "\n";
    int i = 0;
    for (auto it = this->deviceInfos_.begin(); it != deviceInfos_.end(); it++) {
        str += "[" +to_string(i++) + "]" + to_string(it->first) + "\n";
    }
    return str;
}
}
}