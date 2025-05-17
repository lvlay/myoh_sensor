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

#include "vibrator_convert_napi_utils.h"

#include <string>

#include <sys/stat.h>

#include "securec.h"

#include "sensor_log.h"
#include "sensor_napi_error.h"
#include "vibration_convert_type.h"

#undef LOG_TAG
#define LOG_TAG "VibratorConvertNapiUtils"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t RESULT_LENGTH = 2;
constexpr int64_t INVALID_FILE_SIZE = -1;
}  // namespace

AsyncCallbackInfo::~AsyncCallbackInfo()
{
    CALL_LOG_ENTER;
    if (asyncWork != nullptr) {
        SEN_HILOGD("delete work");
        napi_delete_async_work(env, asyncWork);
    }
    if (callback[0] != nullptr) {
        SEN_HILOGD("delete reference");
        napi_delete_reference(env, callback[0]);
    }
}

int64_t GetFileSize(int32_t fd)
{
    CALL_LOG_ENTER;
    if (fd < 0) {
        SEN_HILOGE("fd is invalid, fd:%{public}d", fd);
        return INVALID_FILE_SIZE;
    }
    struct stat64 statbuf = { 0 };
    if (fstat64(fd, &statbuf) != 0) {
        SEN_HILOGE("fstat error, errno:%{public}d", errno);
        return INVALID_FILE_SIZE;
    }
    return static_cast<int64_t>(statbuf.st_size);
}

bool IsMatchNapiType(const napi_env &env, const napi_value &value, const napi_valuetype &type)
{
    CALL_LOG_ENTER;
    napi_valuetype paramType = napi_undefined;
    napi_status ret = napi_typeof(env, value, &paramType);
    if ((ret != napi_ok) || (paramType != type)) {
        SEN_HILOGE("Type mismatch");
        return false;
    }
    return true;
}

bool GetInt32Value(const napi_env &env, const napi_value &value, int32_t &result)
{
    CALL_LOG_ENTER;
    napi_valuetype valuetype = napi_undefined;
    CHKCF(napi_typeof(env, value, &valuetype) == napi_ok, "napi_typeof failed");
    CHKCF((valuetype == napi_number), "Wrong argument type. Number expected");
    CHKCF(napi_get_value_int32(env, value, &result) == napi_ok, "napi_get_value_int32 failed");
    return true;
}

bool GetInt64Value(const napi_env &env, const napi_value &value, int64_t &result)
{
    CALL_LOG_ENTER;
    napi_valuetype valuetype = napi_undefined;
    CHKCF(napi_typeof(env, value, &valuetype) == napi_ok, "napi_typeof failed");
    CHKCF((valuetype == napi_number), "Wrong argument type. Number expected");
    CHKCF(napi_get_value_int64(env, value, &result) == napi_ok, "napi_get_value_int64 failed");
    return true;
}

bool GetPropertyInt32(const napi_env &env, const napi_value &value, const std::string &type, int32_t &result)
{
    CALL_LOG_ENTER;
    bool exist = false;
    napi_status status = napi_has_named_property(env, value, type.c_str(), &exist);
    if (status != napi_ok || !exist) {
        SEN_HILOGE("can not find %{public}s property", type.c_str());
        return false;
    }
    napi_value item = nullptr;
    CHKCF((napi_get_named_property(env, value, type.c_str(), &item) == napi_ok), "napi_get_named_property failed");
    if (!GetInt32Value(env, item, result)) {
        SEN_HILOGE("GetInt32Value failed");
        return false;
    }
    return true;
}

bool GetPropertyInt64(const napi_env &env, const napi_value &value, const std::string &type, int64_t &result)
{
    CALL_LOG_ENTER;
    bool exist = false;
    napi_status status = napi_has_named_property(env, value, type.c_str(), &exist);
    if (status != napi_ok || !exist) {
        SEN_HILOGE("can not find %{public}s property", type.c_str());
        return false;
    }
    napi_value item = nullptr;
    CHKCF((napi_get_named_property(env, value, type.c_str(), &item) == napi_ok), "napi get property failed");
    if (!GetInt64Value(env, item, result)) {
        SEN_HILOGE("GetInt64Value failed");
        return false;
    }
    return true;
}

std::map<int32_t, ConstructResultFunc> g_convertFuncList = {
    { AUDIO_ATTRIBUTE_CALLBACK, GetAudioAttributeResult },
    { AUDIO_DATA_CALLBACK, GetAudioDataResult },
    { AUDIO_TO_HAPTIC_CALLBACK, ConvertAudioToHapticResult },
};

bool ConvertErrorToResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo, napi_value &result)
{
    CALL_LOG_ENTER;
    CHKPF(asyncCallbackInfo);
    int32_t code = asyncCallbackInfo->error.code;
    std::optional<std::string> msg = GetNapiError(code);
    if (!msg) {
        SEN_HILOGE("errCode: %{public}d is invalid", code);
        return false;
    }
    return (CreateBusinessError(env, code, msg.value()) != nullptr);
}

napi_value GetResultInfo(sptr<AsyncCallbackInfo> asyncCallbackInfo)
{
    CALL_LOG_ENTER;
    CHKPP(asyncCallbackInfo);
    AudioAttribute audioAttribute = asyncCallbackInfo->audioAttribute;
    napi_env env = asyncCallbackInfo->env;
    napi_value object = nullptr;
    CHKCP((napi_create_object(env, &object) == napi_ok), "napi_create_object failed");
    napi_value sampleRate = nullptr;
    CHKCP((napi_create_int32(env, audioAttribute.sampleRate, &sampleRate) == napi_ok),
        "napi_create_int32 sampleRate failed");
    CHKCP((napi_set_named_property(env, object, "sampleRate", sampleRate) == napi_ok),
        "napi_set_named_property sampleRate failed");
    napi_value duration = nullptr;
    CHKCP((napi_create_int32(env, audioAttribute.duration, &duration) == napi_ok), "napi_create_int32 duration failed");
    CHKCP((napi_set_named_property(env, object, "duration", duration) == napi_ok),
        "napi_set_named_property duration failed");
    napi_value dataCount = nullptr;
    CHKCP((napi_create_int32(env, audioAttribute.dataCount, &dataCount) == napi_ok),
        "napi_create_int32 dataCount failed");
    CHKCP((napi_set_named_property(env, object, "dataCount", dataCount) == napi_ok),
        "napi_set_named_property dataCount failed");
    return object;
}

bool GetAudioAttributeResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo,
    napi_value result[], int32_t length)
{
    CALL_LOG_ENTER;
    CHKPF(asyncCallbackInfo);
    CHKCF(length == RESULT_LENGTH, "length check failed");
    if (asyncCallbackInfo->error.code != SUCCESS) {
        CHKCF(ConvertErrorToResult(env, asyncCallbackInfo, result[0]), "Create napi err failed in async work");
        CHKCF((napi_get_undefined(env, &result[1]) == napi_ok), "napi_get_undefined failed");
    } else {
        CHKCF((napi_get_undefined(env, &result[0]) == napi_ok), "napi_get_undefined result failed");
        result[1] = GetResultInfo(asyncCallbackInfo);
    }
    return true;
}

bool CreateArray(const napi_env &env, std::vector<double> audioData, napi_value &result)
{
    CALL_LOG_ENTER;
    CHKCF((napi_create_array(env, &result) == napi_ok), "napi_create_array failed");
    for (size_t i = 0; i < audioData.size(); ++i) {
        napi_value message = nullptr;
        CHKCF((napi_create_double(env, audioData[i], &message) == napi_ok), "napi_create_double failed");
        CHKCF((napi_set_element(env, result, i, message) == napi_ok), "napi_set_element failed");
    }
    return true;
}

napi_value GetAudioDataInfo(sptr<AsyncCallbackInfo> asyncCallbackInfo)
{
    CALL_LOG_ENTER;
    CHKPP(asyncCallbackInfo);
    AudioData datas = asyncCallbackInfo->audioData;
    napi_env env = asyncCallbackInfo->env;
    napi_value object = nullptr;
    CHKCP((napi_create_object(env, &object) == napi_ok), "napi_create_object failed");
    napi_value min = nullptr;
    CHKCP((napi_create_double(env, datas.min, &min) == napi_ok), "napi_create_double min failed");
    CHKCP((napi_set_named_property(env, object, "dataMin", min) == napi_ok), "napi_set_named_property dataMin failed");
    napi_value max = nullptr;
    CHKCP((napi_create_double(env, datas.max, &max) == napi_ok), "napi_create_double max failed");
    CHKCP((napi_set_named_property(env, object, "dataMax", max) == napi_ok), "napi_set_named_property dataMax failed");
    napi_value data = nullptr;
    if (!CreateArray(env, datas.audioDatas, data)) {
        SEN_HILOGE("CreateArray failed");
        return nullptr;
    }
    CHKCP((napi_set_named_property(env, object, "data", data) == napi_ok), "napi_set_named_property data failed");
    return object;
}

bool GetAudioDataResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo,
    napi_value result[], int32_t length)
{
    CALL_LOG_ENTER;
    CHKPF(asyncCallbackInfo);
    CHKCF(length == RESULT_LENGTH, "Array length is different");
    if (asyncCallbackInfo->error.code != SUCCESS) {
        CHKCF(ConvertErrorToResult(env, asyncCallbackInfo, result[0]), "Create napi err failed in async work");
        CHKCF((napi_get_undefined(env, &result[1]) == napi_ok), "napi_get_undefined failed");
    } else {
        CHKCF((napi_get_undefined(env, &result[0]) == napi_ok), "napi_get_undefined result failed");
        result[1] = GetAudioDataInfo(asyncCallbackInfo);
    }
    return true;
}

napi_value GetAudioToHapticInfo(sptr<AsyncCallbackInfo> asyncCallbackInfo)
{
    CALL_LOG_ENTER;
    CHKPP(asyncCallbackInfo);
    napi_env env = asyncCallbackInfo->env;
    napi_value result = nullptr;
    CHKCP((napi_create_array(env, &result) == napi_ok), "napi_create_array failed");
    int32_t index = 0;
    for (const auto &item : asyncCallbackInfo->hapticEvents) {
        napi_value object = nullptr;
        CHKCP((napi_create_object(env, &object) == napi_ok), "napi_create_object failed");
        napi_value vibrateTag = nullptr;
        CHKCP((napi_create_int32(env, item.vibrateTag, &vibrateTag) == napi_ok), "napi_create_int32 vibrateTag failed");
        CHKCP((napi_set_named_property(env, object, "vibrateTag", vibrateTag) == napi_ok),
            "napi_set_named_property vibrateTag failed");
        napi_value startTime = nullptr;
        CHKCP((napi_create_int32(env, item.startTime, &startTime) == napi_ok), "napi_create_int32 startTime failed");
        CHKCP((napi_set_named_property(env, object, "startTime", startTime) == napi_ok),
            "napi_set_named_property startTime failed");
        napi_value duration = nullptr;
        CHKCP((napi_create_int32(env, item.duration, &duration) == napi_ok), "napi_create_int32 duration failed");
        CHKCP((napi_set_named_property(env, object, "duration", duration) == napi_ok),
            "napi_set_named_property duration failed");
        napi_value intensity = nullptr;
        CHKCP((napi_create_int32(env, item.intensity, &intensity) == napi_ok), "napi_create_int32 intensity failed");
        CHKCP((napi_set_named_property(env, object, "intensity", intensity) == napi_ok),
            "napi_set_named_property intensity failed");
        napi_value frequency = nullptr;
        CHKCP((napi_create_int32(env, item.frequency, &frequency) == napi_ok), "napi_create_int32 frequency failed");
        CHKCP((napi_set_named_property(env, object, "frequency", frequency) == napi_ok),
            "napi_set_named_property frequency failed");
        CHKCP((napi_set_element(env, result, index, object) == napi_ok), "napi_set_element failed");
        ++index;
    }
    return result;
}

bool ConvertAudioToHapticResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo,
    napi_value result[], int32_t length)
{
    CALL_LOG_ENTER;
    CHKPF(asyncCallbackInfo);
    CHKCF(length == RESULT_LENGTH, "Array length is different");
    if (asyncCallbackInfo->error.code != SUCCESS) {
        CHKCF(ConvertErrorToResult(env, asyncCallbackInfo, result[0]), "Create napi err failed in async work");
        CHKCF((napi_get_undefined(env, &result[1]) == napi_ok), "napi_get_undefined failed");
    } else {
        CHKCF((napi_get_undefined(env, &result[0]) == napi_ok), "napi_get_undefined result failed");
        result[1] = GetAudioToHapticInfo(asyncCallbackInfo);
    }
    return true;
}

void EmitHapticAsyncCallbackWork(sptr<AsyncCallbackInfo> asyncCallbackInfo)
{
    CALL_LOG_ENTER;
    CHKPV(asyncCallbackInfo);
    CHKPV(asyncCallbackInfo->env);
    napi_env env = asyncCallbackInfo->env;
    napi_value resourceName = nullptr;
    napi_status ret = napi_create_string_latin1(env, "AsyncCallback", NAPI_AUTO_LENGTH, &resourceName);
    CHKCV((ret == napi_ok), "napi_create_string_latin1 failed");
    asyncCallbackInfo->IncStrongRef(nullptr);
    napi_status status = napi_create_async_work(
        env, nullptr, resourceName, [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            CALL_LOG_ENTER;
            sptr<AsyncCallbackInfo> asyncCallbackInfo(static_cast<AsyncCallbackInfo *>(data));
            /**
             * After the asynchronous task is created, the asyncCallbackInfo reference count is reduced
             * to 0 destruction, so you need to add 1 to the asyncCallbackInfo reference count when the
             * asynchronous task is created, and subtract 1 from the reference count after the naked
             * pointer is converted to a pointer when the asynchronous task is executed, the reference
             * count of the smart pointer is guaranteed to be 1.
             */
            asyncCallbackInfo->DecStrongRef(nullptr);
            CHKPV(asyncCallbackInfo->callback[0]);
            napi_value callback = nullptr;
            napi_status ret = napi_get_reference_value(env, asyncCallbackInfo->callback[0], &callback);
            CHKCV((ret == napi_ok), "napi_get_reference_value failed");
            napi_value result[RESULT_LENGTH] = { 0 };
            CHKCV((g_convertFuncList.find(asyncCallbackInfo->callbackType) != g_convertFuncList.end()),
                "Callback type invalid in async work");
            bool state = g_convertFuncList[asyncCallbackInfo->callbackType](env, asyncCallbackInfo,
                result, RESULT_LENGTH);
            CHKCV(state, "Create napi data failed in async work");
            napi_value callResult = nullptr;
            CHKCV((napi_call_function(env, nullptr, callback, 2, result, &callResult) == napi_ok),
                "napi_call_function failed");
        },
        asyncCallbackInfo.GetRefPtr(), &asyncCallbackInfo->asyncWork);
    if (status != napi_ok
        || napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork) != napi_ok) {
        SEN_HILOGE("Create async work failed");
        asyncCallbackInfo->DecStrongRef(nullptr);
    }
}

void EmitHapticPromiseWork(sptr<AsyncCallbackInfo> asyncCallbackInfo)
{
    CALL_LOG_ENTER;
    CHKPV(asyncCallbackInfo);
    CHKPV(asyncCallbackInfo->env);
    napi_value resourceName = nullptr;
    napi_env env = asyncCallbackInfo->env;
    napi_status ret = napi_create_string_latin1(env, "Promise", NAPI_AUTO_LENGTH, &resourceName);
    CHKCV((ret == napi_ok), "napi_create_string_latin1 failed");
    // Make the reference count of asyncCallbackInfo add 1, and the function exits the non-destructor
    asyncCallbackInfo->IncStrongRef(nullptr);
    napi_status status = napi_create_async_work(
        env, nullptr, resourceName, [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            CALL_LOG_ENTER;
            sptr<AsyncCallbackInfo> asyncCallbackInfo(static_cast<AsyncCallbackInfo *>(data));
            /**
             * After the asynchronous task is created, the asyncCallbackInfo reference count is reduced
             * to 0 destruction, so you need to add 1 to the asyncCallbackInfo reference count when the
             * asynchronous task is created, and subtract 1 from the reference count after the naked
             * pointer is converted to a pointer when the asynchronous task is executed, the reference
             * count of the smart pointer is guaranteed to be 1.
             */
            asyncCallbackInfo->DecStrongRef(nullptr);
            CHKPV(asyncCallbackInfo->deferred);
            napi_value result[RESULT_LENGTH] = { 0 };
            CHKCV((g_convertFuncList.find(asyncCallbackInfo->callbackType) != g_convertFuncList.end()),
                "Callback type invalid in promise");
            bool ret = g_convertFuncList[asyncCallbackInfo->callbackType](env, asyncCallbackInfo,
                result, RESULT_LENGTH);
            CHKCV(ret, "Callback type invalid in promise");
            if (asyncCallbackInfo->error.code != SUCCESS) {
                CHKCV((napi_reject_deferred(env, asyncCallbackInfo->deferred, result[0]) == napi_ok),
                    "napi_reject_deferred failed");
            } else {
                CHKCV((napi_resolve_deferred(env, asyncCallbackInfo->deferred, result[1]) == napi_ok),
                    "napi_resolve_deferred result failed");
            }
        }, asyncCallbackInfo.GetRefPtr(), &asyncCallbackInfo->asyncWork);
    if (status != napi_ok
        || napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork) != napi_ok) {
        SEN_HILOGE("Create async work failed");
        asyncCallbackInfo->DecStrongRef(nullptr);
    }
}
}  // namespace Sensors
}  // namespace OHOS
