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

#ifndef DRIVER_OS_ACCOUNT_SUBSCRIBER_H
#define DRIVER_OS_ACCOUNT_SUBSCRIBER_H

#include "os_account_subscriber.h"
#include "drv_bundle_state_callback.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace OHOS::AccountSA;
class DriverOsAccountSwitched final : public OsAccountSubscriber {
public:
    explicit DriverOsAccountSwitched(const OsAccountSubscribeInfo &subscribeInfo,
        sptr<DrvBundleStateCallback> callback);

    void OnAccountsChanged(const int &id) override;
    void OnAccountsSwitch(const int &newId, const int &oldId) override;
private:
    sptr<DrvBundleStateCallback> bundleStateCallback_ = nullptr;
};

class DriverOsAccountSwitching final : public OsAccountSubscriber {
public:
    explicit DriverOsAccountSwitching(const OsAccountSubscribeInfo &subscribeInfo,
        sptr<DrvBundleStateCallback> callback);

    void OnAccountsChanged(const int &id) override;
    void OnAccountsSwitch(const int &newId, const int &oldId) override;
private:
    sptr<DrvBundleStateCallback> bundleStateCallback_ = nullptr;
};
}
}

#endif