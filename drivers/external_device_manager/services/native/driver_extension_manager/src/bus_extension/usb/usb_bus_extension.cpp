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
#include "string_ex.h"
#include "sstream"
#include "iostream"

#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "ibus_extension.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bus_extension_core.h"
#include "usb_dev_subscriber.h"
#include "usb_device_info.h"
#include "usb_driver_info.h"
#include "usb_bus_extension.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;

UsbBusExtension::UsbBusExtension()
{
    this->subScriber_ = nullptr;
    this->usbInterface_ = nullptr;
}

UsbBusExtension::~UsbBusExtension()
{
    if (this->usbInterface_ != nullptr && this->subScriber_ != nullptr && this->recipient_ != nullptr) {
        this->usbInterface_->UnbindUsbdSubscriber(this->subScriber_);
        sptr<IRemoteObject> remote = OHOS::HDI::hdi_objcast<HDI::Usb::V1_0::IUsbInterface>(usbInterface_);
        remote->RemoveDeathRecipient(recipient_);
        recipient_.clear();
    }
}

void UsbBusExtension::SetUsbInferface(sptr<IUsbInterface> iusb)
{
    this->usbInterface_ = iusb;
}

void UsbBusExtension::SetUsbDdk(sptr<IUsbDdk> iUsbDdk)
{
    this->iUsbDdk_ = iUsbDdk;
}

BusType UsbBusExtension::GetBusType()
{
    return BusType::BUS_TYPE_USB;
}

int32_t UsbBusExtension::SetDevChangeCallback(shared_ptr<IDevChangeCallback> devCallback)
{
    if (this->usbInterface_ == nullptr) {
        this->usbInterface_ = IUsbInterface::Get();
        if (this->usbInterface_ == nullptr) {
            EDM_LOGE(MODULE_BUS_USB,  "get IUsbInterface error");
            return EDM_ERR_INVALID_OBJECT;
        }
        EDM_LOGD(MODULE_BUS_USB,  "get usbInferface sucess");
        recipient_ = new UsbdDeathRecipient();
        sptr<IRemoteObject> remote = OHOS::HDI::hdi_objcast<HDI::Usb::V1_0::IUsbInterface>(usbInterface_);
        if (!remote->AddDeathRecipient(recipient_)) {
            EDM_LOGE(MODULE_BUS_USB, "add DeathRecipient failed");
            return EDM_NOK;
        }
    }

    if (this->iUsbDdk_ == nullptr) {
        this->iUsbDdk_ = IUsbDdk::Get();
        if (this->iUsbDdk_ == nullptr) {
            EDM_LOGE(MODULE_BUS_USB,  "get IUsbDdk error");
            return EDM_ERR_INVALID_OBJECT;
        }
    }

    if (this->subScriber_ == nullptr) {
        this->subScriber_ = new UsbDevSubscriber();
        if (this->subScriber_ == nullptr) {
            EDM_LOGE(MODULE_BUS_USB,  "get usbDevSubscriber error");
            return EDM_EER_MALLOC_FAIL;
        }
        EDM_LOGD(MODULE_BUS_USB,  "get subScriber_ sucess");
    }

    this->subScriber_->Init(devCallback, usbInterface_, iUsbDdk_);
    this->usbInterface_->BindUsbdSubscriber(subScriber_);

    return 0;
};

bool UsbBusExtension::MatchDriver(const DriverInfo &driver, const DeviceInfo &device)
{
    if (LowerStr(driver.GetBusName()) != "usb") {
        EDM_LOGW(MODULE_BUS_USB,  "driver bus not support by this module [UsbBusExtension]");
        return false;
    }

    if (device.GetBusType() != BusType::BUS_TYPE_USB) {
        EDM_LOGW(MODULE_BUS_USB,  "deivce type not support %d != %d",
            (uint32_t)device.GetBusType(), (uint32_t)BusType::BUS_TYPE_USB);
        return false;
    }
    const UsbDriverInfo *usbDriverInfo = static_cast<const UsbDriverInfo *>(driver.GetInfoExt().get());
    const UsbDeviceInfo *usbDeviceInfo = static_cast<const UsbDeviceInfo *>(&device);
    if (usbDriverInfo == nullptr || usbDeviceInfo == nullptr) {
        EDM_LOGE(MODULE_BUS_USB,  "static_cast error, the usbDriverInfo or usbDeviceInfo is nullptr");
        return false;
    }
    string usbDrvInfoStr;
    const_cast<UsbDriverInfo*>(usbDriverInfo)->Serialize(usbDrvInfoStr);
    EDM_LOGD(MODULE_BUS_USB, "UsbDriverInfo:%{public}s", usbDrvInfoStr.c_str());
    EDM_LOGD(MODULE_BUS_USB, "UsbDeviceInfo: vid = %{public}d, pid = %{public}d",
        usbDeviceInfo->idVendor_, usbDeviceInfo->idProduct_);
    auto vidFind = find(usbDriverInfo->vids_.begin(), usbDriverInfo->vids_.end(), usbDeviceInfo->idVendor_);
    if (vidFind == usbDriverInfo->vids_.end()) {
        EDM_LOGI(MODULE_BUS_USB,  "vid not match\n");
        return false;
    }
    auto pidFind = find(usbDriverInfo->pids_.begin(), usbDriverInfo->pids_.end(), usbDeviceInfo->idProduct_);
    if (pidFind == usbDriverInfo->pids_.end()) {
        EDM_LOGI(MODULE_BUS_USB,  "pid not match\n");
        return false;
    }
    EDM_LOGI(MODULE_BUS_USB,  "Driver and Device match sucess\n");
    return true;
}

shared_ptr<DriverInfoExt> UsbBusExtension::ParseDriverInfo(const map<string, string> &metadata)
{
    shared_ptr<UsbDriverInfo> usbDriverInfo = make_shared<UsbDriverInfo>();
    if (usbDriverInfo == nullptr) {
        EDM_LOGE(MODULE_BUS_USB,  "creat UsbDriverInfo obj fail\n");
        return nullptr;
    }
    for (auto& meta : metadata) {
        if (LowerStr(meta.first) == "pid") {
            usbDriverInfo->pids_ = this->ParseCommaStrToVectorUint16(meta.second);
        } else if (LowerStr(meta.first) == "vid") {
            usbDriverInfo->vids_ = this->ParseCommaStrToVectorUint16(meta.second);
        }
    }
    return usbDriverInfo;
}

vector<uint16_t> UsbBusExtension::ParseCommaStrToVectorUint16(const string &str)
{
    vector<uint16_t> ret;
    stringstream ss(str);
    string s;
    int num;
    while (getline(ss, s, ',')) {
        stringstream out;
        out << hex << s;
        out >> num;
        ret.push_back(num);
    }
    if (ret.size() == 0) {
        EDM_LOGW(MODULE_BUS_USB,  "parse error, size 0, str:%{public}s.", str.c_str());
    } else {
        EDM_LOGD(MODULE_BUS_USB,  "parse sucess, size %{public}zu, str:%{public}s", ret.size(), str.c_str());
    }
    return ret;
}
void UsbBusExtension::UsbdDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "get samgr failed");
        return;
    }

    auto ret = samgrProxy->UnloadSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_BUS_USB, "unload failed");
    }
}

shared_ptr<DriverInfoExt> UsbBusExtension::GetNewDriverInfoExtObject()
{
    return make_shared<UsbDriverInfo>();
}

__attribute__ ((constructor)) static void RegBusExtension()
{
    EDM_LOGI(MODULE_COMMON, "installing UsbBusExtension");
    RegisterBusExtension<UsbBusExtension>(BusType::BUS_TYPE_USB);
}
}
}