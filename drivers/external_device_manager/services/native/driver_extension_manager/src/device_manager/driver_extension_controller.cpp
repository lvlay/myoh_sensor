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

#include "ability_manager_client.h"
#include "hilog_wrapper.h"
#include "ability_connect_callback_stub.h"
#include "edm_errors.h"
#include "driver_extension_controller.h"
namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
IMPLEMENT_SINGLE_INSTANCE(DriverExtensionController);

class DriverExtensionController::DriverExtensionAbilityConnection : public OHOS::AAFwk::AbilityConnectionStub {
public:
    DriverExtensionAbilityConnection() = default;
    ~DriverExtensionAbilityConnection() = default;
    void OnAbilityConnectDone(
        const OHOS::AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override
    {
        EDM_LOGI(MODULE_EA_MGR,
            "OnAbilityConnectDone, bundle = %{public}s, ability = %{public}s, resultCode = %{public}d",
            element.GetBundleName().c_str(), element.GetAbilityName().c_str(), resultCode);
        remoteObject_ = remoteObject;
        auto cb = callback_.lock();
        if (cb != nullptr) {
            cb->OnConnectDone(remoteObject, resultCode);
        }
    }
    void OnAbilityDisconnectDone(
        const OHOS::AppExecFwk::ElementName &element, int resultCode) override
    {
        EDM_LOGI(MODULE_EA_MGR,
            "OnAbilityDisconnectDone, bundle = %{public}s,ability = %{public}s, resultCode = %{public}d",
            element.GetBundleName().c_str(), element.GetAbilityName().c_str(), resultCode);
        remoteObject_ = nullptr;
        auto cb = callback_.lock();
        if (cb != nullptr) {
            cb->OnDisconnectDone(resultCode);
        }
    }
    std::weak_ptr<IDriverExtensionConnectCallback> callback_;
    sptr<IRemoteObject> remoteObject_;
};

sptr<IRemoteObject> IDriverExtensionConnectCallback::GetRemoteObj()
{
    return info_->connectInner_->remoteObject_;
}

bool IDriverExtensionConnectCallback::IsConnectDone()
{
    return info_->connectInner_->remoteObject_ != nullptr;
}

int32_t DriverExtensionController::StartDriverExtension(
    std::string bundleName,
    std::string abilityName)
{
    EDM_LOGI(MODULE_EA_MGR, "Begin to start DriverExtension, bundle:%{public}s, ability:%{public}s", \
        bundleName.c_str(), abilityName.c_str());
    auto abmc = AAFwk::AbilityManagerClient::GetInstance();
    if (abmc == nullptr) {
        EDM_LOGE(MODULE_EA_MGR, "Get AMC Instance failed");
        return EDM_ERR_INVALID_OBJECT;
    }
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);

    auto ret = abmc->StartExtensionAbility(want, nullptr);
    if (ret != 0) {
        EDM_LOGE(MODULE_EA_MGR, "StartExtensionAbility failed %{public}d", ret);
        return ret;
    }
    EDM_LOGI(MODULE_EA_MGR, "StartExtensionAbility success");
    return EDM_OK;
}

int32_t DriverExtensionController::StopDriverExtension(
    std::string bundleName,
    std::string abilityName,
    int32_t userId)
{
    EDM_LOGI(MODULE_EA_MGR, "Begin to stop DriverExtension, bundle:%{public}s, ability:%{public}s", \
        bundleName.c_str(), abilityName.c_str());
    auto abmc = AAFwk::AbilityManagerClient::GetInstance();
    if (abmc == nullptr) {
        EDM_LOGE(MODULE_EA_MGR, "Get AMC Instance failed");
        return EDM_ERR_INVALID_OBJECT;
    }
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);
    auto ret = abmc->StopExtensionAbility(want, nullptr, userId);
    if (ret != 0) {
        EDM_LOGE(MODULE_EA_MGR, "StopExtensionAbility failed %{public}d", ret);
        return ret;
    }
    EDM_LOGI(MODULE_EA_MGR, "StopExtensionAbility success");
    return EDM_OK;
}
int32_t DriverExtensionController::ConnectDriverExtension(
    std::string bundleName,
    std::string abilityName,
    std::shared_ptr<IDriverExtensionConnectCallback> callback,
    uint32_t deviceId
)
{
    EDM_LOGI(MODULE_EA_MGR, "Begin to Connect DriverExtension, bundle:%{public}s, ability:%{public}s", \
        bundleName.c_str(), abilityName.c_str());

    if (callback == nullptr) {
        EDM_LOGE(MODULE_EA_MGR, "param callback is nullptr");
        return EDM_ERR_INVALID_PARAM;
    }
    if (callback->info_ != nullptr) {
        EDM_LOGE(MODULE_EA_MGR, "callback->info_ is not empty, please use an new callback");
        return EDM_ERR_INVALID_OBJECT;
    }

    callback->info_ = make_shared<DrvExtConnectionInfo>();
    callback->info_->bundleName_ = bundleName;
    callback->info_->abilityName_ = abilityName;
    callback->info_->deviceId_ = deviceId;
    callback->info_->connectInner_ = new DriverExtensionAbilityConnection();
    callback->info_->connectInner_->callback_ = callback;
    auto abmc = AAFwk::AbilityManagerClient::GetInstance();
    if (abmc == nullptr) {
        EDM_LOGE(MODULE_EA_MGR, "Get AMC Instance failed");
        return EDM_ERR_INVALID_OBJECT;
    }
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);
    want.SetParam("deviceId", static_cast<int>(deviceId));
    auto ret = abmc->ConnectAbility(want, callback->info_->connectInner_, -1);
    if (ret != 0) {
        EDM_LOGE(MODULE_EA_MGR, "ConnectExtensionAbility failed %{public}d", ret);
        return ret;
    }

    EDM_LOGI(MODULE_EA_MGR, "ConnectExtensionAbility success");
    return EDM_OK;
}
int32_t DriverExtensionController::DisconnectDriverExtension(
    std::string bundleName,
    std::string abilityName,
    std::shared_ptr<IDriverExtensionConnectCallback> callback,
    uint32_t deviceId
)
{
    EDM_LOGI(MODULE_EA_MGR, "Begin to Disconnect DriverExtension, bundle:%{public}s, ability:%{public}s", \
        bundleName.c_str(), abilityName.c_str());
    if (callback == nullptr) {
        EDM_LOGE(MODULE_EA_MGR, "param callback is nullptr");
        return EDM_ERR_INVALID_PARAM;
    }
    if (callback->info_ == nullptr) {
        EDM_LOGE(MODULE_EA_MGR, "param callback->info_ is nullptr");
        return EDM_ERR_INVALID_PARAM;
    }
    if (callback->info_->bundleName_ != bundleName ||
        callback->info_->abilityName_ != abilityName ||
        callback->info_->deviceId_ != deviceId) {
        EDM_LOGE(MODULE_EA_MGR, "bundleName, abilityName, or deviceId not match info in callback");
        return EDM_ERR_INVALID_OBJECT;
    }
    auto abmc = AAFwk::AbilityManagerClient::GetInstance();
    if (abmc == nullptr) {
        EDM_LOGE(MODULE_EA_MGR, "Get AMC Instance failed");
        return EDM_ERR_INVALID_OBJECT;
    }
    auto ret = abmc->DisconnectAbility(callback->info_->connectInner_);
    if (ret != 0) {
        EDM_LOGE(MODULE_EA_MGR, "DisconnectExtensionAbility failed %{public}d", ret);
        return ret;
    }
    EDM_LOGI(MODULE_EA_MGR, "DisconnectExtensionAbility success");
    return EDM_OK;
}
}
}