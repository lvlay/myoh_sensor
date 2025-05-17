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

#ifndef DRIVER_EXTENSION_MANAGER_PROXY_H
#define DRIVER_EXTENSION_MANAGER_PROXY_H

#include "idriver_ext_mgr.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace ExternalDeviceManager {
class DriverExtMgrProxy : public IRemoteProxy<IDriverExtMgr> {
public:
    explicit DriverExtMgrProxy(const sptr<IRemoteObject> &remote) : IRemoteProxy<IDriverExtMgr>(remote) {}
    ~DriverExtMgrProxy() = default;

    UsbErrCode QueryDevice(uint32_t busType, std::vector<std::shared_ptr<DeviceData>> &devices) override;
    UsbErrCode BindDevice(uint64_t deviceId, const sptr<IDriverExtMgrCallback> &connectCallback) override;
    UsbErrCode UnBindDevice(uint64_t deviceId) override;
    UsbErrCode QueryDeviceInfo(std::vector<std::shared_ptr<DeviceInfoData>> &deviceInfos,
        bool isByDeviceId = false, const uint64_t deviceId = 0) override;
    UsbErrCode QueryDriverInfo(std::vector<std::shared_ptr<DriverInfoData>> &driverInfos,
        bool isByDriverUid = false, const std::string &driverUid = "") override;

private:
    static inline BrokerDelegator<DriverExtMgrProxy> delegator_;
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // DRIVER_EXTENSION_MANAGER_PROXY_H
