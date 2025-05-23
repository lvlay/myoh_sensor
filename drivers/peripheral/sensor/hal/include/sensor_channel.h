/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef HDI_SENSOR_CHANNEL_H
#define HDI_SENSOR_CHANNEL_H

#include "hdf_io_service_if.h"
#include "sensor_type.h"

#define HDI_SENSOR_GRAVITY          (9.80665f)
#define HDI_SENSOR_ACCEL_LSB        1024
#define HDI_SENSOR_UNITS            1000
#define HDI_SENSOR_PI               (3.14f)
#define HDI_SENSOR_SEMICIRCLE       (180.0f)
#define HDI_SENSOR_FLOAT_UNITS      (1000.0f)
#define HDI_SENSOR_FLOAT_HUN_UNITS  (100.0f)
#define HDI_SENSOR_FLOAT_TEN_UNITS  (10.0f)
#define MAX_DUMP_DATA_SIZE 10
#define DATA_LENGTH 32

enum SensorDataDimension {
    DATA_X             = 1,
    DATA_XY            = 2,
    DATA_XYZ           = 3,
    DATA_XYZA          = 4,
    DATA_XYZABC        = 6,
    DATA_POSTURE       = 8,
    DATA_MAX_DATA_SIZE = DATA_POSTURE
};

enum SensorStatusAccuracy {
    SENSOR_STATUS_UNRELIABLE = 0,
    SENSOR_STATUS_ACCURACY_LOW = 1,
    SENSOR_STATUS_ACCURACY_MEDIUM = 2,
    SENSOR_STATUS_ACCURACY_HIGH = 3,
};

struct SensorCovertCoff {
    int32_t sensorTypeId;
    int32_t sensorId;
    enum SensorDataDimension dim;
    float coff[DATA_MAX_DATA_SIZE];
};

struct SensorDumpDate {
    int32_t sensorId;
    int32_t version;
    int64_t timestamp;
    uint32_t option;
    int32_t mode;
    uint8_t data[DATA_LENGTH];
    uint32_t dataLen;
};

struct SensorDatePack {
    int32_t count;
    int32_t pos;
    struct SensorDumpDate listDumpArr[MAX_DUMP_DATA_SIZE];
};

int32_t Register(int32_t groupId, RecordDataCallback cb);
int32_t Unregister(int32_t groupId, RecordDataCallback cb);
struct HdfDevEventlistener *GetSensorListener(void);
void SetSensorIdBySensorType(enum SensorTypeTag type, int32_t sensorId);
struct SensorDatePack *GetEventData(void);
void ConvertSensorData(struct SensorEvents *event);

#endif /* HDI_SENSOR_CHANNEL_H */
