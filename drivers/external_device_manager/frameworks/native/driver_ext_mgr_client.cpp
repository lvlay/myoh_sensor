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

#include "driver_ext_mgr_client.h"
#include <if_system_ability_manager.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "hilog_wrapper.h"
namespace OHOS {
namespace ExternalDeviceManager {
DriverExtMgrClient::DriverExtMgrClient() {}

DriverExtMgrClient::~DriverExtMgrClient()
{
    if (proxy_ != nullptr) {
        auto remote = proxy_->AsObject();
        if (remote != nullptr) {
            remote->RemoveDeathRecipient(deathRecipient_);
        }
    }
}

UsbErrCode DriverExtMgrClient::Connect()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ != nullptr) {
        return UsbErrCode::EDM_OK;
    }

    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        EDM_LOGF(MODULE_FRAMEWORK, "Failed to obtain SystemAbilityMgr");
        return UsbErrCode::EDM_ERR_GET_SYSTEM_ABILITY_MANAGER_FAILED;
    }

    sptr<IRemoteObject> remote = sam->CheckSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
    if (remote == nullptr) {
        EDM_LOGF(MODULE_FRAMEWORK, "Check SystemAbility failed");
        return UsbErrCode::EDM_ERR_GET_SERVICE_FAILED;
    }

    deathRecipient_ = new DriverExtMgrDeathRecipient();
    if (deathRecipient_ == nullptr) {
        EDM_LOGF(MODULE_FRAMEWORK, "Failed to create DriverExtMgrDeathRecipient");
        return UsbErrCode::EDM_ERR_INVALID_OBJECT;
    }

    if ((remote->IsProxyObject()) && (!remote->AddDeathRecipient(deathRecipient_))) {
        EDM_LOGF(MODULE_FRAMEWORK, "Failed to add death recipient to DriverExtMgrProxy service");
        return EDM_ERR_NOT_SUPPORT;
    }

    proxy_ = iface_cast<IDriverExtMgr>(remote);
    if (proxy_ == nullptr) {
        EDM_LOGF(MODULE_FRAMEWORK, "Failed to cast DriverExtMgrProxy object");
    }
    EDM_LOGI(MODULE_FRAMEWORK, "Connecting DriverExtMgrProxy success");
    return UsbErrCode::EDM_OK;
}

void DriverExtMgrClient::DisConnect(const wptr<IRemoteObject> &remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        return;
    }

    auto serviceRemote = proxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
    }
}

void DriverExtMgrClient::DriverExtMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (remote == nullptr) {
        EDM_LOGF(MODULE_FRAMEWORK, "OnRemoteDied failed, remote is nullptr");
        return;
    }

    DriverExtMgrClient::GetInstance().DisConnect(remote);
    EDM_LOGF(MODULE_FRAMEWORK, "received death notification of remote and finished to disconnect");
}

UsbErrCode DriverExtMgrClient::QueryDevice(uint32_t busType, std::vector<std::shared_ptr<DeviceData>> &devices)
{
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }
    return proxy_->QueryDevice(busType, devices);
}

UsbErrCode DriverExtMgrClient::BindDevice(uint64_t deviceId, const sptr<IDriverExtMgrCallback> &connectCallback)
{
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }
    return proxy_->BindDevice(deviceId, connectCallback);
}

UsbErrCode DriverExtMgrClient::UnBindDevice(uint64_t deviceId)
{
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }
    return proxy_->UnBindDevice(deviceId);
}

UsbErrCode DriverExtMgrClient::QueryDeviceInfo(std::vector<std::shared_ptr<DeviceInfoData>> &deviceInfos)
{
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }
    return proxy_->QueryDeviceInfo(deviceInfos);
}

UsbErrCode DriverExtMgrClient::QueryDeviceInfo(const uint64_t deviceId,
    std::vector<std::shared_ptr<DeviceInfoData>> &deviceInfos)
{
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }
    return proxy_->QueryDeviceInfo(deviceInfos, true, deviceId);
}

UsbErrCode DriverExtMgrClient::QueryDriverInfo(std::vector<std::shared_ptr<DriverInfoData>> &driverInfos)
{
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }
    return proxy_->QueryDriverInfo(driverInfos);
}

UsbErrCode DriverExtMgrClient::QueryDriverInfo(const std::string &driverUid,
    std::vector<std::shared_ptr<DriverInfoData>> &driverInfos)
{
    if (Connect() != UsbErrCode::EDM_OK) {
        return UsbErrCode::EDM_ERR_CONNECTION_FAILED;
    }
    return proxy_->QueryDriverInfo(driverInfos, true, driverUid);
}

} // namespace ExternalDeviceManager
} // namespace OHOS