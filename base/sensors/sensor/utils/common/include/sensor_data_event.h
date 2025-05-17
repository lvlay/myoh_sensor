/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef SENSOR_DATA_EVENT_H
#define SENSOR_DATA_EVENT_H

namespace OHOS {
namespace Sensors {
constexpr int32_t RESERVED_DATA_LEN = 3;
constexpr int32_t EXTRA_INFO_DATA_LEN = 14;
constexpr int32_t DEFAULT_SENSOR_DATA_DIMS = 16;
constexpr int32_t SENSOR_MAX_LENGTH = 64;

enum {
    WAKE_UP_SENSOR = 1u,
    CONTINUOUS_SENSOR = 0u,
    ON_CHANGE_SENSOR = 2u,
    ONE_SHOT_SENSOR = 4u,
};

struct SensorData {
    int32_t sensorTypeId;  /**< Sensor type ID */
    int32_t version;       /**< Sensor algorithm version */
    int64_t timestamp;     /**< Time when sensor data was reported */
    int32_t option;       /**< Sensor data options, including the measurement range and accuracy */
    int32_t mode;          /**< Sensor data reporting mode (described in {@link SensorMode}) */
    uint8_t data[SENSOR_MAX_LENGTH];         /**< Sensor data */
    uint32_t dataLen;      /**< Sensor data length */
};

struct ExtraInfo {
    int32_t sensorId;
    int32_t type;    // type of payload data, see ExtraInfo
    int32_t serial;  // sequence number of this frame for this type
    union {
        // for each frame, a single data type, either int32_t or float, should be used.
        int32_t data_int32[EXTRA_INFO_DATA_LEN];
        float data_float[EXTRA_INFO_DATA_LEN];
    };
};

struct ExtraSensorInfo {
    ExtraInfo additional_info;
};
} // namespace Sensors
} // namespace OHOS
#endif // SENSOR_DATA_EVENT_H
