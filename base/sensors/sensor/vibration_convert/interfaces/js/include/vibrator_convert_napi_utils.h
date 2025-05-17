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

#ifndef VIBRATOR_CONVERT_NAPI_UTILS_H
#define VIBRATOR_CONVERT_NAPI_UTILS_H

#include <cstring>
#include <iostream>
#include <map>
#include <optional>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "refbase.h"

#include "sensors_errors.h"
#include "vibration_convert_type.h"

namespace OHOS {
namespace Sensors {
constexpr int32_t CALLBACK_NUM = 1;

enum CallbackType {
    AUDIO_ATTRIBUTE_CALLBACK = 1,
    AUDIO_DATA_CALLBACK,
    AUDIO_TO_HAPTIC_CALLBACK,
};

class AsyncCallbackInfo : public RefBase {
public:
    struct AsyncCallbackError {
        int32_t code { 0 };
        std::string message;
    };

    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback[CALLBACK_NUM] = { 0 };
    AudioAttribute audioAttribute;
    AudioData audioData;
    AsyncCallbackError error;
    CallbackType callbackType { AUDIO_ATTRIBUTE_CALLBACK };
    int32_t eventCount { 0 };
    std::vector<HapticEvent> hapticEvents;
    explicit AsyncCallbackInfo(napi_env env) : env(env) {}
    ~AsyncCallbackInfo();
};

using ConstructResultFunc = bool(*)(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo,
    napi_value result[], int32_t length);

bool IsMatchNapiType(const napi_env &env, const napi_value &value, const napi_valuetype &type);
bool GetInt32Value(const napi_env &env, const napi_value &value, int32_t &result);
bool GetInt64Value(const napi_env &env, const napi_value &value, int64_t &result);
bool GetPropertyInt32(const napi_env &env, const napi_value &value, const std::string &type, int32_t &result);
bool GetPropertyInt64(const napi_env &env, const napi_value &value, const std::string &type, int64_t &result);
bool ConvertErrorToResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo, napi_value &result);
bool GetAudioAttributeResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo,
    napi_value result[], int32_t length);
bool GetAudioDataResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo,
    napi_value result[], int32_t length);
bool ConvertAudioToHapticResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo,
    napi_value result[], int32_t length);
bool CreateArray(const napi_env &env, float *data, int32_t dataLength, napi_value &result);
napi_value GetResultInfo(sptr<AsyncCallbackInfo> asyncCallbackInfo);
napi_value GetAudioDataInfo(sptr<AsyncCallbackInfo> asyncCallbackInfo);
napi_value GetAudioToHapticInfo(sptr<AsyncCallbackInfo> asyncCallbackInfo);
void EmitHapticAsyncCallbackWork(sptr<AsyncCallbackInfo> async_callback_info);
void EmitHapticPromiseWork(sptr<AsyncCallbackInfo> asyncCallbackInfo);
int64_t GetFileSize(int32_t fd);
}  // namespace Sensors
}  // namespace OHOS
#endif // VIBRATOR_CONVERT_NAPI_UTILS_H