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

#include <unistd.h>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <string>
#include <map>
#include <vector>
#include <uv.h>

#include "napi_remote_object.h"
#include "device_manager_middle.h"

namespace OHOS {
namespace ExternalDeviceManager {
constexpr int32_t PARAM_COUNT_0 = 0;
constexpr int32_t PARAM_COUNT_1 = 1;
constexpr int32_t PARAM_COUNT_2 = 2;
constexpr int32_t PARAM_COUNT_3 = 3;
constexpr uint64_t MAX_JS_NUMBER = 9007199254740991;

static const std::map<int32_t, std::string> ERROR_MESSAGES = {
    {SERVICE_EXCEPTION,  "ExternalDeviceManager service exception."},
    {PERMISSION_DENIED,  "Permission denied."},
    {PERMISSION_NOT_SYSTEM_APP,  "Permission denied. A non-system application cannot call a system API."},
    {PARAMETER_ERROR,  "The parameter check failed."},
    {SERVICE_EXCEPTION_NEW, "ExternalDeviceManager service exception."}
};

static std::mutex mapMutex;
static std::map<uint64_t, sptr<AsyncData>> g_callbackMap = {};
static DriverExtMgrClient &g_edmClient = DriverExtMgrClient::GetInstance();
static sptr<DeviceManagerCallback> g_edmCallback = new (std::nothrow) DeviceManagerCallback {};

static napi_value ConvertToBusinessError(const napi_env &env, const ErrMsg &errMsg);
static napi_value ConvertToJsDeviceId(const napi_env &env, uint64_t deviceId);
static napi_value GetCallbackResult(const napi_env &env, uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj);

static void BindDeviceWorkCb(uv_work_t *work, int status)
{
    if (work == nullptr) {
        return;
    }
    sptr<AsyncData> data(reinterpret_cast<AsyncData *>(work->data));
    data->DecStrongRef(nullptr);
    delete work;

    napi_value result = GetCallbackResult(data->env, data->deviceId, data->drvExtObj);
    napi_value err = ConvertToBusinessError(data->env, data->errMsg);
    if (data->bindCallback != nullptr) {
        napi_value callback;
        napi_get_reference_value(data->env, data->bindCallback, &callback);
        napi_value argv[PARAM_COUNT_2] = {err, result};
        napi_value callResult;
        napi_call_function(data->env, nullptr, callback, PARAM_COUNT_2, argv, &callResult);
        EDM_LOGI(MODULE_DEV_MGR, "bind device callback finish.");
    } else if (data->bindDeferred != nullptr) {
        if (data->errMsg.IsOk()) {
            napi_resolve_deferred(data->env, data->bindDeferred, result);
        } else {
            napi_reject_deferred(data->env, data->bindDeferred, err);
        }
        EDM_LOGI(MODULE_DEV_MGR, "bind device promise finish.");
    }
}

void UvDeleteRef(uv_work_t *work, int status)
{
    if (work == nullptr) {
        return;
    }
    AsyncDataWorker *data = static_cast<AsyncDataWorker *>(work->data);
    if (data == nullptr) {
        delete work;
        work = nullptr;
        return;
    }
    if (data->bindCallback != nullptr) {
        napi_delete_reference(data->env, data->bindCallback);
    }
    if (data->onDisconnect != nullptr) {
        napi_delete_reference(data->env, data->onDisconnect);
    }
    if (data->unbindCallback != nullptr) {
        napi_delete_reference(data->env, data->unbindCallback);
    }
    delete data;
    data = nullptr;
    delete work;
}

void AsyncData::DeleteNapiRef()
{
    if (env == nullptr) {
        return;
    }
    uv_loop_t* loop = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_uv_event_loop(env, &loop));
    AsyncDataWorker *data = new (std::nothrow) AsyncDataWorker();
    if (data == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "new AsyncDataWorker fail");
        return;
    }
    data->env = env;
    data->bindCallback = bindCallback;
    data->onDisconnect = onDisconnect;
    data->unbindCallback = unbindCallback;

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "new work fail");
        delete data;
        data = nullptr;
        return;
    }
    work->data = static_cast<void *>(data);
    auto ret = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, UvDeleteRef);
    if (ret != 0) {
        delete data;
        data = nullptr;
        delete work;
        work = nullptr;
    }
}

void DeviceManagerCallback::OnConnect(uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj, const ErrMsg &errMsg)
{
    EDM_LOGE(MODULE_DEV_MGR, "bind device callback: %{public}016" PRIX64, deviceId);
    std::lock_guard<std::mutex> mapLock(mapMutex);
    if (g_callbackMap.count(deviceId) == 0) {
        EDM_LOGE(MODULE_DEV_MGR, "device OnConnect is null");
        return;
    }

    auto asyncData = g_callbackMap[deviceId];
    if (!errMsg.IsOk()) {
        g_callbackMap.erase(deviceId);
    }
    uv_loop_t* loop = nullptr;
    NAPI_CALL_RETURN_VOID(asyncData->env, napi_get_uv_event_loop(asyncData->env, &loop));
    uv_work_t* work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "new work fail");
        return;
    }
    asyncData->drvExtObj = drvExtObj;
    asyncData->errMsg = errMsg;
    asyncData->IncStrongRef(nullptr);
    work->data = asyncData.GetRefPtr();
    auto ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, BindDeviceWorkCb);
    if (ret != 0) {
        delete work;
        asyncData->DecStrongRef(nullptr);
    }
}

static void DisConnectWorkCb(uv_work_t *work, int status)
{
    if (work == nullptr) {
            return;
        }
        sptr<AsyncData> data(reinterpret_cast<AsyncData*>(work->data));
        data->DecStrongRef(nullptr);
        delete work;

        napi_value disConnCallback;
        napi_status napiSatus = napi_get_reference_value(data->env, data->onDisconnect, &disConnCallback);
        if (napiSatus == napi_ok) {
            napi_value err = ConvertToBusinessError(data->env, data->errMsg);
            napi_value result = ConvertToJsDeviceId(data->env, data->deviceId);
            napi_value argv[PARAM_COUNT_2] = {err, result};
            napi_value callResult;
            napi_call_function(data->env, nullptr, disConnCallback, PARAM_COUNT_2, argv, &callResult);
            EDM_LOGI(MODULE_DEV_MGR, "onDisconnect callback finish.");
        }

        napi_value err = ConvertToBusinessError(data->env, data->unBindErrMsg);
        napi_value result = ConvertToJsDeviceId(data->env, data->deviceId);
        if (data->unbindCallback != nullptr) {
            napi_value callback;
            NAPI_CALL_RETURN_VOID(data->env, napi_get_reference_value(data->env, data->unbindCallback, &callback));

            napi_value argv[PARAM_COUNT_2] = {err, result};
            napi_value callResult;
            napi_call_function(data->env, nullptr, callback, PARAM_COUNT_2, argv, &callResult);
            EDM_LOGI(MODULE_DEV_MGR, "unbind device callback finish.");
        } else if (data->unbindDeferred != nullptr) {
            if (data->unBindErrMsg.IsOk()) {
                napi_resolve_deferred(data->env, data->unbindDeferred, result);
            } else {
                napi_reject_deferred(data->env, data->unbindDeferred, err);
            }
            EDM_LOGI(MODULE_DEV_MGR, "unbind device promise finish.");
        }
}

void DeviceManagerCallback::OnDisconnect(uint64_t deviceId, const ErrMsg &errMsg)
{
    EDM_LOGE(MODULE_DEV_MGR, "device onDisconnect: %{public}016" PRIX64, deviceId);
    std::lock_guard<std::mutex> mapLock(mapMutex);
    if (g_callbackMap.count(deviceId) == 0) {
        EDM_LOGE(MODULE_DEV_MGR, "device callback map is null");
        return;
    }

    auto asyncData = g_callbackMap[deviceId];
    g_callbackMap.erase(deviceId);
    if (asyncData == nullptr || (asyncData->onDisconnect == nullptr && asyncData->unbindCallback == nullptr
        && asyncData->unbindDeferred == nullptr)) {
        EDM_LOGE(MODULE_DEV_MGR, "device callback is null");
        return;
    }
    uv_loop_t* loop = nullptr;
    NAPI_CALL_RETURN_VOID(asyncData->env, napi_get_uv_event_loop(asyncData->env, &loop));
    uv_work_t* work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "new work fail");
        return;
    }
    asyncData->errMsg = errMsg;
    asyncData->IncStrongRef(nullptr);
    work->data = asyncData.GetRefPtr();
    auto ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, DisConnectWorkCb);
    if (ret != 0) {
        delete work;
        asyncData->DecStrongRef(nullptr);
    }
}

void DeviceManagerCallback::OnUnBind(uint64_t deviceId, const ErrMsg &errMsg)
{
    EDM_LOGI(MODULE_DEV_MGR, "unbind device callback: %{public}016" PRIX64, deviceId);
    std::lock_guard<std::mutex> mapLock(mapMutex);
    if (g_callbackMap.count(deviceId) == 0) {
        EDM_LOGE(MODULE_DEV_MGR, "device unbind map is null");
        return;
    }

    auto asyncData = g_callbackMap[deviceId];
    if (asyncData == nullptr || (asyncData->unbindCallback == nullptr && asyncData->unbindDeferred == nullptr)) {
        EDM_LOGE(MODULE_DEV_MGR, "device unbind is null");
        return;
    }
    asyncData->unBindErrMsg = errMsg;
}

static bool IsMatchType(const napi_env &env, const napi_value &value, const napi_valuetype &type)
{
    napi_valuetype paramType = napi_undefined;
    napi_typeof(env, value, &paramType);
    return paramType == type;
}

static napi_value GetCallbackResult(const napi_env &env, uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj)
{
    napi_value id = ConvertToJsDeviceId(env, deviceId);

    napi_value remoteObj;
    if (drvExtObj == nullptr) {
        napi_get_undefined(env, &remoteObj);
        EDM_LOGE(MODULE_DEV_MGR, "Remote obj is null.");
    } else {
        EDM_LOGI(MODULE_DEV_MGR, "Remote obj create.");
        remoteObj = NAPI_ohos_rpc_CreateJsRemoteObject(env, drvExtObj);
    }

    napi_value result;
    napi_create_object(env, &result);
    napi_set_named_property(env, result, "deviceId", id);
    napi_set_named_property(env, result, "remote", remoteObj);

    return result;
}

static std::optional<std::string> GetNapiError(int32_t errorCode)
{
    auto iter = ERROR_MESSAGES.find(errorCode);
    if (iter != ERROR_MESSAGES.end()) {
        return iter->second;
    }
    return std::nullopt;
}

static napi_value CreateBusinessError(const napi_env &env, const int32_t errCode, const std::string &errMessage)
{
    napi_value businessError = nullptr;
    napi_value code = nullptr;
    napi_value msg = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &code));
    NAPI_CALL(env, napi_create_string_utf8(env, errMessage.c_str(), NAPI_AUTO_LENGTH, &msg));
    napi_create_error(env, nullptr, msg, &businessError);
    napi_set_named_property(env, businessError, "code", code);
    napi_set_named_property(env, businessError, "message", msg);
    return businessError;
}

static napi_value ConvertToBusinessError(const napi_env &env, const ErrMsg &errMsg)
{
    napi_value businessError = nullptr;
    if (errMsg.IsOk()) {
        napi_get_undefined(env, &businessError);
        return businessError;
    }

    auto msgString = GetNapiError(SERVICE_EXCEPTION);
    if (!msgString) {
        napi_get_undefined(env, &businessError);
        return businessError;
    }

    napi_value code = nullptr;
    napi_value msg = nullptr;
    NAPI_CALL(env, napi_create_int32(env, SERVICE_EXCEPTION, &code));
    NAPI_CALL(env, napi_create_string_utf8(env, msgString.value().c_str(), NAPI_AUTO_LENGTH, &msg));
    napi_create_error(env, nullptr, msg, &businessError);
    napi_set_named_property(env, businessError, "code", code);
    napi_set_named_property(env, businessError, "message", msg);
    return businessError;
}

static napi_value ConvertToJsDeviceId(const napi_env &env, uint64_t deviceId)
{
    napi_value result;
    if (deviceId > MAX_JS_NUMBER) {
        NAPI_CALL(env, napi_create_bigint_uint64(env, deviceId, &result));
    } else {
        NAPI_CALL(env, napi_create_int64(env, deviceId, &result));
    }
    return result;
}

void ThrowErr(const napi_env &env, const int32_t errCode, const std::string &printMsg)
{
    EDM_LOGE(MODULE_DEV_MGR, "message: %{public}s, code: %{public}d", printMsg.c_str(), errCode);
    auto msg = GetNapiError(errCode);
    if (!msg) {
        EDM_LOGE(MODULE_DEV_MGR, "errCode: %{public}d is invalid", errCode);
        return;
    }
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    napi_value error = CreateBusinessError(env, errCode, msg.value());
    napi_throw(env, error);
    napi_close_handle_scope(env, scope);
}

static napi_value ConvertDeviceToJsDevice(napi_env& env, std::shared_ptr<DeviceData> device)
{
    napi_value object;
    napi_value value;
    NAPI_CALL(env, napi_create_object(env, &object));
    NAPI_CALL(env, napi_create_int32(env, device->busType, &value));
    NAPI_CALL(env, napi_set_named_property(env, object, "busType", value));
    value = ConvertToJsDeviceId(env, device->deviceId);
    NAPI_CALL(env, napi_set_named_property(env, object, "deviceId", value));
    if (device->busType == BusType::BUS_TYPE_USB) {
        std::shared_ptr<USBDevice> usb = std::static_pointer_cast<USBDevice>(device);
        NAPI_CALL(env, napi_create_uint32(env, usb->vendorId, &value));
        NAPI_CALL(env, napi_set_named_property(env, object, "vendorId", value));
        NAPI_CALL(env, napi_create_uint32(env, usb->productId, &value));
        NAPI_CALL(env, napi_set_named_property(env, object, "productId", value));
    }

    return object;
}

static napi_value ConvertToJsUSBInterfaceDesc(napi_env& env, std::shared_ptr<USBInterfaceDesc> desc)
{
    if (desc == nullptr) {
        return nullptr;
    }
    napi_value object;
    napi_value value;
    NAPI_CALL(env, napi_create_object(env, &object));
    NAPI_CALL(env, napi_create_uint32(env, desc->bInterfaceNumber, &value));
    NAPI_CALL(env, napi_set_named_property(env, object, "bInterfaceNumber", value));
    NAPI_CALL(env, napi_create_uint32(env, desc->bClass, &value));
    NAPI_CALL(env, napi_set_named_property(env, object, "bClass", value));
    NAPI_CALL(env, napi_create_uint32(env, desc->bSubClass, &value));
    NAPI_CALL(env, napi_set_named_property(env, object, "bSubClass", value));
    NAPI_CALL(env, napi_create_uint32(env, desc->bProtocol, &value));
    NAPI_CALL(env, napi_set_named_property(env, object, "bProtocol", value));
    return object;
}

static napi_value ConvertToJsDeviceInfo(napi_env& env, std::shared_ptr<DeviceInfoData> deviceInfo)
{
    EDM_LOGD(MODULE_DEV_MGR, "ConvertToJsDeviceInfo start");
    if (deviceInfo == nullptr) {
        return nullptr;
    }
    napi_value object;
    napi_value value;
    NAPI_CALL(env, napi_create_object(env, &object));
    value = ConvertToJsDeviceId(env, deviceInfo->deviceId);
    NAPI_CALL(env, napi_set_named_property(env, object, "deviceId", value));
    NAPI_CALL(env, napi_get_boolean(env, deviceInfo->isDriverMatched, &value));
    NAPI_CALL(env, napi_set_named_property(env, object, "isDriverMatched", value));
    if (deviceInfo->isDriverMatched) {
        NAPI_CALL(env, napi_create_string_utf8(env, deviceInfo->driverUid.c_str(),
            deviceInfo->driverUid.size(), &value));
        NAPI_CALL(env, napi_set_named_property(env, object, "driverUid", value));
    }
    BusType busType = DeviceInfoData::GetBusTypeByDeviceId(deviceInfo->deviceId);
    if (busType == BusType::BUS_TYPE_USB) {
        std::shared_ptr<USBDeviceInfoData> usbDeviceInfo = std::static_pointer_cast<USBDeviceInfoData>(deviceInfo);
        NAPI_CALL(env, napi_create_uint32(env, usbDeviceInfo->vendorId, &value));
        NAPI_CALL(env, napi_set_named_property(env, object, "vendorId", value));
        NAPI_CALL(env, napi_create_uint32(env, usbDeviceInfo->productId, &value));
        NAPI_CALL(env, napi_set_named_property(env, object, "productId", value));

        napi_value interfaceDescList;
        NAPI_CALL(env, napi_create_array(env, &interfaceDescList));
        EDM_LOGD(MODULE_DEV_MGR, "interfaceDescList size = %{public}zu", usbDeviceInfo->interfaceDescList.size());
        for (size_t i = 0; i < usbDeviceInfo->interfaceDescList.size(); i++) {
            napi_value element = ConvertToJsUSBInterfaceDesc(env, usbDeviceInfo->interfaceDescList[i]);
            NAPI_CALL(env, napi_set_element(env, interfaceDescList, i, element));
        }
        NAPI_CALL(env, napi_set_named_property(env, object, "interfaceDescList", interfaceDescList));
    }
    EDM_LOGD(MODULE_DEV_MGR, "ConvertToJsDeviceInfo end");
    return object;
}

static napi_value ConvertToJsDriverInfo(napi_env& env, std::shared_ptr<DriverInfoData> driverInfo)
{
    if (driverInfo == nullptr) {
        return nullptr;
    }
    napi_value object;
    napi_value value;
    NAPI_CALL(env, napi_create_object(env, &object));
    NAPI_CALL(env, napi_create_int32(env, driverInfo->busType, &value));
    NAPI_CALL(env, napi_set_named_property(env, object, "busType", value));
    NAPI_CALL(env, napi_create_string_utf8(env, driverInfo->driverUid.c_str(), driverInfo->driverUid.size(), &value));
    NAPI_CALL(env, napi_set_named_property(env, object, "driverUid", value));
    NAPI_CALL(env, napi_create_string_utf8(env, driverInfo->driverName.c_str(), driverInfo->driverName.size(), &value));
    NAPI_CALL(env, napi_set_named_property(env, object, "driverName", value));
    NAPI_CALL(env, napi_create_string_utf8(env, driverInfo->bundleSize.c_str(), driverInfo->bundleSize.size(), &value));
    NAPI_CALL(env, napi_set_named_property(env, object, "driverSize", value));
    NAPI_CALL(env, napi_create_string_utf8(env, driverInfo->version.c_str(), driverInfo->version.size(), &value));
    NAPI_CALL(env, napi_set_named_property(env, object, "driverVersion", value));
    NAPI_CALL(env, napi_create_string_utf8(env, driverInfo->description.c_str(),
        driverInfo->description.size(), &value));
    NAPI_CALL(env, napi_set_named_property(env, object, "description", value));
    if (driverInfo->busType == BusType::BUS_TYPE_USB) {
        std::shared_ptr<USBDriverInfoData> usbDriverInfo = std::static_pointer_cast<USBDriverInfoData>(driverInfo);
        napi_value pids;
        NAPI_CALL(env, napi_create_array(env, &pids));
        for (size_t i = 0; i < usbDriverInfo->pids.size(); i++) {
            NAPI_CALL(env, napi_create_uint32(env, usbDriverInfo->pids[i], &value));
            NAPI_CALL(env, napi_set_element(env, pids, i, value));
        }
        NAPI_CALL(env, napi_set_named_property(env, object, "productIdList", pids));
        napi_value vids;
        NAPI_CALL(env, napi_create_array(env, &vids));
        for (size_t i = 0; i < usbDriverInfo->vids.size(); i++) {
            NAPI_CALL(env, napi_create_uint32(env, usbDriverInfo->vids[i], &value));
            NAPI_CALL(env, napi_set_element(env, vids, i, value));
        }
        NAPI_CALL(env, napi_set_named_property(env, object, "vendorIdList", vids));
    }
    return object;
}

static napi_value QueryDevices(napi_env env, napi_callback_info info)
{
    EDM_LOGI(MODULE_DEV_MGR, "queryDevices start");
    size_t argc = PARAM_COUNT_1;
    napi_value argv[PARAM_COUNT_1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    int32_t busType = BusType::BUS_TYPE_USB;
    if (argc > 0 && IsMatchType(env, argv[0], napi_number)) {
        NAPI_CALL(env, napi_get_value_int32(env, argv[0], &busType));
        EDM_LOGI(MODULE_DEV_MGR, "bus type is %{public}d", busType);
    }

    std::vector<std::shared_ptr<DeviceData>> devices;
    UsbErrCode retCode = g_edmClient.QueryDevice(busType, devices);
    if (retCode != UsbErrCode::EDM_OK) {
        if (retCode == UsbErrCode::EDM_ERR_NO_PERM) {
            ThrowErr(env, PERMISSION_DENIED, "queryDevice: no permission");
        } else {
            ThrowErr(env, SERVICE_EXCEPTION, "Query device service fail");
        }
        return nullptr;
    }

    napi_value resultArray;
    NAPI_CALL(env, napi_create_array(env, &resultArray));
    for (size_t index = 0; index < devices.size(); index++) {
        napi_value element = ConvertDeviceToJsDevice(env, devices[index]);
        NAPI_CALL(env, napi_set_element(env, resultArray, index, element));
    }
    EDM_LOGI(MODULE_DEV_MGR, "query device finish");

    return resultArray;
}

static bool ParseDeviceId(const napi_env& env, const napi_value& value, uint64_t* deviceId)
{
    napi_valuetype type;
    NAPI_CALL_BASE(env, napi_typeof(env, value, &type), false);
    if (type == napi_bigint) {
        bool lossless;
        NAPI_CALL_BASE(env, napi_get_value_bigint_uint64(env, value, deviceId, &lossless), false);
    } else if (type == napi_number) {
        int64_t temp;
        NAPI_CALL_BASE(env, napi_get_value_int64(env, value, &temp), false);
        *deviceId = static_cast<uint64_t>(temp);
    } else {
        return false;
    }
    return true;
}

static napi_value BindDevice(napi_env env, napi_callback_info info)
{
    size_t argc = PARAM_COUNT_3;
    napi_value argv[PARAM_COUNT_3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < PARAM_COUNT_2) {
        ThrowErr(env, PARAMETER_ERROR, "bindDevice parameter count not match");
        return nullptr;
    }

    uint64_t deviceId;
    if (!ParseDeviceId(env, argv[0], &deviceId)) {
        ThrowErr(env, PARAMETER_ERROR, "deviceid type error");
        return nullptr;
    }
    EDM_LOGI(MODULE_DEV_MGR, "Enter bindDevice:%{public}016" PRIX64, deviceId);

    if (!IsMatchType(env, argv[1], napi_function)) {
        ThrowErr(env, PARAMETER_ERROR, "onDisconnect param is error");
        return nullptr;
    }

    std::lock_guard<std::mutex> mapLock(mapMutex);
    UsbErrCode retCode = g_edmClient.BindDevice(deviceId, g_edmCallback);
    if (retCode != UsbErrCode::EDM_OK) {
        if (retCode == UsbErrCode::EDM_ERR_NO_PERM) {
            ThrowErr(env, PERMISSION_DENIED, "bindDevice: no permission");
        } else {
            ThrowErr(env, SERVICE_EXCEPTION, "bindDevice service failed");
        }
        return nullptr;
    }

    sptr<AsyncData> data = new (std::nothrow) AsyncData {};
    if (data == nullptr) {
        ThrowErr(env, PARAMETER_ERROR, "malloc callback data fail");
        return nullptr;
    }
    data->env = env;
    data->deviceId = deviceId;
    NAPI_CALL(env, napi_create_reference(env, argv[1], 1, &data->onDisconnect));
    napi_value promise = nullptr;
    if (argc > PARAM_COUNT_2 && IsMatchType(env, argv[PARAM_COUNT_2], napi_function)) {
        NAPI_CALL(env, napi_create_reference(env, argv[PARAM_COUNT_2], 1, &data->bindCallback));
    } else {
        NAPI_CALL(env, napi_create_promise(env, &data->bindDeferred, &promise));
    }
    g_callbackMap[data->deviceId] = data;

    return promise;
}

static napi_value UnbindDevice(napi_env env, napi_callback_info info)
{
    size_t argc = PARAM_COUNT_2;
    napi_value argv[PARAM_COUNT_2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < PARAM_COUNT_1) {
        ThrowErr(env, PARAMETER_ERROR, "Param count error");
        return nullptr;
    }

    uint64_t deviceId;
    if (!ParseDeviceId(env, argv[0], &deviceId)) {
        ThrowErr(env, PARAMETER_ERROR, "deviceid type error");
        return nullptr;
    }
    EDM_LOGI(MODULE_DEV_MGR, "Enter unbindDevice:%{public}016" PRIX64, deviceId);

    UsbErrCode retCode = g_edmClient.UnBindDevice(deviceId);
    if (retCode != UsbErrCode::EDM_OK) {
        if (retCode == UsbErrCode::EDM_ERR_NO_PERM) {
            ThrowErr(env, PERMISSION_DENIED, "unbindDevice: no permission");
        } else {
            ThrowErr(env, SERVICE_EXCEPTION, "unbindDevice service failed");
        }
        return nullptr;
    }

    std::lock_guard<std::mutex> mapLock(mapMutex);
    if (g_callbackMap.find(deviceId) == g_callbackMap.end()) {
        ThrowErr(env, SERVICE_EXCEPTION, "Unbind callback does not exist");
        return nullptr;
    }
    auto data = g_callbackMap[deviceId];
    napi_value promise = nullptr;
    if (argc > PARAM_COUNT_1 && IsMatchType(env, argv[1], napi_function)) {
        NAPI_CALL(env, napi_create_reference(env, argv[1], 1, &data->unbindCallback));
    } else {
        NAPI_CALL(env, napi_create_promise(env, &data->unbindDeferred, &promise));
    }

    return promise;
}

static napi_value QueryDeviceInfo(napi_env env, napi_callback_info info)
{
    size_t argc = PARAM_COUNT_1;
    napi_value argv[PARAM_COUNT_1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    uint64_t deviceId = 0;
    if (argc > PARAM_COUNT_0 && !ParseDeviceId(env, argv[0], &deviceId)) {
        ThrowErr(env, PARAMETER_ERROR, "deviceId type error");
        return nullptr;
    }
    std::vector<std::shared_ptr<DeviceInfoData>> deviceInfos;
    int32_t ret;
    if (argc > PARAM_COUNT_0) {
        ret = g_edmClient.QueryDeviceInfo(deviceId, deviceInfos);
    } else {
        ret = g_edmClient.QueryDeviceInfo(deviceInfos);
    }
    if (ret != UsbErrCode::EDM_OK) {
        if (ret == UsbErrCode::EDM_ERR_NOT_SYSTEM_APP) {
            ThrowErr(env, PERMISSION_NOT_SYSTEM_APP, "queryDeviceInfo: none system app");
        } else if (ret == UsbErrCode::EDM_ERR_NO_PERM) {
            ThrowErr(env, PERMISSION_DENIED, "queryDeviceInfo: no permission");
        } else {
            ThrowErr(env, SERVICE_EXCEPTION_NEW, "Query device info service fail");
        }
        return nullptr;
    }

    napi_value resultArray;
    NAPI_CALL(env, napi_create_array(env, &resultArray));
    for (size_t i = 0; i < deviceInfos.size(); i++) {
        napi_value element = ConvertToJsDeviceInfo(env, deviceInfos[i]);
        NAPI_CALL(env, napi_set_element(env, resultArray, i, element));
    }

    return resultArray;
}

static napi_value QueryDriverInfo(napi_env env, napi_callback_info info)
{
    size_t argc = PARAM_COUNT_1;
    napi_value argv[PARAM_COUNT_1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc > PARAM_COUNT_0 && !IsMatchType(env, argv[0], napi_string)) {
        ThrowErr(env, PARAMETER_ERROR, "driverUid type is invalid");
        return nullptr;
    }

    std::vector<std::shared_ptr<DriverInfoData>> driverInfos;
    int32_t ret;
    if (argc > PARAM_COUNT_0) {
        size_t len;
        NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len));
        std::string driverUid;
        driverUid.resize(len);
        NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], &driverUid[0], len + 1, &len));
        ret = g_edmClient.QueryDriverInfo(driverUid, driverInfos);
    } else {
        ret = g_edmClient.QueryDriverInfo(driverInfos);
    }

    if (ret != UsbErrCode::EDM_OK) {
        if (ret == UsbErrCode::EDM_ERR_NOT_SYSTEM_APP) {
            ThrowErr(env, PERMISSION_NOT_SYSTEM_APP, "queryDriverInfo: none system app");
        } else if (ret == UsbErrCode::EDM_ERR_NO_PERM) {
            ThrowErr(env, PERMISSION_DENIED, "queryDriverInfo: no permission");
        } else {
            ThrowErr(env, SERVICE_EXCEPTION_NEW, "Query driver info service fail");
        }
        return nullptr;
    }

    napi_value resultArray;
    NAPI_CALL(env, napi_create_array(env, &resultArray));
    for (size_t index = 0; index < driverInfos.size(); index++) {
        napi_value element = ConvertToJsDriverInfo(env, driverInfos[index]);
        NAPI_CALL(env, napi_set_element(env, resultArray, index, element));
    }

    return resultArray;
}

static napi_value EnumBusTypeConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    void* data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data));
    return thisArg;
}

static void CreateEnumBusType(napi_env env, napi_value exports)
{
    napi_value usb = nullptr;
    napi_create_int32(env, 1, &usb);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("USB", usb),
    };

    napi_value result = nullptr;
    napi_define_class(env, "BusType", NAPI_AUTO_LENGTH, EnumBusTypeConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "BusType", result);
}

EXTERN_C_START
/*
 * function for module exports
 */
static napi_value ExtDeviceManagerInit(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("queryDevices", QueryDevices),
        DECLARE_NAPI_FUNCTION("bindDevice", BindDevice),
        DECLARE_NAPI_FUNCTION("bindDeviceDriver", BindDevice),
        DECLARE_NAPI_FUNCTION("unbindDevice", UnbindDevice),
        DECLARE_NAPI_FUNCTION("queryDeviceInfo", QueryDeviceInfo),
        DECLARE_NAPI_FUNCTION("queryDriverInfo", QueryDriverInfo),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    CreateEnumBusType(env, exports);

    return exports;
}
EXTERN_C_END

/*
 * Module definition
 */
static napi_module g_moduleManager = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ExtDeviceManagerInit,
    .nm_modname = "driver.deviceManager",
    .nm_priv = nullptr,
    .reserved = {nullptr}
};

/*
 * Module registration
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_moduleManager);
}
}
}