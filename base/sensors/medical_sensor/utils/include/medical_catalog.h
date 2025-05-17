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

#ifndef SENSOR_CATALOG_H
#define SENSOR_CATALOG_H

namespace OHOS {
namespace Sensors {
enum SensorGroup {
    DEVICE_MOTION = 0,
    ENVIRONMENT = 1,
    ORIENTATION = 2,
    LIGHT = 3,
    OTHER = 4,
    BODY = 5,
};

enum GroupMotion {
    SENSOR_TYPE_ACCELEROMETER = 0,
    SENSOR_TYPE_ACCELEROMETER_UNCALIBRATED = 1,
    SENSOR_TYPE_LINEAR_ACCELERATION = 2,
    SENSOR_TYPE_GRAVITY = 3,
    SENSOR_TYPE_GYROSCOPE = 4,
    SENSOR_TYPE_GYROSCOPE_UNCALIBRATED = 5,
    SENSOR_TYPE_SIGNIFICANT_MOTION = 6,
    SENSOR_TYPE_DROP_DETECTION = 7,
    SENSOR_TYPE_STEP_DETECTOR = 8,
    SENSOR_TYPE_STEP_COUNTER = 9,
};

enum GroupEnvironment {
    SENSOR_TYPE_AMBIENT_TEMPERATURE = 0,
    SENSOR_TYPE_MAGNETIC_FIELD = 1,
    SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED = 2,
    SENSOR_TYPE_HUMIDITY = 3,
    SENSOR_TYPE_BAROMETER = 4,
    SENSOR_TYPE_SAR = 5,
};

enum GroupOrientation {
    SENSOR_TYPE_6DOF_ATTITUDE = 0,
    SENSOR_TYPE_SCREEN_ROTATION = 1,
    SENSOR_TYPE_DEVICE_ORIENTATION = 2,
    SENSOR_TYPE_ORIENTATION = 3,
    SENSOR_TYPE_ROTATION_VECTOR = 4,
    SENSOR_TYPE_GAME_ROTATION_VECTOR = 5,
    SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR = 6,
};

enum GroupLight {
    SENSOR_TYPE_PROXIMITY = 0,
    SENSOR_TYPE_TOF = 1,
    SENSOR_TYPE_AMBIENT_LIGHT = 2,
    SENSOR_TYPE_COLOR_TEMPERATURE = 3,
    SENSOR_TYPE_COLOR_RGB = 4,
    SENSOR_TYPE_COLOR_XYZ = 5,
};

enum GroupOther {
    SENSOR_TYPE_HALL = 0,
    SENSOR_TYPE_GRIP_DETECTOR = 1,
    SENSOR_TYPE_MAGNET_BRACKET = 2,
    SENSOR_TYPE_PRESSURE_DETECTOR = 3,
    SENSOR_TYPE_FLUSH = 4,
};

enum GroupBody {
    SENSOR_TYPE_HEART_RATE_DETECTOR = 0,
    SENSOR_TYPE_WEAR_DETECTOR = 1,
};

struct SensorCombination {
    SensorGroup sensorCatagory;
    uint8_t sensorType;
    uint8_t sensorIndex;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // SENSOR_CATALOG_H
