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

#ifndef IDRIVER_EXTENSION_MANAGER_H
#define IDRIVER_EXTENSION_MANAGER_H
#include <vector>

#include "driver_ext_mgr_types.h"
#include "edm_errors.h"
#include "ext_object.h"
#include "hdf_ext_devmgr_interface_code.h"
#include "idriver_ext_mgr_callback.h"
#include "iremote_broker.h"

namespace OHOS {
namespace ExternalDeviceManager {
class IDriverExtMgr : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.driver.IDriverExtMgr");
    ~IDriverExtMgr() = default;

    virtual UsbErrCode QueryDevice(uint32_t busType, std::vector<std::shared_ptr<DeviceData>> &devices) = 0;
    virtual UsbErrCode BindDevice(uint64_t deviceId, const sptr<IDriverExtMgrCallback> &connectCallback) = 0;
    virtual UsbErrCode UnBindDevice(uint64_t deviceId) = 0;
    virtual UsbErrCode QueryDeviceInfo(std::vector<std::shared_ptr<DeviceInfoData>> &deviceInfos,
        bool isByDeviceId = false, const uint64_t deviceId = 0) = 0;
    virtual UsbErrCode QueryDriverInfo(std::vector<std::shared_ptr<DriverInfoData>> &driverInfos,
        bool isByDriverUid = false, const std::string &driverUid = "") = 0;
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // IDRIVER_EXTENSION_MANAGER_H