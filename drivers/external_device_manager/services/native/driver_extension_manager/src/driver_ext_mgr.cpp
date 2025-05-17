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

#include "driver_ext_mgr.h"

#include "bus_extension_core.h"
#include "dev_change_callback.h"
#include "driver_extension_controller.h"
#include "driver_pkg_manager.h"
#include "edm_errors.h"
#include "etx_device_mgr.h"
#include "ext_permission_manager.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "usb_device_info.h"
#include "usb_driver_info.h"

namespace OHOS {
namespace ExternalDeviceManager {
const bool G_REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<DriverExtMgr>::GetInstance().get());
static const std::string PERMISSION_NAME = "ohos.permission.ACCESS_EXTENSIONAL_DEVICE_DRIVER";

DriverExtMgr::DriverExtMgr() : SystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID, true) {}
DriverExtMgr::~DriverExtMgr() {}

void DriverExtMgr::OnStart()
{
    int32_t ret;
    EDM_LOGI(MODULE_SERVICE, "hdf_ext_devmgr OnStart");
    BusExtensionCore::GetInstance().LoadBusExtensionLibs();
    ret = DriverPkgManager::GetInstance().Init(bmsFuture_, accountFuture_, commEventFuture_);
    AddSystemAbilityListener(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN);
    AddSystemAbilityListener(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_SERVICE, "DriverPkgManager Init failed %{public}d", ret);
    }
    ret = ExtDeviceManager::GetInstance().Init();
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_SERVICE, "ExtDeviceManager Init failed %{public}d", ret);
    }
    std::shared_ptr<DevChangeCallback> callback = std::make_shared<DevChangeCallback>();
    ret = BusExtensionCore::GetInstance().Init(callback);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_SERVICE, "BusExtensionCore Init failed %{public}d", ret);
    }
    if (!Publish(AsObject())) {
        EDM_LOGE(MODULE_DEV_MGR, "OnStart register to system ability manager failed.");
        return;
    }
}

void DriverExtMgr::OnStop()
{
    EDM_LOGI(MODULE_SERVICE, "hdf_ext_devmgr OnStop");
    delete &(DriverPkgManager::GetInstance());
    delete &(ExtDeviceManager::GetInstance());
    delete &(BusExtensionCore::GetInstance());
    delete &(DriverExtensionController::GetInstance());
}

int DriverExtMgr::Dump(int fd, const std::vector<std::u16string> &args)
{
    return 0;
}

void DriverExtMgr::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    EDM_LOGI(MODULE_SERVICE, "OnAddSystemAbility systemAbilityId: %{public}d", systemAbilityId);
    std::lock_guard<std::mutex> lock(promiseMutex_);
    switch (systemAbilityId) {
        case SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN: {
            EDM_LOGI(MODULE_SERVICE, "OnAddSystemAbility accountMgr");
            DriverPkgManager::GetInstance().SubscribeOsAccountSwitch();
            if (!accountPromiseUsed_) {
                accountPromise_.set_value(systemAbilityId);
                accountPromiseUsed_ = true;
            }
            break;
        }
        case  BUNDLE_MGR_SERVICE_SYS_ABILITY_ID: {
            EDM_LOGI(MODULE_SERVICE, "OnAddSystemAbility BMS");
            if (!bmsPromiseUsed_) {
                bmsPromise_.set_value(systemAbilityId);
                bmsPromiseUsed_ = true;
            }
            break;
        }
        case  COMMON_EVENT_SERVICE_ID: {
            EDM_LOGI(MODULE_SERVICE, "OnAddSystemAbility CommonEventService");
            DriverPkgManager::GetInstance().RegisterBundleStatusCallback();
            if (!cesPromiseUsed_) {
                commEventPromise_.set_value(systemAbilityId);
                cesPromiseUsed_ = true;
            }
            break;
        }
        default:
            break;
    }
}

UsbErrCode DriverExtMgr::QueryDevice(uint32_t busType, std::vector<std::shared_ptr<DeviceData>> &devices)
{
    if (!ExtPermissionManager::VerifyPermission(PERMISSION_NAME)) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s no permission", __func__);
        return UsbErrCode::EDM_ERR_NO_PERM;
    }

    if (busType == BusType::BUS_TYPE_INVALID) {
        EDM_LOGE(MODULE_DEV_MGR, "invalid busType:%{public}d", static_cast<int32_t>(busType));
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    std::vector<std::shared_ptr<DeviceInfo>> deviceInfos =
        ExtDeviceManager::GetInstance().QueryDevice(static_cast<BusType>(busType));
    for (const auto &deviceInfo : deviceInfos) {
        switch (deviceInfo->GetBusType()) {
            case BusType::BUS_TYPE_USB: {
                std::shared_ptr<UsbDeviceInfo> usbDeviceInfo = std::static_pointer_cast<UsbDeviceInfo>(deviceInfo);
                std::shared_ptr<USBDevice> device = std::make_shared<USBDevice>();
                device->busType = usbDeviceInfo->GetBusType();
                device->deviceId = usbDeviceInfo->GetDeviceId();
                device->descripton = usbDeviceInfo->GetDeviceDescription();
                device->productId = usbDeviceInfo->GetProductId();
                device->vendorId = usbDeviceInfo->GetVendorId();
                devices.push_back(device);
                break;
            }
            default: {
                break;
            }
        }
    }

    return UsbErrCode::EDM_OK;
}

UsbErrCode DriverExtMgr::BindDevice(uint64_t deviceId, const sptr<IDriverExtMgrCallback> &connectCallback)
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    if (!ExtPermissionManager::VerifyPermission(PERMISSION_NAME)) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s no permission", __func__);
        return UsbErrCode::EDM_ERR_NO_PERM;
    }

    return static_cast<UsbErrCode>(ExtDeviceManager::GetInstance().ConnectDevice(deviceId, connectCallback));
}

UsbErrCode DriverExtMgr::UnBindDevice(uint64_t deviceId)
{
    EDM_LOGD(MODULE_DEV_MGR, "%{public}s enter", __func__);
    if (!ExtPermissionManager::VerifyPermission(PERMISSION_NAME)) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s no permission", __func__);
        return UsbErrCode::EDM_ERR_NO_PERM;
    }

    return static_cast<UsbErrCode>(ExtDeviceManager::GetInstance().DisConnectDevice(deviceId));
}

static std::shared_ptr<DeviceInfoData> ParseToDeviceInfoData(const std::shared_ptr<Device> &device)
{
    EDM_LOGD(MODULE_DEV_MGR, "%{public}s enter", __func__);
    if (device == nullptr || device->GetDeviceInfo() == nullptr) {
        EDM_LOGD(MODULE_DEV_MGR, "device or deviceInfo is null");
        return nullptr;
    }
    auto deviceInfo = device->GetDeviceInfo();
    auto busType = deviceInfo->GetBusType();
    if (busType <= BusType::BUS_TYPE_INVALID || busType >= BusType::BUS_TYPE_MAX) {
        EDM_LOGD(MODULE_DEV_MGR, "invalid busType:%{public}u", busType);
        return nullptr;
    }
    std::shared_ptr<DeviceInfoData> tempDeviceInfo = nullptr;
    
    switch (busType) {
        case BusType::BUS_TYPE_USB: {
            std::shared_ptr<UsbDeviceInfo> usbDeviceInfo = std::static_pointer_cast<UsbDeviceInfo>(deviceInfo);
            std::shared_ptr<USBDeviceInfoData> tempUsbDeviceInfo = std::make_shared<USBDeviceInfoData>();
            tempUsbDeviceInfo->productId = usbDeviceInfo->GetProductId();
            tempUsbDeviceInfo->vendorId = usbDeviceInfo->GetVendorId();
            std::transform(usbDeviceInfo->interfaceDescList_.begin(), usbDeviceInfo->interfaceDescList_.end(),
                std::back_inserter(tempUsbDeviceInfo->interfaceDescList),
                [](UsbInterfaceDescriptor desc) {
                    std::shared_ptr<USBInterfaceDesc> interfaceDesc = std::make_shared<USBInterfaceDesc>();
                    interfaceDesc->bInterfaceNumber = desc.bInterfaceNumber;
                    interfaceDesc->bClass = desc.bInterfaceClass;
                    interfaceDesc->bSubClass = desc.bInterfaceSubClass;
                    interfaceDesc->bProtocol = desc.bInterfaceProtocol;
                    return interfaceDesc;
                });
            tempDeviceInfo = tempUsbDeviceInfo;
            break;
        }
        default:
            break;
    }
    if (tempDeviceInfo != nullptr) {
        tempDeviceInfo->deviceId = deviceInfo->GetDeviceId();
        tempDeviceInfo->isDriverMatched = device->HasDriver();
        if (tempDeviceInfo->isDriverMatched) {
            tempDeviceInfo->driverUid = device->GetDriverUid();
        }
    }
    
    return tempDeviceInfo;
}

static std::shared_ptr<DriverInfoData> ParseToDriverInfoData(const std::shared_ptr<DriverInfo> &driverInfo)
{
    EDM_LOGD(MODULE_DEV_MGR, "%{public}s enter", __func__);
    if (driverInfo == nullptr || driverInfo->GetInfoExt() == nullptr) {
        EDM_LOGD(MODULE_DEV_MGR, "driverInfo or extInfo is null");
        return nullptr;
    }
    auto busType = driverInfo->GetBusType();
    if (busType <= BusType::BUS_TYPE_INVALID || busType >= BusType::BUS_TYPE_MAX) {
        EDM_LOGD(MODULE_DEV_MGR, "invalid busType:%{public}u", busType);
        return nullptr;
    }
    std::shared_ptr<DriverInfoData> tempDriverInfo = nullptr;

    switch (busType) {
        case BusType::BUS_TYPE_USB: {
            std::shared_ptr<UsbDriverInfo> usbDriverInfo
                = std::static_pointer_cast<UsbDriverInfo>(driverInfo->GetInfoExt());
            std::shared_ptr<USBDriverInfoData> tempUsbDriverInfo = std::make_shared<USBDriverInfoData>();
            tempUsbDriverInfo->pids = usbDriverInfo->GetProductIds();
            tempUsbDriverInfo->vids = usbDriverInfo->GetVendorIds();
            tempDriverInfo = tempUsbDriverInfo;
            break;
        }
        default:
            break;
    }
    if (tempDriverInfo != nullptr) {
        tempDriverInfo->busType = busType;
        tempDriverInfo->driverUid = driverInfo->GetDriverUid();
        tempDriverInfo->driverName = driverInfo->GetDriverName();
        tempDriverInfo->bundleSize = driverInfo->GetDriverSize();
        tempDriverInfo->version = driverInfo->GetVersion();
        tempDriverInfo->description = driverInfo->GetDescription();
    }
    return tempDriverInfo;
}

UsbErrCode DriverExtMgr::QueryDeviceInfo(std::vector<std::shared_ptr<DeviceInfoData>> &deviceInfos,
    bool isByDeviceId, const uint64_t deviceId)
{
    EDM_LOGD(MODULE_DEV_MGR, "%{public}s enter", __func__);
    if (!ExtPermissionManager::IsSystemApp()) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s none system app", __func__);
        return UsbErrCode::EDM_ERR_NOT_SYSTEM_APP;
    }

    if (!ExtPermissionManager::VerifyPermission(PERMISSION_NAME)) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s no permission", __func__);
        return UsbErrCode::EDM_ERR_NO_PERM;
    }

    vector<shared_ptr<Device>> devices;
    if (isByDeviceId) {
        devices = ExtDeviceManager::GetInstance().QueryDevicesById(deviceId);
    } else {
        devices = ExtDeviceManager::GetInstance().QueryAllDevices();
    }
    
    for (const auto &device : devices) {
        auto tempDeviceInfo = ParseToDeviceInfoData(device);
        if (tempDeviceInfo != nullptr) {
            deviceInfos.push_back(tempDeviceInfo);
        }
    }
    return UsbErrCode::EDM_OK;
}

UsbErrCode DriverExtMgr::QueryDriverInfo(std::vector<std::shared_ptr<DriverInfoData>> &driverInfos,
    bool isByDriverUid, const std::string &driverUid)
{
    if (!ExtPermissionManager::IsSystemApp()) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s none system app", __func__);
        return UsbErrCode::EDM_ERR_NOT_SYSTEM_APP;
    }

    if (!ExtPermissionManager::VerifyPermission(PERMISSION_NAME)) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s no permission", __func__);
        return UsbErrCode::EDM_ERR_NO_PERM;
    }

    vector<shared_ptr<DriverInfo>> tempDriverInfos;
    int32_t ret = DriverPkgManager::GetInstance().QueryDriverInfo(tempDriverInfos, isByDriverUid, driverUid);
    if (ret != UsbErrCode::EDM_OK) {
        return static_cast<UsbErrCode>(ret);
    }
    for (const auto &driverInfo : tempDriverInfos) {
        auto tempDriverInfo = ParseToDriverInfoData(driverInfo);
        if (tempDriverInfo != nullptr) {
            driverInfos.push_back(tempDriverInfo);
        }
    }
    EDM_LOGD(MODULE_DEV_MGR, "driverInfos size: %{public}zu enter", driverInfos.size());

    return UsbErrCode::EDM_OK;
}
} // namespace ExternalDeviceManager
} // namespace OHOS
