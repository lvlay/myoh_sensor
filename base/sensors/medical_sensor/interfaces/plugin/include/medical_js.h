/*
 * Copyright (c) 2022 Chipsea Technologies (Shenzhen) Corp., Ltd.
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
#ifndef MEDICAL_SENSOR_JS_H
#define MEDICAL_SENSOR_JS_H

#include "medical_native_impl.h"

namespace OHOS {
namespace Sensors {
static int32_t UnsubscribeSensor(int32_t sensorTypeId);
static void DataCallbackImpl(SensorEvent *event);
static int32_t SubscribeSensor(int32_t sensorTypeId, int64_t interval, const MedicalSensorUser *user);
} // namespace Sensors
} // namespace OHOS
#endif // MEDICAL_SENSOR_JS_H