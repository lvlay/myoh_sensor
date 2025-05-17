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

#include "driver_ext_mgr_callback_stub.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace ExternalDeviceManager {
int DriverExtMgrCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    EDM_LOGD(MODULE_DEV_MGR, "cmd:%u, flags:%d", code, option.GetFlags());
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        EDM_LOGE(MODULE_DEV_MGR, "remote descriptor is not matched");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    switch (code) {
        case static_cast<uint32_t>(DriverExtMgrCallbackInterfaceCode::ON_CONNECT):
            return StubOnConnect(data, reply, option);
        case static_cast<uint32_t>(DriverExtMgrCallbackInterfaceCode::ON_DISCONNECT):
            return StubOnDisconnect(data, reply, option);
        case static_cast<uint32_t>(DriverExtMgrCallbackInterfaceCode::ON_UNBIND):
            return StubOnUnBind(data, reply, option);
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t DriverExtMgrCallbackStub::StubOnConnect(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    ErrMsg errMsg;
    if (!ErrMsg::UnMarshalling(data, errMsg)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read errMsg");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    uint64_t deviceId = 0;
    sptr<IRemoteObject> drvExtObj = nullptr;
    if (errMsg.IsOk()) {
        if (!data.ReadUint64(deviceId)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read deviceId");
            return UsbErrCode::EDM_ERR_INVALID_PARAM;
        }

        drvExtObj = data.ReadRemoteObject();
        if (drvExtObj == nullptr) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read drvExtObj");
            return UsbErrCode::EDM_ERR_INVALID_PARAM;
        }
    }

    OnConnect(deviceId, drvExtObj, errMsg);
    return UsbErrCode::EDM_OK;
}

int32_t DriverExtMgrCallbackStub::StubOnDisconnect(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    ErrMsg errMsg;
    if (!ErrMsg::UnMarshalling(data, errMsg)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read errMsg");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    uint64_t deviceId = 0;
    if (errMsg.IsOk()) {
        if (!data.ReadUint64(deviceId)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read deviceId");
            return UsbErrCode::EDM_ERR_INVALID_PARAM;
        }
    }

    OnDisconnect(deviceId, errMsg);
    return UsbErrCode::EDM_OK;
}

int32_t DriverExtMgrCallbackStub::StubOnUnBind(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    ErrMsg errMsg;
    if (!ErrMsg::UnMarshalling(data, errMsg)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read errMsg");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    uint64_t deviceId = 0;
    if (errMsg.IsOk()) {
        if (!data.ReadUint64(deviceId)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read deviceId");
            return UsbErrCode::EDM_ERR_INVALID_PARAM;
        }
    }

    OnUnBind(deviceId, errMsg);
    return UsbErrCode::EDM_OK;
}
} // namespace ExternalDeviceManager
} // namespace OHOS
