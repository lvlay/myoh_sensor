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
#ifndef DEVICE_MANAGER_MIDDLE_H
#define DEVICE_MANAGER_MIDDLE_H

#include "hilog_wrapper.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "idriver_ext_mgr_callback.h"
#include "driver_ext_mgr_callback_stub.h"
#include "driver_ext_mgr_client.h"

namespace OHOS {
namespace ExternalDeviceManager {
enum ErrorCode : int32_t {
    PERMISSION_DENIED = 201, // Use this error code when permission is denied.
    PERMISSION_NOT_SYSTEM_APP = 202,
    PARAMETER_ERROR = 401, // Use this error code when the input parameter type or range does not match.
    SERVICE_EXCEPTION = 22900001, // Use this error code when the service is exception.
    SERVICE_EXCEPTION_NEW = 26300001
};

class AsyncData : public RefBase {
public:
    uint64_t deviceId;
    napi_env env;
    napi_ref bindCallback;
    napi_ref onDisconnect;
    napi_ref unbindCallback;
    napi_deferred bindDeferred;
    napi_deferred unbindDeferred;
    sptr<IRemoteObject> drvExtObj;
    ErrMsg errMsg;
    ErrMsg unBindErrMsg;

    void DeleteNapiRef();

    ~AsyncData()
    {
        EDM_LOGE(MODULE_DEV_MGR, "Release callback data: %{public}016" PRIX64, deviceId);
        DeleteNapiRef();
        drvExtObj = nullptr;
    }
};

struct AsyncDataWorker {
    napi_env env = nullptr;
    napi_ref bindCallback = nullptr;
    napi_ref onDisconnect = nullptr;
    napi_ref unbindCallback = nullptr;
};

class DeviceManagerCallback : public DriverExtMgrCallbackStub {
public:
    void OnConnect(uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj, const ErrMsg &errMsg) override;

    void OnDisconnect(uint64_t deviceId, const ErrMsg &errMsg) override;

    void OnUnBind(uint64_t deviceId, const ErrMsg &errMsg) override;
};
}
}
#endif