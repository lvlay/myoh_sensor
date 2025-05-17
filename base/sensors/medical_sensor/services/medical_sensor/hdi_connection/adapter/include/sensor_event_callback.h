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

#ifndef SENSOR_EVENT_CALLBACK_H
#define SENSOR_EVENT_CALLBACK_H

#include "v1_0/isensor_callback.h"

using OHOS::HDI::Sensor::V1_0::HdfSensorEvents;
using OHOS::HDI::Sensor::V1_0::ISensorCallback;

namespace OHOS {
namespace Sensors {
class SensorEventCallback : public ISensorCallback {
public:

    virtual ~SensorEventCallback() {}

    int32_t OnDataEvent(const HdfSensorEvents& event) override;
};
}  // namespace Sensors
}  // namespace OHOS
#endif // SENSOR_EVENT_CALLBACK_H

