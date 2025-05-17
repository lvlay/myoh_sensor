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

#include "js_driver_extension.h"

#include "ability_info.h"
#include "hitrace_meter.h"
#include "hilog_wrapper.h"
#include "js_extension_common.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_driver_extension_context.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
}

namespace {
napi_value PromiseCallback(napi_env env, napi_callback_info info)
{
    if (info == nullptr) {
        HILOG_ERROR("PromiseCallback, Invalid input info.");
        return nullptr;
    }
    void *data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data);
    if (data == nullptr) {
        HILOG_ERROR("PromiseCallback, Invalid input data info.");
        return nullptr;
    }

    auto *callbackInfo = static_cast<AppExecFwk::AbilityTransactionCallbackInfo<> *>(data);
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
    data = nullptr;

    return nullptr;
}

napi_value OnConnectPromiseCallback(napi_env env, napi_callback_info info)
{
    if (info == nullptr) {
        HILOG_ERROR("PromiseCallback, Invalid input info.");
        return nullptr;
    }
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, nullptr, &data);
    if (data == nullptr) {
        HILOG_ERROR("PromiseCallback, Invalid input data info.");
        return nullptr;
    }

    auto *callbackInfo = static_cast<AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *>(data);
    sptr<IRemoteObject> service = nullptr;
    if (argc > 0) {
        service = NAPI_ohos_rpc_getNativeRemoteObject(env, argv[0]);
    }
    callbackInfo->Call(service);
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>>::Destroy(callbackInfo);
    data = nullptr;

    return nullptr;
}
}

using namespace OHOS::AppExecFwk;

napi_value AttachDriverExtensionContext(napi_env env, void *value, void *)
{
    HILOG_INFO("AttachDriverExtensionContext");
    if (value == nullptr) {
        HILOG_WARN("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<DriverExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        HILOG_WARN("invalid context.");
        return nullptr;
    }
    napi_value object = CreateJsDriverExtensionContext(env, ptr);
    auto contextObjRef = JsRuntime::LoadSystemModuleByEngine(env,
        "application.DriverExtensionContext", &object, 1);
    napi_value contextObj = contextObjRef->GetNapiValue();

    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachDriverExtensionContext,
        value, nullptr);

    auto workContext = new (std::nothrow) std::weak_ptr<DriverExtensionContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env env, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr driver extension context is called");
            delete static_cast<std::weak_ptr<DriverExtensionContext> *>(data);
        }, nullptr, nullptr);

    return contextObj;
}

JsDriverExtension* JsDriverExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new JsDriverExtension(static_cast<JsRuntime&>(*runtime));
}

JsDriverExtension::JsDriverExtension(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsDriverExtension::~JsDriverExtension() = default;

void JsDriverExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    DriverExtension::Init(record, application, handler, token);
    std::string srcPath = "";
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        HILOG_ERROR("Failed to get srcPath");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    HILOG_DEBUG("JsStaticSubscriberExtension::Init moduleName:%{public}s,srcPath:%{public}s.",
        moduleName.c_str(), srcPath.c_str());
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        HILOG_ERROR("Failed to get jsObj_");
        return;
    }
    HILOG_INFO("JsDriverExtension::Init ConvertNativeValueTo.");

    napi_value obj = jsObj_->GetNapiValue();
    BindContext(env, obj);
    SetExtensionCommon(JsExtensionCommon::Create(jsRuntime_, static_cast<NativeReference&>(*jsObj_), shellContextRef_));
}

void JsDriverExtension::BindContext(napi_env env, napi_value obj)
{
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    HILOG_INFO("JsDriverExtension::Init CreateJsDriverExtensionContext.");
    napi_value contextObj = CreateJsDriverExtensionContext(env, context);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.DriverExtensionContext",
        &contextObj, ARGC_ONE);

    napi_value nativeObj = shellContextRef_->GetNapiValue();

    auto workContext = new (std::nothrow) std::weak_ptr<DriverExtensionContext>(context);
    napi_coerce_to_native_binding_object(env, nativeObj, DetachCallbackFunc, AttachDriverExtensionContext,
        workContext, nullptr);

    HILOG_INFO("JsDriverExtension::Init Bind.");
    context->Bind(jsRuntime_, shellContextRef_.get());
    HILOG_INFO("JsDriverExtension::SetProperty.");
    napi_set_named_property(env, obj, "context", contextObj);
    HILOG_INFO("Set driver extension context");
    napi_wrap(env, nativeObj, workContext,
        [](napi_env, void* data, void*) {
            HILOG_INFO("Finalizer for weak_ptr driver extension context is called");
            delete static_cast<std::weak_ptr<DriverExtensionContext>*>(data);
        }, nullptr, nullptr);
    HILOG_INFO("JsDriverExtension::Init end.");
}

void JsDriverExtension::OnStart(const AAFwk::Want &want)
{
    Extension::OnStart(want);
    HILOG_INFO("JsDriverExtension OnStart begin..");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    CallObjectMethod(env, "onInit", argv, ARGC_ONE);
    HILOG_INFO("%{public}s end.", __func__);
}

void JsDriverExtension::OnStop()
{
    DriverExtension::OnStop();
    HILOG_INFO("JsDriverExtension OnStop begin.");
    napi_env env = jsRuntime_.GetNapiEnv();
    CallObjectMethod(env, "onRelease");
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        HILOG_INFO("The driver extension connection is not disconnected.");
    }
    HILOG_INFO("%{public}s end.", __func__);
}

sptr<IRemoteObject> JsDriverExtension::OnConnect(const AAFwk::Want &want)
{
    HandleScope handleScope(jsRuntime_);
    napi_value result = CallOnConnect(want);
    napi_env env = jsRuntime_.GetNapiEnv();
    auto remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(env, result);
    if (remoteObj == nullptr) {
        HILOG_ERROR("remoteObj nullptr.");
    }
    return remoteObj;
}

sptr<IRemoteObject> JsDriverExtension::OnConnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    HandleScope handleScope(jsRuntime_);
    napi_value result = CallOnConnect(want);
    napi_env env = jsRuntime_.GetNapiEnv();
    bool isPromise = CheckPromise(result);
    if (!isPromise) {
        isAsyncCallback = false;
        sptr<IRemoteObject> remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(env, result);
        if (remoteObj == nullptr) {
            HILOG_ERROR("remoteObj nullptr.");
        }
        return remoteObj;
    }

    bool callResult = false;
    do {
        napi_value then = nullptr;
        napi_get_named_property(env, result, "then", &then);
        bool isCallable = false;
        napi_is_callable(env, then, &isCallable);
        if (!isCallable) {
            HILOG_ERROR("CallPromise, property then is not callable.");
            break;
        }
        napi_value promiseCallback = nullptr;
        napi_create_function(env, "promiseCallback", strlen("promiseCallback"),
            OnConnectPromiseCallback, callbackInfo, &promiseCallback);
        napi_value argv[1] = { promiseCallback };
        napi_call_function(env, result, then, 1, argv, nullptr);
        callResult = true;
    } while (false);

    if (!callResult) {
        HILOG_ERROR("Failed to call promise.");
        isAsyncCallback = false;
    } else {
        isAsyncCallback = true;
    }
    return nullptr;
}

void JsDriverExtension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnDisconnect(want);
    HILOG_DEBUG("%{public}s begin.", __func__);
    CallOnDisconnect(want, false);
    HILOG_DEBUG("%{public}s end.", __func__);
}

void JsDriverExtension::OnDisconnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnDisconnect(want);
    HILOG_DEBUG("%{public}s begin.", __func__);
    napi_value result = CallOnDisconnect(want, true);
    bool isPromise = CheckPromise(result);
    if (!isPromise) {
        isAsyncCallback = false;
        return;
    }
    bool callResult = CallPromise(result, callbackInfo);
    if (!callResult) {
        HILOG_ERROR("Failed to call promise.");
        isAsyncCallback = false;
    } else {
        isAsyncCallback = true;
    }

    HILOG_DEBUG("%{public}s end.", __func__);
}

napi_value JsDriverExtension::CallObjectMethod(napi_env env, const char* name, const napi_value* argv, size_t argc)
{
    HILOG_INFO("JsDriverExtension::CallObjectMethod(%{public}s), begin", name);

    if (!jsObj_) {
        HILOG_WARN("Not found DriverExtension.js");
        return nullptr;
    }
    napi_value obj = jsObj_->GetNapiValue();

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    napi_valuetype type = napi_undefined;
    napi_typeof(env, method, &type);
    if (type != napi_function) {
        HILOG_ERROR("Failed to get '%{public}s' from DriverExtension object", name);
        return nullptr;
    }
    HILOG_INFO("JsDriverExtension::CallFunction(%{public}s), success", name);
    napi_value result = nullptr;
    napi_call_function(env, obj, method, argc, argv, &result);
    return result;
}

void JsDriverExtension::GetSrcPath(std::string &srcPath)
{
    if (!Extension::abilityInfo_->isModuleJson) {
        /* temporary compatibility api8 + config.json */
        srcPath.append(Extension::abilityInfo_->package);
        srcPath.append("/assets/js/");
        if (!Extension::abilityInfo_->srcPath.empty()) {
            srcPath.append(Extension::abilityInfo_->srcPath);
        }
        srcPath.append("/").append(Extension::abilityInfo_->name).append(".abc");
        return;
    }

    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        srcPath.erase(srcPath.rfind('.'));
        srcPath.append(".abc");
    }
}

napi_value JsDriverExtension::CallOnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnConnect(want);
    HILOG_DEBUG("%{public}s begin.", __func__);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    return CallObjectMethod(env, "onConnect", argv, ARGC_ONE);
}

napi_value JsDriverExtension::CallOnDisconnect(const AAFwk::Want &want, bool withResult)
{
    HandleEscape handleEscape(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    napi_value result = CallObjectMethod(env, "onDisconnect", argv, ARGC_ONE);
    if (withResult) {
        return handleEscape.Escape(result);
    } else {
        return nullptr;
    }
}

bool JsDriverExtension::CheckPromise(napi_value result)
{
    if (result == nullptr) {
        HILOG_DEBUG("CheckPromise, result is null, no need to call promise.");
        return false;
    }
    bool isPromise = false;
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_is_promise(env, result, &isPromise);
    if (!isPromise) {
        HILOG_DEBUG("CheckPromise, result is not promise, no need to call promise.");
        return false;
    }
    return true;
}

bool JsDriverExtension::CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo)
{
    napi_value then = nullptr;
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_get_named_property(env, result, "then", &then);

    bool isCallable = false;
    napi_is_callable(env, then, &isCallable);
    if (!isCallable) {
        HILOG_ERROR("CallPromise, property then is not callable.");
        return false;
    }
    HandleScope handleScope(jsRuntime_);
    napi_value promiseCallback = nullptr;
    napi_create_function(env, "promiseCallback", strlen("promiseCallback"), PromiseCallback,
        callbackInfo, &promiseCallback);
    napi_value argv[1] = { promiseCallback };
    napi_call_function(env, result, then, 1, argv, nullptr);
    return true;
}

void JsDriverExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    Extension::Dump(params, info);
    HILOG_INFO("%{public}s called.", __func__);
    HandleScope handleScope(jsRuntime_);
    napi_env env  = jsRuntime_.GetNapiEnv();
    // create js array object of params
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, params.size(), &arrayValue);
    uint32_t index = 0;
    for (const auto &param : params) {
        napi_set_element(env, arrayValue, index++, CreateJsValue(env, param));
    }

    napi_value argv[] = { arrayValue };
    napi_value dumpInfo = CallObjectMethod(env, "onDump", argv, ARGC_ONE);
    bool isArray = false;
    napi_is_array(env, dumpInfo, &isArray);
    if (isArray) {
        HILOG_ERROR("dumpInfo is not array.");
        return;
    }
    uint32_t arrayLen = 0;
    napi_get_array_length(env, dumpInfo, &arrayLen);
    if (arrayLen <= 0) {
        HILOG_ERROR("dumpInfo array length is error.");
        return;
    }
    for (uint32_t i = 0; i < arrayLen; i++) {
        napi_value element;
        std::string dumpInfoStr;
        napi_get_element(env, dumpInfo, i, &element);
        if (!ConvertFromJsValue(env, element, dumpInfoStr)) {
            HILOG_ERROR("Parse dumpInfoStr failed");
            return;
        }
        info.push_back(dumpInfoStr);
    }
    HILOG_DEBUG("Dump info size: %{public}zu", info.size());
}
}
}
