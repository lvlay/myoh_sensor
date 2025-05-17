/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef SENSOR_SYSTEM_JS_H
#define SENSOR_SYSTEM_JS_H
#include "napi/native_api.h"
#include "napi/native_node_api.h"
namespace OHOS {
namespace Sensors {
napi_value SubscribeAccelerometer(napi_env env, napi_callback_info info);
napi_value UnsubscribeAccelerometer(napi_env env, napi_callback_info info);
napi_value SubscribeCompass(napi_env env, napi_callback_info info);
napi_value UnsubscribeCompass(napi_env env, napi_callback_info info);
napi_value SubscribeProximity(napi_env env, napi_callback_info info);
napi_value UnsubscribeProximity(napi_env env, napi_callback_info info);
napi_value SubscribeLight(napi_env env, napi_callback_info info);
napi_value UnsubscribeLight(napi_env env, napi_callback_info info);
napi_value SubscribeStepCounter(napi_env env, napi_callback_info info);
napi_value UnsubscribeStepCounter(napi_env env, napi_callback_info info);
napi_value SubscribeBarometer(napi_env env, napi_callback_info info);
napi_value UnsubscribeBarometer(napi_env env, napi_callback_info info);
napi_value SubscribeHeartRate(napi_env env, napi_callback_info info);
napi_value UnsubscribeHeartRate(napi_env env, napi_callback_info info);
napi_value SubscribeOnBodyState(napi_env env, napi_callback_info info);
napi_value UnsubscribeOnBodyState(napi_env env, napi_callback_info info);
napi_value GetOnBodyState(napi_env env, napi_callback_info info);
napi_value SubscribeDeviceOrientation(napi_env env, napi_callback_info info);
napi_value UnsubscribeDeviceOrientation(napi_env env, napi_callback_info info);
napi_value SubscribeGyroscope(napi_env env, napi_callback_info info);
napi_value UnsubscribeGyroscope(napi_env env, napi_callback_info info);
napi_value SubscribeGravity(napi_env env, napi_callback_info info);
napi_value UnsubscribeGravity(napi_env env, napi_callback_info info);
napi_value SubscribeMagnetic(napi_env env, napi_callback_info info);
napi_value UnsubscribeMagnetic(napi_env env, napi_callback_info info);
napi_value SubscribeHall(napi_env env, napi_callback_info info);
napi_value UnsubscribeHall(napi_env env, napi_callback_info info);
}  // namespace Sensors
}  // namespace OHOS
#endif // SENSOR_SYSTEM_JS_H