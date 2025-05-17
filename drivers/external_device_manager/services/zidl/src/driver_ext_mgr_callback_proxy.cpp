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

#include "driver_ext_mgr_callback_proxy.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace ExternalDeviceManager {
void DriverExtMgrCallbackProxy::OnConnect(uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj, const ErrMsg &errMsg)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write interface token");
        return;
    }

    if (!errMsg.Marshalling(data)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write errMsg");
        return;
    }

    if (errMsg.IsOk()) {
        if (!data.WriteUint64(deviceId)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write deviceId");
            return;
        }

        if (!data.WriteRemoteObject(drvExtObj)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write drvExtObj");
            return;
        }
    }

    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(DriverExtMgrCallbackInterfaceCode::ON_CONNECT), data, reply, option);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "SendRequest is failed, ret: %{public}d", ret);
    }
}

void DriverExtMgrCallbackProxy::OnDisconnect(uint64_t deviceId, const ErrMsg &errMsg)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write interface token");
        return;
    }

    if (!errMsg.Marshalling(data)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write errMsg");
        return;
    }

    if (errMsg.IsOk()) {
        if (!data.WriteUint64(deviceId)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write deviceId");
            return;
        }
    }

    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DriverExtMgrCallbackInterfaceCode::ON_DISCONNECT), data, reply, option);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "SendRequest is failed, ret: %{public}d", ret);
    }
}

void DriverExtMgrCallbackProxy::OnUnBind(uint64_t deviceId, const ErrMsg &errMsg)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "remote is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write interface token");
        return;
    }

    if (!errMsg.Marshalling(data)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write errMsg");
        return;
    }

    if (errMsg.IsOk()) {
        if (!data.WriteUint64(deviceId)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write deviceId");
            return;
        }
    }

    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(DriverExtMgrCallbackInterfaceCode::ON_UNBIND), data, reply, option);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "SendRequest is failed, ret: %{public}d", ret);
    }
}
} // namespace ExternalDeviceManager
} // namespace OHOS
