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
#include "sensor_system_js.h"

#include "async_callback_info.h"
#include "sensor_js.h"

namespace OHOS {
namespace Sensors {
napi_value SubscribeAccelerometer(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Subscribe(env, info, SENSOR_TYPE_ID_ACCELEROMETER, SUBSCRIBE_CALLBACK);
}

napi_value UnsubscribeAccelerometer(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Unsubscribe(env, info, SENSOR_TYPE_ID_ACCELEROMETER);
}

napi_value SubscribeCompass(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Subscribe(env, info, SENSOR_TYPE_ID_ORIENTATION, SUBSCRIBE_COMPASS);
}

napi_value UnsubscribeCompass(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Unsubscribe(env, info, SENSOR_TYPE_ID_ORIENTATION);
}

napi_value SubscribeProximity(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Subscribe(env, info, SENSOR_TYPE_ID_PROXIMITY, SUBSCRIBE_CALLBACK);
}

napi_value UnsubscribeProximity(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Unsubscribe(env, info, SENSOR_TYPE_ID_PROXIMITY);
}

napi_value SubscribeLight(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Subscribe(env, info, SENSOR_TYPE_ID_AMBIENT_LIGHT, SUBSCRIBE_CALLBACK);
}

napi_value UnsubscribeLight(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Unsubscribe(env, info, SENSOR_TYPE_ID_AMBIENT_LIGHT);
}

napi_value SubscribeStepCounter(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Subscribe(env, info, SENSOR_TYPE_ID_PEDOMETER, SUBSCRIBE_CALLBACK);
}

napi_value UnsubscribeStepCounter(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Unsubscribe(env, info, SENSOR_TYPE_ID_PEDOMETER);
}

napi_value SubscribeBarometer(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Subscribe(env, info, SENSOR_TYPE_ID_BAROMETER, SUBSCRIBE_CALLBACK);
}

napi_value UnsubscribeBarometer(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Unsubscribe(env, info, SENSOR_TYPE_ID_BAROMETER);
}

napi_value SubscribeHeartRate(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Subscribe(env, info, SENSOR_TYPE_ID_HEART_RATE, SUBSCRIBE_CALLBACK);
}

napi_value UnsubscribeHeartRate(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Unsubscribe(env, info, SENSOR_TYPE_ID_HEART_RATE);
}

napi_value SubscribeOnBodyState(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Subscribe(env, info, SENSOR_TYPE_ID_WEAR_DETECTION, SUBSCRIBE_CALLBACK);
}

napi_value UnsubscribeOnBodyState(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Unsubscribe(env, info, SENSOR_TYPE_ID_WEAR_DETECTION);
}

napi_value GetOnBodyState(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return GetBodyState(env, info);
}

napi_value SubscribeDeviceOrientation(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Subscribe(env, info, SENSOR_TYPE_ID_ORIENTATION, SUBSCRIBE_CALLBACK);
}

napi_value UnsubscribeDeviceOrientation(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Unsubscribe(env, info, SENSOR_TYPE_ID_ORIENTATION);
}

napi_value SubscribeGyroscope(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Subscribe(env, info, SENSOR_TYPE_ID_GYROSCOPE, SUBSCRIBE_CALLBACK);
}

napi_value UnsubscribeGyroscope(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Unsubscribe(env, info, SENSOR_TYPE_ID_GYROSCOPE);
}

napi_value SubscribeGravity(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Subscribe(env, info, SENSOR_TYPE_ID_GRAVITY, SUBSCRIBE_CALLBACK);
}

napi_value UnsubscribeGravity(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Unsubscribe(env, info, SENSOR_TYPE_ID_GRAVITY);
}

napi_value SubscribeMagnetic(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Subscribe(env, info, SENSOR_TYPE_ID_MAGNETIC_FIELD, SUBSCRIBE_CALLBACK);
}

napi_value UnsubscribeMagnetic(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Unsubscribe(env, info, SENSOR_TYPE_ID_MAGNETIC_FIELD);
}

napi_value SubscribeHall(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Subscribe(env, info, SENSOR_TYPE_ID_HALL, SUBSCRIBE_CALLBACK);
}

napi_value UnsubscribeHall(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    return Unsubscribe(env, info, SENSOR_TYPE_ID_HALL);
}
}  // namespace Sensors
}  // namespace OHOS