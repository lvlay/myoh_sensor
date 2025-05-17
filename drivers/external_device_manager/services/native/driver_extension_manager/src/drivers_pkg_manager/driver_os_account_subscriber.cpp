/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "driver_os_account_subscriber.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace ExternalDeviceManager {
DriverOsAccountSwitched::DriverOsAccountSwitched(const OsAccountSubscribeInfo &subscribeInfo,
    sptr<DrvBundleStateCallback> callback)
    : OsAccountSubscriber(subscribeInfo)
{
    bundleStateCallback_ = callback;
}

void DriverOsAccountSwitched::OnAccountsChanged(const int &id)
{
    EDM_LOGI(MODULE_PKG_MGR, "DriverOsAccountSwitched::OnAccountsChanged, id=%{public}d", id);
}

void DriverOsAccountSwitched::OnAccountsSwitch(const int &newId, const int &oldId)
{
    EDM_LOGI(MODULE_PKG_MGR, "OnAccountsSwitched, newId=%{public}d, oldId=%{public}d", newId, oldId);
    if (bundleStateCallback_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "OnAccountsSwitched bundleStateCallback_ is null");
        return;
    }
    bundleStateCallback_->ResetInitOnce();
    bundleStateCallback_->GetAllDriverInfosAsync(true);
}

DriverOsAccountSwitching::DriverOsAccountSwitching(const OsAccountSubscribeInfo &subscribeInfo,
    sptr<DrvBundleStateCallback> callback)
    : OsAccountSubscriber(subscribeInfo)
{
    bundleStateCallback_ = callback;
}

void DriverOsAccountSwitching::OnAccountsChanged(const int &id)
{
    EDM_LOGI(MODULE_PKG_MGR, "DriverOsAccountSwitching::OnAccountsChanged, id=%{public}d", id);
}

void DriverOsAccountSwitching::OnAccountsSwitch(const int &newId, const int &oldId)
{
    EDM_LOGI(MODULE_PKG_MGR, "OnAccountsSwitching, newId=%{public}d, oldId=%{public}d", newId, oldId);
    if (bundleStateCallback_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "OnAccountsSwitching bundleStateCallback_ is null");
        return;
    }
    bundleStateCallback_->ResetMatchedBundles(oldId);
}
}
}