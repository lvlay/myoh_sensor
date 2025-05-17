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

#include "vibrator_convert_js.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "audio_parsing.h"
#include "sensor_napi_error.h"
#include "vibrator_convert_napi_utils.h"

#undef LOG_TAG
#define LOG_TAG "VibratorConvert"

namespace OHOS {
namespace Sensors {
namespace {
const char* CONVERT = "convert";
const char* CONVERT_CLASS = "convert_class";
}  // namespace

std::shared_ptr<AudioParsing> VibratorConvert::GetInterfaces()
{
    CALL_LOG_ENTER;
    return audioParsing_;
}

napi_value VibratorConvert::CreateInstance(napi_env env)
{
    CALL_LOG_ENTER;
    napi_value global = nullptr;
    CHKCP((napi_get_global(env, &global) == napi_ok), "napi_get_global failed");
    constexpr char className[] = "VibratorConvert";
    napi_value jsClass = nullptr;
    napi_property_descriptor desc[] = {};
    napi_status status = napi_define_class(env, className, sizeof(className),
        VibratorConvert::ConvertAudioJsConstructor, nullptr, sizeof(desc) / sizeof(desc[0]), nullptr, &jsClass);
    CHKCP((status == napi_ok), "napi_define_class failed");
    CHKCP((napi_set_named_property(env, global, CONVERT_CLASS, jsClass) == napi_ok), "napi_set_named_property failed");
    napi_value jsInstance = nullptr;
    CHKCP((napi_new_instance(env, jsClass, 0, nullptr, &jsInstance) == napi_ok), "napi_new_instance failed");
    CHKCP((napi_set_named_property(env, global, CONVERT, jsInstance) == napi_ok),
        "napi_set_named_property convert failed");

    VibratorConvert *vConvert = nullptr;
    CHKCP((napi_unwrap(env, jsInstance, (void**)&vConvert) == napi_ok), "napi_unwrap failed");
    CHKPP(vConvert);
    CHKCP((napi_create_reference(env, jsInstance, 1, &(vConvert->contextRef_)) == napi_ok),
        "napi_create_reference failed");
    uint32_t refCount = 0;
    status = napi_reference_ref(env, vConvert->contextRef_, &refCount);
    if (status != napi_ok) {
        SEN_HILOGE("napi_reference_ref failed");
        napi_delete_reference(env, vConvert->contextRef_);
        return nullptr;
    }
    return jsInstance;
}

napi_value VibratorConvert::ConvertAudioJsConstructor(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    CHKCP((napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, &data) == napi_ok), "napi_get_cb_info failed");

    VibratorConvert *vConvert = new (std::nothrow) VibratorConvert();
    CHKPP(vConvert);
    napi_status status = napi_wrap(env, thisVar, vConvert, [](napi_env env, void *data, void *hin) {
        SEN_HILOGI("jsvm ends");
        VibratorConvert *context = static_cast<VibratorConvert*>(data);
        delete context;
    }, nullptr, nullptr);
    if (status != napi_ok) {
        delete vConvert;
        SEN_HILOGE("napi_wrap failed");
        napi_throw_error(env, nullptr, "napi_wrap failed");
        return nullptr;
    }
    return thisVar;
}

VibratorConvert *VibratorConvert::GetInstance(napi_env env)
{
    CALL_LOG_ENTER;
    napi_value global = nullptr;
    CHKCP((napi_get_global(env, &global) == napi_ok), "napi_get_global failed");

    bool result = false;
    CHKCP((napi_has_named_property(env, global, CONVERT, &result) == napi_ok), "napi_has_named_property failed");
    if (!result) {
        SEN_HILOGE("convert was not found");
        return nullptr;
    }
    napi_value object = nullptr;
    CHKCP((napi_get_named_property(env, global, CONVERT, &object) == napi_ok), "napi_get_named_property failed");
    if (object == nullptr) {
        SEN_HILOGE("object is nullptr");
        return nullptr;
    }
    VibratorConvert *instance = nullptr;
    CHKCP((napi_unwrap(env, object, (void**)&instance) == napi_ok), "napi_unwrap failed");
    if (instance == nullptr) {
        SEN_HILOGE("instance is nullptr");
        return nullptr;
    }
    return instance;
} 

bool VibratorConvert::ParseParameter(napi_env env, napi_value &value, RawFileDescriptor &fileDescriptor)
{
    CALL_LOG_ENTER;
    CHKCF(GetPropertyInt32(env, value, "fd", fileDescriptor.fd), "Get vibrate fd failed");
    GetPropertyInt64(env, value, "offset", fileDescriptor.offset);
    fileDescriptor.length = GetFileSize(fileDescriptor.fd) - fileDescriptor.offset;
    GetPropertyInt64(env, value, "length", fileDescriptor.length);
    return true;
}

bool VibratorConvert::ParseAudioSettings(napi_env env, napi_value &value, AudioSetting &audioSetting)
{
    CALL_LOG_ENTER;
    CHKCF(GetPropertyInt32(env, value, "transientDetection", audioSetting.transientDetection),
        "Get audio transientDetection failed");
    CHKCF(GetPropertyInt32(env, value, "intensityTreshold", audioSetting.intensityTreshold),
        "Get audio intensityTreshold failed");
    CHKCF(GetPropertyInt32(env, value, "frequencyTreshold", audioSetting.frequencyTreshold),
        "Get audio frequencyTreshold failed");
    CHKCF(GetPropertyInt32(env, value, "frequencyMaxValue", audioSetting.frequencyMaxValue),
        "Get audio frequencyMaxValue failed");
    CHKCF(GetPropertyInt32(env, value, "frequencyMinValue", audioSetting.frequencyMinValue),
        "Get audio frequencyMinValue failed");
    return true;
}

napi_value VibratorConvert::ConvertAudioToHapticConstructor(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    size_t argc = 1;
    napi_value args[1] = {};
    napi_value thisArg = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &thisArg, nullptr);
    if ((status != napi_ok) || (argc < 1)) {
        ThrowErr(env, PARAMETER_ERROR, "napi_get_cb_info failed");
        return nullptr;
    }
    if (!IsMatchNapiType(env, args[0], napi_object)) {
        ThrowErr(env, PARAMETER_ERROR, "Wrong argument type, should be object");
        return nullptr;
    }
    RawFileDescriptor fileDescriptor;
    if (!ParseParameter(env, args[0], fileDescriptor)) {
        ThrowErr(env, PARAMETER_ERROR, "Parse parameter failed");
        return nullptr;
    }
    VibratorConvert *vConvert = VibratorConvert::GetInstance(env);
    CHKPP(vConvert);
    vConvert->audioParsing_ = std::make_shared<AudioParsing>(fileDescriptor);
    if (vConvert->audioParsing_->ParseAudioFile() != Sensors::SUCCESS) {
        ThrowErr(env, PARAMETER_ERROR, "ParseAudioFile failed or fileDescriptor invalid");
        return nullptr;
    }
    return thisArg;
}

napi_value VibratorConvert::GetAudioAttribute(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    size_t argc = 1;
    napi_value args[1] = {};
    napi_value thisArg = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &thisArg, nullptr);
    if (status != napi_ok) {
        ThrowErr(env, PARAMETER_ERROR, "napi_get_cb_info failed");
        return nullptr;
    }
    sptr<AsyncCallbackInfo> asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    CHKPP(asyncCallbackInfo);
    VibratorConvert *vConvert = VibratorConvert::GetInstance(env);
    CHKPP(vConvert);
    std::shared_ptr<AudioParsing> audioParsing = vConvert->GetInterfaces();
    CHKPP(audioParsing);
    asyncCallbackInfo->error.code = audioParsing->GetAudioAttribute(asyncCallbackInfo->audioAttribute);

    asyncCallbackInfo->callbackType = AUDIO_ATTRIBUTE_CALLBACK;
    if ((argc > 0) && (IsMatchNapiType(env, args[0], napi_function))) {
        CHKCP((napi_create_reference(env, args[0], 1, &asyncCallbackInfo->callback[0]) == napi_ok),
            "napi_create_reference failed");
        EmitHapticAsyncCallbackWork(asyncCallbackInfo);
        return nullptr;
    }
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    CHKCP((napi_create_promise(env, &deferred, &promise) == napi_ok), "napi_create_promise failed");
    asyncCallbackInfo->deferred = deferred;
    EmitHapticPromiseWork(asyncCallbackInfo);
    return promise;
}

napi_value VibratorConvert::GetAudioData(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    size_t argc = 2;
    napi_value args[2] = {};
    napi_value thisArg = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &thisArg, nullptr);
    if ((status != napi_ok) || (argc < 1)) {
        ThrowErr(env, PARAMETER_ERROR, "napi_get_cb_info failed");
        return nullptr;
    }
    if (!IsMatchNapiType(env, args[0], napi_number)) {
        ThrowErr(env, PARAMETER_ERROR, "Wrong argument type, should be number");
        return nullptr;
    }
    int32_t samplingInterval = 0;
    if (!GetInt32Value(env, args[0], samplingInterval)) {
        ThrowErr(env, PARAMETER_ERROR, "Wrong argument type, get number failed");
        return nullptr;
    }
    sptr<AsyncCallbackInfo> asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    CHKPP(asyncCallbackInfo);
    VibratorConvert *vConvert = VibratorConvert::GetInstance(env);
    CHKPP(vConvert);
    std::shared_ptr<AudioParsing> audioParsing = vConvert->GetInterfaces();
    CHKPP(audioParsing);
    asyncCallbackInfo->error.code = audioParsing->GetAudioData(samplingInterval, asyncCallbackInfo->audioData);

    asyncCallbackInfo->callbackType = AUDIO_DATA_CALLBACK;
    if ((argc > 1) && (IsMatchNapiType(env, args[1], napi_function))) {
        CHKCP((napi_create_reference(env, args[1], 1, &asyncCallbackInfo->callback[0]) == napi_ok),
            "napi_create_reference failed");
        EmitHapticAsyncCallbackWork(asyncCallbackInfo);
        return nullptr;
    } 
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    CHKCP((napi_create_promise(env, &deferred, &promise) == napi_ok), "napi_create_promise failed");
    asyncCallbackInfo->deferred = deferred;
    EmitHapticPromiseWork(asyncCallbackInfo);
    return promise;
}

napi_value VibratorConvert::ConvertAudioToHaptic(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    size_t argc = 2;
    napi_value args[2] = {};
    napi_value thisArg = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &thisArg, nullptr);
    if ((status != napi_ok) || (argc < 1)) {
        ThrowErr(env, PARAMETER_ERROR, "napi_get_cb_info failed");
        return nullptr;
    }
    if (!IsMatchNapiType(env, args[0], napi_object)) {
        ThrowErr(env, PARAMETER_ERROR, "Wrong argument type, should be object");
        return nullptr;
    }
    AudioSetting audioSetting;
    if (!ParseAudioSettings(env, args[0], audioSetting)) {
        ThrowErr(env, PARAMETER_ERROR, "parameter failed");
        return nullptr;
    }
    sptr<AsyncCallbackInfo> asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo(env);
    CHKPP(asyncCallbackInfo);
    VibratorConvert *vConvert = VibratorConvert::GetInstance(env);
    CHKPP(vConvert);
    std::shared_ptr<AudioParsing> audioParsing = vConvert->GetInterfaces();
    CHKPP(audioParsing);
    asyncCallbackInfo->error.code = audioParsing->ConvertAudioToHaptic(audioSetting, asyncCallbackInfo->hapticEvents);

    asyncCallbackInfo->callbackType = AUDIO_TO_HAPTIC_CALLBACK;
    if ((argc > 1) && (IsMatchNapiType(env, args[1], napi_function))) {
        CHKCP((napi_create_reference(env, args[1], 1, &asyncCallbackInfo->callback[0]) == napi_ok),
            "napi_create_reference failed");
        EmitHapticAsyncCallbackWork(asyncCallbackInfo);
        return nullptr;
    }
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    CHKCP((napi_create_promise(env, &deferred, &promise) == napi_ok), "napi_create_promise failed");
    asyncCallbackInfo->deferred = deferred;
    EmitHapticPromiseWork(asyncCallbackInfo);
    return promise;
}
}  // namespace Sensors
}  // namespace OHOS
