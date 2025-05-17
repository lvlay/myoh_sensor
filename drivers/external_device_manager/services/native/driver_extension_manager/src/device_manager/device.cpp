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

#include <unistd.h>

#include "ability_manager_errors.h"
#include "hilog_wrapper.h"
#include "device.h"
#include "etx_device_mgr.h"

namespace OHOS {
namespace ExternalDeviceManager {
std::string Device::GetBundleName(const std::string &bundleInfo)
{
    std::string::size_type pos = bundleInfo.find(stiching_);
    if (pos == std::string::npos) {
        EDM_LOGI(MODULE_DEV_MGR, "bundleInfo not find stiching name");
        return "";
    }

    return bundleInfo.substr(0, pos);
}

std::string Device::GetAbilityName(const std::string &bundleInfo)
{
    std::string::size_type pos = bundleInfo.find(stiching_);
    if (pos == std::string::npos) {
        EDM_LOGI(MODULE_DEV_MGR, "bundleInfo not find stiching name");
        return "";
    }

    return bundleInfo.substr(pos + stiching_.length());
}

int32_t Device::Connect()
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    std::lock_guard<std::recursive_mutex> lock(deviceMutex_);
    uint32_t busDevId = GetDeviceInfo()->GetBusDevId();
    std::string bundleInfo = GetBundleInfo();
    std::string bundleName = Device::GetBundleName(bundleInfo);
    std::string abilityName = Device::GetAbilityName(bundleInfo);
    AddDrvExtConnNotify();
    int32_t ret = DriverExtensionController::GetInstance().ConnectDriverExtension(
        bundleName, abilityName, connectNofitier_, busDevId);
    // RESOLVE_ABILITY_ERR maybe due to bms is not ready in the boot process, sleep 100ms and try again
    int retry = 0;
    const int retryTimes = 30;
    const int waitTimeMs = 100;
    const int msToUs = 1000;
    while (ret != UsbErrCode::EDM_OK && retry < retryTimes) {
        EDM_LOGW(MODULE_DEV_MGR,
            "%{public}s sleep 100ms to reconnect %{public}s %{public}s, ret %{public}d, retry %{public}d",
            __func__, bundleName.c_str(), abilityName.c_str(), ret, retry);
        usleep(waitTimeMs * msToUs);
        drvExtRemote_ = nullptr;
        connectNofitier_->ClearDrvExtConnectionInfo();
        ret = DriverExtensionController::GetInstance().ConnectDriverExtension(
            bundleName, abilityName, connectNofitier_, busDevId);
        retry++;
    }
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to connect driver extension");
        return ret;
    }
    return UsbErrCode::EDM_OK;
}

int32_t Device::Connect(const sptr<IDriverExtMgrCallback> &connectCallback)
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    std::lock_guard<std::recursive_mutex> lock(deviceMutex_);
    uint64_t deviceId = GetDeviceInfo()->GetDeviceId();
    if (drvExtRemote_ != nullptr) {
        connectCallback->OnConnect(deviceId, drvExtRemote_, {UsbErrCode::EDM_OK, ""});
        int32_t ret = RegisterDrvExtMgrCallback(connectCallback);
        if (ret != UsbErrCode::EDM_OK) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to register callback object");
            return ret;
        }
        return ret;
    }

    int32_t ret = RegisterDrvExtMgrCallback(connectCallback);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to register callback object");
        return ret;
    }

    UpdateDrvExtConnNotify();
    std::string bundleInfo = GetBundleInfo();
    std::string bundleName = Device::GetBundleName(bundleInfo);
    std::string abilityName = Device::GetAbilityName(bundleInfo);
    AddDrvExtConnNotify();
    uint32_t busDevId = GetDeviceInfo()->GetBusDevId();
    ret = DriverExtensionController::GetInstance().ConnectDriverExtension(
        bundleName, abilityName, connectNofitier_, busDevId);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to connect driver extension");
        UnregisterDrvExtMgrCallback(connectCallback);
        return ret;
    }
    return UsbErrCode::EDM_OK;
}

int32_t Device::Disconnect()
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    std::lock_guard<std::recursive_mutex> lock(deviceMutex_);
    if (connectNofitier_ != nullptr && connectNofitier_->IsInvalidDrvExtConnectionInfo()) {
        EDM_LOGI(MODULE_DEV_MGR, "driver extension has been disconnected");
        return UsbErrCode::EDM_OK;
    }
    uint32_t busDevId = GetDeviceInfo()->GetBusDevId();
    std::string bundleInfo = GetBundleInfo();
    std::string bundleName = Device::GetBundleName(bundleInfo);
    std::string abilityName = Device::GetAbilityName(bundleInfo);
    int32_t ret = DriverExtensionController::GetInstance().DisconnectDriverExtension(
        bundleName, abilityName, connectNofitier_, busDevId);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to disconnect driver extension");
        return ret;
    }

    return UsbErrCode::EDM_OK;
}

void Device::OnConnect(const sptr<IRemoteObject> &remote, int resultCode)
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    if (remote == nullptr || resultCode != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to connect driver extension %{public}d", resultCode);
    }

    std::lock_guard<std::recursive_mutex> lock(deviceMutex_);
    drvExtRemote_ = remote;

    // notify application
    for (auto &callback : callbacks_) {
        callback->OnConnect(GetDeviceInfo()->GetDeviceId(), drvExtRemote_, {static_cast<UsbErrCode>(resultCode), ""});
    }
}

void Device::OnDisconnect(int resultCode)
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    if (resultCode != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to disconnect driver extension %{public}d", resultCode);
    }

    std::lock_guard<std::recursive_mutex> lock(deviceMutex_);
    drvExtRemote_ = nullptr;
    connectNofitier_->ClearDrvExtConnectionInfo();
    for (auto &callback : callbacks_) {
        callback->OnUnBind(GetDeviceInfo()->GetDeviceId(), {static_cast<UsbErrCode>(resultCode), ""});
        callback->OnDisconnect(GetDeviceInfo()->GetDeviceId(), {static_cast<UsbErrCode>(resultCode), ""});
    }
    callbacks_.clear();
    if (IsUnRegisted()) {
        ExtDeviceManager::GetInstance().RemoveDeviceOfDeviceMap(shared_from_this());
    }
    std::string bundleInfo = GetBundleInfo();
    std::string bundleName = Device::GetBundleName(bundleInfo);
    std::string abilityName = Device::GetAbilityName(bundleInfo);
    DriverExtensionController::GetInstance().StopDriverExtension(bundleName, abilityName);
    ExtDeviceManager::GetInstance().UnLoadSA();
}

void Device::UpdateDrvExtConnNotify()
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    connectNofitier_ = std::make_shared<DrvExtConnNotify>(shared_from_this());
}

int32_t Device::RegisterDrvExtMgrCallback(const sptr<IDriverExtMgrCallback> &callback)
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    if (callback == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to register callback because of invalid callback object");
        return UsbErrCode::EDM_ERR_INVALID_OBJECT;
    }

    std::lock_guard<std::recursive_mutex> lock(deviceMutex_);
    auto ret = callbacks_.insert(callback);
    if (ret.second == false) {
        EDM_LOGD(MODULE_DEV_MGR, "insert callback object repeatedly");
    }

    if (!RegisteDeathRecipient(callback)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to register death recipient");
        return UsbErrCode::EDM_NOK;
    }

    return UsbErrCode::EDM_OK;
}

void Device::UnregisterDrvExtMgrCallback(const sptr<IDriverExtMgrCallback> &callback)
{
    std::lock_guard<std::recursive_mutex> lock(deviceMutex_);
    auto resIter =
        std::find_if(callbacks_.begin(), callbacks_.end(), [&callback](const sptr<IDriverExtMgrCallback> &element) {
            return element->AsObject() == callback->AsObject();
        });
    if (resIter != callbacks_.end()) {
        callbacks_.erase(resIter);
    }
}

void Device::UnregisterDrvExtMgrCallback(const wptr<IRemoteObject> &object)
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    std::lock_guard<std::recursive_mutex> lock(deviceMutex_);
    auto resIter =
        std::find_if(callbacks_.begin(), callbacks_.end(), [&object](const sptr<IDriverExtMgrCallback> &element) {
            return element->AsObject() == object;
        });
    if (resIter != callbacks_.end()) {
        callbacks_.erase(resIter);
    }
}

bool Device::RegisteDeathRecipient(const sptr<IDriverExtMgrCallback> &callback)
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    sptr<DriverExtMgrCallbackDeathRecipient> callbackDeathRecipient =
        new DriverExtMgrCallbackDeathRecipient(shared_from_this());
    return callback->AsObject()->AddDeathRecipient(callbackDeathRecipient);
}

void DriverExtMgrCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    auto device = device_.lock();
    if (device == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "invalid device object");
        return;
    }

    device->UnregisterDrvExtMgrCallback(remote);
}

int32_t DrvExtConnNotify::OnConnectDone(const sptr<IRemoteObject> &remote, int resultCode)
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    auto device = device_.lock();
    if (device == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "invalid device object");
        return UsbErrCode::EDM_ERR_INVALID_OBJECT;
    }

    device->OnConnect(remote, resultCode);
    return UsbErrCode::EDM_OK;
}

int32_t DrvExtConnNotify::OnDisconnectDone(int resultCode)
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    auto device = device_.lock();
    if (device == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "invalid device object");
        return UsbErrCode::EDM_ERR_INVALID_OBJECT;
    }

    device->OnDisconnect(resultCode);
    return UsbErrCode::EDM_OK;
}
} // namespace ExternalDeviceManager
} // namespace OHOS
