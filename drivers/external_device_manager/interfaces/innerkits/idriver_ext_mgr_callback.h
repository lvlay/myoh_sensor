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

#ifndef IDRIVER_EXTENSION_MANAGER_CALLBACK_H
#define IDRIVER_EXTENSION_MANAGER_CALLBACK_H

#include <string>
#include <iremote_broker.h>
#include "driver_ext_mgr_types.h"
#include "edm_errors.h"

namespace OHOS {
namespace ExternalDeviceManager {
class IDriverExtMgrCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.driver.IDriverExtMgrCallback");

    virtual ~IDriverExtMgrCallback() = default;

    enum class DriverExtMgrCallbackInterfaceCode : uint32_t {
        ON_CONNECT,
        ON_DISCONNECT,
        ON_UNBIND,
    };

    virtual void OnConnect(uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj, const ErrMsg &errMsg) = 0;
    virtual void OnDisconnect(uint64_t deviceId, const ErrMsg &errMsg) = 0;
    virtual void OnUnBind(uint64_t deviceId, const ErrMsg &errMsg) = 0;
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // IDRIVER_EXTENSION_MANAGER_CALLBACK_H
