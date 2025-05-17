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

/**
 * @addtogroup PanSensor
 * @{
 *
 * @brief Provides standard open APIs for you to use common capabilities of sensors.
 *
 * For example, you can call these APIs to obtain sensor information,
 * subscribe to or unsubscribe from sensor data, enable or disable a sensor,
 * and set the sensor data reporting mode.
 *
 * @since 5
 */

/**
 * @file sensor_agent_type.h
 *
 * @brief Defines the basic data used by the sensor agent to manage sensors.
 *
 * @since 5
 */

#ifndef SENSOR_AGENT_TYPE_H
#define SENSOR_AGENT_TYPE_H

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/** Maximum length of the sensor name */
#ifndef AFE_NAME_MAX_LEN2
#define AFE_NAME_MAX_LEN2 48
#endif /* SENSOR_NAME_MAX_LEN */
/** Size of sensor data */
#ifndef AFE_USER_DATA_SIZE
#define AFE_USER_DATA_SIZE 512
#endif /* AFE_USER_DATA_SIZE */
/** Maximum length of the sensor version */
#ifndef VERSION_MAX_LEN
#define VERSION_MAX_LEN 16
#endif /* VERSION_MAX_LEN */

/**
 * @brief Enumerates sensor types.
 *
 * @since 5
 */
typedef enum {
    TYPE_ID_NONE = 0,                   /**< None, for testing only */
    TYPE_ID_PHOTOPLETHYSMOGRAPH = 129,    /**< Photoplethysmography sensor */
    TYPE_ID_ELECTROCARDIOGRAPH = 130,     /**< Electrocardiogram (ECG) sensor */
    TYPE_ID_HEART_RATE = 278,            /**< Heart rate sensor */
    TYPE_ID_WEAR_DETECTION = 280,        /**< Wear detection sensor */
    TYPE_ID_MAX = 0xFFF,      /**< Maximum number of sensor type IDs */
} MedicalSensorTypeId;

/**
 * @brief Defines sensor information.
 *
 * @since 5
 */
typedef struct MedicalSensorInfo {
    char sensorName[AFE_NAME_MAX_LEN2];   /**< Sensor name */
    char vendorName[AFE_NAME_MAX_LEN2];   /**< Sensor vendor */
    char firmwareVersion[VERSION_MAX_LEN];  /**< Sensor firmware version */
    char hardwareVersion[VERSION_MAX_LEN];  /**< Sensor hardware version */
    int32_t sensorTypeId;  /**< Sensor type ID */
    int32_t sensorId;      /**< Sensor ID */
    float maxRange;        /**< Maximum measurement range of the sensor */
    float precision;       /**< Sensor accuracy */
    float power;           /**< Sensor power */
} MedicalSensorInfo;

/**
 * @brief Defines the data reported by the sensor.
 *
 * @since 5
 */
typedef struct SensorEvent {
    int32_t sensorTypeId;  /**< Sensor type ID */
    int32_t version;       /**< Sensor algorithm version */
    int64_t timestamp;     /**< Time when sensor data was reported */
    uint32_t option;       /**< Sensor data options, including the measurement range and accuracy */
    int32_t mode;          /**< Sensor data reporting mode (described in {@link SensorMode}) */
    uint8_t *data;         /**< Sensor data */
    uint32_t dataLen;      /**< Sensor data length */
} SensorEvent;

struct Test {
char ip[30];
char name[30];
int32_t *data;
};

/**
 * @brief Defines the callback for data reporting by the sensor agent.
 *
 * @since 5
 */
typedef void (*RecordSensorCallback)(SensorEvent *event);

/**
 * @brief Defines a reserved field for the sensor data subscriber.
 *
 * @since 5
 */
typedef struct UserData {
    uint8_t userData[AFE_USER_DATA_SIZE];  /**< Reserved for the sensor data subscriber */
} UserData;

/**
 * @brief Defines information about the sensor data subscriber.
 *
 * @since 5
 */
typedef struct MedicalSensorUser {
    char name[AFE_NAME_MAX_LEN2];  /**< Name of the sensor data subscriber */
    RecordSensorCallback callback;   /**< Callback for reporting sensor data */
    UserData *userData;              /**< Reserved field for the sensor data subscriber */
} MedicalSensorUser;

/**
 * @brief Enumerates data reporting modes of sensors.
 *
 * @since 5
 */
typedef enum SensorMode {
    SENSOR_DEFAULT_MODE = 0,   /**< Default data reporting mode */
    SENSOR_REALTIME_MODE = 1,  /**< Real-time data reporting mode to report a group of data each time */
    SENSOR_ON_CHANGE = 2,   /**< Real-time data reporting mode to report data upon status changes */
    SENSOR_ONE_SHOT = 3,    /**< Real-time data reporting mode to report data only once */
    SENSOR_FIFO_MODE = 4,   /**< FIFO-based data reporting mode to report data based on the <b>BatchCnt</b> setting */
    SENSOR_MODE_MAX2,        /**< Maximum sensor data reporting mode */
} SensorMode;

/**
 * @brief ppg传感器的数据结构
 */
typedef struct PpgData {
    uint32_t reserve : 2;
    uint32_t adcData : 22;
    uint32_t tl : 2;
    uint32_t rx : 2;
    uint32_t phaseGroup : 4;
} PpgData;


/**
 * @brief 心率传感器的数据结构
 */
typedef struct HeartRateData {
    /**< 心率值，单位bpm */
    int32_t heartRateBpm;
    /**< 心率状态 */
    int32_t temperatureStatus;
} HeartRateData;

/**
 * @brief 佩戴检测传感器的数据结构
 */
typedef struct WearDetectionData {
    /**< 标量 */
    int32_t scalar;
} WearDetectionData;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* SENSOR_AGENT_TYPE_H */
/**< @} */