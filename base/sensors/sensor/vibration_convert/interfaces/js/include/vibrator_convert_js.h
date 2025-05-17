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

#ifndef VIBRATOR_CONVERT_JS_H
#define VIBRATOR_CONVERT_JS_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "audio_parsing.h"

namespace OHOS {
namespace Sensors {
class VibratorConvert {
public:
    VibratorConvert() = default;
    ~VibratorConvert() = default;
    static napi_value GetAudioAttribute(napi_env env, napi_callback_info info);
    static napi_value GetAudioData(napi_env env, napi_callback_info info);
    static napi_value ConvertAudioToHaptic(napi_env env, napi_callback_info info);
    static napi_value ConvertAudioToHapticConstructor(napi_env env, napi_callback_info info);
    static napi_value CreateInstance(napi_env env);

private:
    std::shared_ptr<AudioParsing> GetInterfaces();
    static napi_value ConvertAudioJsConstructor(napi_env env, napi_callback_info info);
    static VibratorConvert *GetInstance(napi_env env);
    static bool ParseParameter(napi_env env, napi_value &value, RawFileDescriptor &fileDescriptor);
    static bool ParseAudioSettings(napi_env env, napi_value &value, AudioSetting &audioSetting);

private:
    std::shared_ptr<AudioParsing> audioParsing_ { nullptr };
    std::mutex mutex_;
    napi_ref contextRef_ { nullptr };
};
}  // namespace Sensors
}  // namespace OHOS
#endif // VIBRATOR_CONVERT_JS_H