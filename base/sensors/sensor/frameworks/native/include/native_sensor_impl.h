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

#ifndef NATIVE_SENSOR_IMPL
#define NATIVE_SENSOR_IMPL

#include "oh_sensor.h"
#include "sensor_agent_type.h"

struct Sensor_Info {
    char sensorName[NAME_MAX_LEN];   /**< Sensor name */
    char vendorName[NAME_MAX_LEN];   /**< Sensor vendor */
    char firmwareVersion[VERSION_MAX_LEN];  /**< Sensor firmware version */
    char hardwareVersion[VERSION_MAX_LEN];  /**< Sensor hardware version */
    int32_t sensorTypeId = -1;  /**< Sensor type ID */
    int32_t sensorId = -1;      /**< Sensor ID */
    float maxRange = 0.0;        /**< Maximum measurement range of the sensor */
    float precision = 0.0;       /**< Sensor accuracy */
    float power = 0.0;           /**< Sensor power */
    int64_t minSamplePeriod = -1; /**< Minimum sample period allowed, in ns */
    int64_t maxSamplePeriod = -1; /**< Maximum sample period allowed, in ns */
};

struct Sensor_SubscriptionAttribute {
    int64_t samplingInterval = -1;
    int64_t reportInterval = -1;
};

struct Sensor_SubscriptionId {
    int32_t sensorType = -1;
};

struct Sensor_Subscriber {
    char name[NAME_MAX_LEN];
    Sensor_EventCallback callback;
    UserData *userData = nullptr;
};

struct Sensor_Event {
    int32_t sensorTypeId = -1;
    int32_t version = -1;
    int64_t timestamp = -1;
    int32_t option = -1;
    int32_t mode = -1;
    uint8_t *data = nullptr;
    uint32_t dataLen = 0;
};
#endif // NATIVE_SENSOR_IMPL

