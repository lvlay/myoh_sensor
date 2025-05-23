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

#ifndef DRIVER_EXTENSION_MANAGER_STUB_H
#define DRIVER_EXTENSION_MANAGER_STUB_H

#include <iremote_stub.h>
#include <message_option.h>
#include <nocopyable.h>
#include "idriver_ext_mgr.h"

namespace OHOS {
namespace ExternalDeviceManager {
class DriverExtMgrStub : public IRemoteStub<IDriverExtMgr> {
public:
    DISALLOW_COPY_AND_MOVE(DriverExtMgrStub);

    DriverExtMgrStub() = default;

    virtual ~DriverExtMgrStub() = default;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t OnQueryDevice(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    int32_t OnBindDevice(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    int32_t OnUnBindDevice(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    int32_t OnQueryDeviceInfo(MessageParcel &data, MessageParcel &reply, MessageOption &option);
    int32_t OnQueryDriverInfo(MessageParcel &data, MessageParcel &reply, MessageOption &option);
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // DRIVER_EXTENSION_MANAGER_STUB_H
