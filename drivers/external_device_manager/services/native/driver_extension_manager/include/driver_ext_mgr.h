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

#ifndef DRIVER_EXTENSION_MANAGER_H
#define DRIVER_EXTENSION_MANAGER_H
#include <singleton.h>
#include <system_ability.h>
#include <future>

#include "driver_ext_mgr_stub.h"

namespace OHOS {
namespace ExternalDeviceManager {
class DriverExtMgr : public SystemAbility, public DriverExtMgrStub {
    DECLARE_SYSTEM_ABILITY(DriverExtMgr)
    DECLARE_DELAYED_SINGLETON(DriverExtMgr);

public:
    void OnStart() override;
    void OnStop() override;
    int Dump(int fd, const std::vector<std::u16string> &args) override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    UsbErrCode QueryDevice(uint32_t busType, std::vector<std::shared_ptr<DeviceData>> &devices) override;
    UsbErrCode BindDevice(uint64_t deviceId, const sptr<IDriverExtMgrCallback> &connectCallback) override;
    UsbErrCode UnBindDevice(uint64_t deviceId) override;
    UsbErrCode QueryDeviceInfo(std::vector<std::shared_ptr<DeviceInfoData>> &deviceInfos,
        bool isByDeviceId = false, const uint64_t deviceId = 0) override;
    UsbErrCode QueryDriverInfo(std::vector<std::shared_ptr<DriverInfoData>> &driverInfos,
        bool isByDriverUid = false, const std::string &driverUid = "") override;

private:
    std::mutex connectCallbackMutex;
    std::mutex promiseMutex_;
    std::promise<int32_t> bmsPromise_;
    std::promise<int32_t> accountPromise_;
    std::promise<int32_t> commEventPromise_;
    std::shared_future<int32_t> bmsFuture_ = bmsPromise_.get_future();
    std::shared_future<int32_t> accountFuture_ = accountPromise_.get_future();
    std::shared_future<int32_t> commEventFuture_ = commEventPromise_.get_future();
    bool bmsPromiseUsed_ = false;
    bool accountPromiseUsed_ = false;
    bool cesPromiseUsed_ = false;
    std::map<uint64_t, std::vector<sptr<IDriverExtMgrCallback>>> connectCallbackMap;
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // DRIVER_EXTENSION_MANAGER_H
