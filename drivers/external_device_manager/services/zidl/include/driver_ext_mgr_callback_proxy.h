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

#ifndef DRIVER_EXTENSION_MANAGER_CALLBACK_PROXY_H
#define DRIVER_EXTENSION_MANAGER_CALLBACK_PROXY_H

#include <iremote_proxy.h>
#include "idriver_ext_mgr_callback.h"

namespace OHOS {
namespace ExternalDeviceManager {
class DriverExtMgrCallbackProxy : public IRemoteProxy<IDriverExtMgrCallback> {
public:
    explicit DriverExtMgrCallbackProxy(const sptr<IRemoteObject> &remote) : IRemoteProxy<IDriverExtMgrCallback>(remote)
    {
    }

    ~DriverExtMgrCallbackProxy() = default;

    virtual void OnConnect(uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj, const ErrMsg &errMsg) override;
    virtual void OnDisconnect(uint64_t deviceId, const ErrMsg &errMsg) override;
    virtual void OnUnBind(uint64_t deviceId, const ErrMsg &errMsg) override;

private:
    static inline BrokerDelegator<DriverExtMgrCallbackProxy> delegator_;
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // DRIVER_EXTENSION_MANAGER_CALLBACK_PROXY_H
