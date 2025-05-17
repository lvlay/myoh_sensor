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
#ifndef SENSOR_JS_H
#define SENSOR_JS_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "async_callback_info.h"
#include "sensor_agent.h"

namespace OHOS {
namespace Sensors {
int32_t UnsubscribeSensor(int32_t sensorTypeId);
void DataCallbackImpl(SensorEvent *event);
int32_t SubscribeSensor(int32_t sensorTypeId, int64_t interval, RecordSensorCallback callback);
napi_value Subscribe(napi_env env, napi_callback_info info, int32_t sensorTypeId, CallbackDataType type);
napi_value Unsubscribe(napi_env env, napi_callback_info info, int32_t sensorTypeId);
napi_value GetBodyState(napi_env env, napi_callback_info info);
}  // namespace Sensors
}  // namespace OHOS
#endif // SENSOR_JS_H