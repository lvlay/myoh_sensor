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
#ifndef NAME_MAX_LEN
#define NAME_MAX_LEN 128
#endif /* NAME_MAX_LEN */
/** Size of sensor data */
#ifndef SENSOR_USER_DATA_SIZE
#define SENSOR_USER_DATA_SIZE 104
#endif /* SENSOR_USER_DATA_SIZE */
/** Maximum length of the sensor version */
#ifndef VERSION_MAX_LEN
#define VERSION_MAX_LEN 16
#endif /* SENSOR_USER_DATA_SIZE */

/**
 * @brief Enumerates sensor types.
 *
 * @since 5
 */
typedef enum SensorTypeId {
    SENSOR_TYPE_ID_NONE = 0,                   /**< None */
    SENSOR_TYPE_ID_ACCELEROMETER = 1,          /**< Acceleration sensor */
    SENSOR_TYPE_ID_GYROSCOPE = 2,              /**< Gyroscope sensor */
    SENSOR_TYPE_ID_AMBIENT_LIGHT = 5,          /**< Ambient light sensor */
    SENSOR_TYPE_ID_MAGNETIC_FIELD = 6,         /**< Magnetic field sensor */
    SENSOR_TYPE_ID_CAPACITIVE = 7,             /**< Capacitive sensor */
    SENSOR_TYPE_ID_BAROMETER = 8,              /**< Barometric pressure sensor */
    SENSOR_TYPE_ID_TEMPERATURE = 9,            /**< Temperature sensor */
    SENSOR_TYPE_ID_HALL = 10,                  /**< Hall effect sensor */
    SENSOR_TYPE_ID_GESTURE = 11,               /**< Gesture sensor */
    SENSOR_TYPE_ID_PROXIMITY = 12,             /**< Proximity sensor */
    SENSOR_TYPE_ID_HUMIDITY = 13,              /**< Humidity sensor */
    SENSOR_TYPE_ID_COLOR = 14,                 /**< Color sensor */
    SENSOR_TYPE_ID_SAR = 15,                   /**< Sar sensor */
    SENSOR_TYPE_ID_AMBIENT_LIGHT1 = 16,        /**< Secondary ambient light sensor */
    SENSOR_TYPE_ID_HALL_EXT = 17,              /**< Extended hall effect sensor */
    SENSOR_TYPE_ID_PROXIMITY1 = 18,            /**< Secondary proximity sensor */
    SENSOR_TYPE_ID_PHYSICAL_MAX = 0xFF,        /**< Maximum type ID of a physical sensor */
    SENSOR_TYPE_ID_ORIENTATION = 256,          /**< Orientation sensor */
    SENSOR_TYPE_ID_GRAVITY = 257,              /**< Gravity sensor */
    SENSOR_TYPE_ID_LINEAR_ACCELERATION = 258,  /**< Linear acceleration sensor */
    SENSOR_TYPE_ID_ROTATION_VECTOR = 259,      /**< Rotation vector sensor */
    SENSOR_TYPE_ID_AMBIENT_TEMPERATURE = 260,  /**< Ambient temperature sensor */
    SENSOR_TYPE_ID_MAGNETIC_FIELD_UNCALIBRATED = 261,  /**< Uncalibrated magnetic field sensor */
    SENSOR_TYPE_ID_GAME_ROTATION_VECTOR = 262,    /**< Game rotation vector sensor */
    SENSOR_TYPE_ID_GYROSCOPE_UNCALIBRATED = 263,  /**< Uncalibrated gyroscope sensor */
    SENSOR_TYPE_ID_SIGNIFICANT_MOTION = 264,    /**< Significant motion sensor */
    SENSOR_TYPE_ID_PEDOMETER_DETECTION = 265,   /**< Pedometer detection sensor */
    SENSOR_TYPE_ID_PEDOMETER = 266,             /**< Pedometer sensor */
    SENSOR_TYPE_ID_POSTURE = 267,               /**< Posture sensor */
    SENSOR_TYPE_ID_HEADPOSTURE = 268,           /**< Head posture sensor */
    SENSOR_TYPE_ID_DROP_DETECTION = 269,       /**< Drop detection sensor */
    SENSOR_TYPE_ID_GEOMAGNETIC_ROTATION_VECTOR = 277,  /**< Geomagnetic rotation vector sensor */
    SENSOR_TYPE_ID_HEART_RATE = 278,            /**< Heart rate sensor */
    SENSOR_TYPE_ID_DEVICE_ORIENTATION = 279,    /**< Device orientation sensor */
    SENSOR_TYPE_ID_WEAR_DETECTION = 280,        /**< Wear detection sensor */
    SENSOR_TYPE_ID_ACCELEROMETER_UNCALIBRATED = 281,   /**< Uncalibrated acceleration sensor */
    SENSOR_TYPE_ID_RPC = 282,     /**< Radio power control sensor */
    SENSOR_TYPE_ID_MAX = 30,      /**< Maximum number of sensor type IDs*/
} SensorTypeId;

/**
 * @brief Defines sensor information.
 *
 * @since 5
 */
typedef struct SensorInfo {
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
} SensorInfo;

/**
 * @brief Enumerates the accuracy levels of data reported by a sensor.
 *
 * @since 11
 */
typedef enum SensorAccuracy {
    /**< The sensor data is unreliable.
     * It is possible that the sensor does not contact with the device to measure.*/
    ACCURACY_UNRELIABLE = 0,
    /**< The sensor data is at a low accuracy level.
     * You are required to calibrate the data based on the environment before using it. */
    ACCURACY_LOW = 1,
    /**< The sensor data is at a medium accuracy level.
     * You are advised to calibrate the data based on the environment before using it. */
    ACCURACY_MEDIUM = 2,
    /**< The sensor data is at a high accuracy level.
     * The data can be used directly. */
    ACCURACY_HIGH = 3,
} SensorAccuracy;

/**
 * @brief Defines the data reported by the sensor.
 *
 * @since 5
 */
typedef struct SensorEvent {
    int32_t sensorTypeId = -1;  /**< Sensor type ID */
    int32_t version = -1;       /**< Sensor algorithm version */
    int64_t timestamp = -1;     /**< Time when sensor data was reported */
    int32_t option = -1;       /**< Sensor data options, including the measurement range and accuracy */
    int32_t mode = -1;          /**< Sensor data reporting mode (described in {@link SensorMode}) */
    uint8_t *data = nullptr;         /**< Sensor data */
    uint32_t dataLen = 0;      /**< Sensor data length */
} SensorEvent;

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
    char userData[SENSOR_USER_DATA_SIZE];  /**< Reserved for the sensor data subscriber */
} UserData;

/**
 * @brief Defines information about the sensor data subscriber.
 *
 * @since 5
 */
typedef struct SensorUser {
    char name[NAME_MAX_LEN];  /**< Name of the sensor data subscriber */
    RecordSensorCallback callback;   /**< Callback for reporting sensor data */
    UserData *userData = nullptr;              /**< Reserved field for the sensor data subscriber */
} SensorUser;

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
 * @brief Defines the struct of the data reported by the acceleration sensor.
 * This sensor measures the acceleration applied to the device on three physical axes (x, y, and z), in m/s2.
 *
 */
typedef struct AccelData {
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
} AccelData;

/**
 * @brief Defines the struct of the data reported by the linear acceleration sensor.
 * This sensor measures the linear acceleration applied to the device on three physical axes (x, y, and z), in m/s2.
 */
typedef struct LinearAccelData {
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
} LinearAccelData;

/**
 * @brief Defines the struct of the data reported by the gyroscope sensor.
 * This sensor measures the angular velocity of the device on three physical axes (x, y, and z), in rad/s.
 */
typedef struct GyroscopeData {
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
} GyroscopeData;

/**
 * @brief Defines the struct of the data reported by the gravity sensor.
 * This sensor measures the acceleration of gravity applied to the device on three physical axes (x, y, and z), in m/s2.
 */
typedef struct GravityData {
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
} GravityData;

/**
 * @brief Defines the struct of the data reported by the uncalibrated acceleration sensor.
 * This sensor measures the uncalibrated acceleration applied to the device on three physical axes (x, y, and z),
 * in m/s2.
 */
typedef struct AccelUncalibratedData {
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
    float biasX = 0.0;
    float biasY = 0.0;
    float biasZ = 0.0;
} AccelUncalibratedData;

/**
 * @brief Defines the struct of the data reported by the uncalibrated gyroscope sensor.
 * This sensor measures the uncalibrated angular velocity of the device on three physical axes (x, y, and z), in rad/s.
 */
typedef struct GyroUncalibratedData {
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
    float biasX = 0.0;
    float biasY = 0.0;
    float biasZ = 0.0;
} GyroUncalibratedData;

/**
 * @brief Defines the struct of the data reported by the significant motion sensor.
 * This sensor detects whether there is substantial motion in the device on the three physical axes (x, y, and z).
 * The value <b>1</b> means that there is substantial motion, and <b>0</b> means the opposite.
 */
typedef struct SignificantMotionData {
    float scalar = 0.0;
} SignificantMotionData;

/**
 * @brief Defines the struct of the data reported by the pedometer detection sensor.
 * This sensor detects whether a user is walking.
 * The value <b>1</b> means that the user is walking, and <b>0</b> means the opposite.
 */
typedef struct PedometerDetectData {
    float scalar = 0.0;
} PedometerDetectData;

/**
 * @brief Defines the struct of the data reported by the pedometer sensor.
 * This sensor counts the number of steps taken by a user.
 */
typedef struct PedometerData {
    float steps = 0.0;
} PedometerData;

/**
 * @brief Defines the struct of the data reported by the ambient temperature sensor.
 * This sensor measures the ambient temperature, in degrees Celsius (°C).
 */
typedef struct AmbientTemperatureData {
    float temperature = 0.0;
} AmbientTemperatureData;

/**
 * @brief Defines the struct of the data reported by the humidity sensor.
 * This sensor measures the relative humidity of the environment,
 * expressed as a percentage (%).
 */
typedef struct HumidityData {
    float humidity = 0.0;
} HumidityData;

/**
 * @brief Defines the struct of the data reported by the temperature sensor.
 * This sensor measures the relative temperature of the environment, in degrees Celsius (°C).
 */
typedef struct TemperatureData {
    float temperature = 0.0;
} TemperatureData;

/**
 * @brief Defines the struct of the data reported by the magnetic field sensor.
 * This sensor measures the ambient geomagnetic field in three physical axes (x, y, z), in μT.
 */
typedef struct MagneticFieldData {
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
} MagneticFieldData;

/**
 * @brief Defines the struct of the data reported by the uncalibrated magnetic field sensor.
 * This sensor measures the uncalibrated ambient geomagnetic field in three physical axes (x, y, z), in μT.
 */
typedef struct MagneticFieldUncalibratedData {
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
    float biasX = 0.0;
    float biasY = 0.0;
    float biasZ = 0.0;
} MagneticFieldUncalibratedData;

/**
 * @brief Defines the struct of the data reported by the barometer sensor.
 * This sensor measures the atmospheric pressure, in hPa or mb.
 */
typedef struct BarometerData {
    float pressure = 0.0;
} BarometerData;

/**
 * @brief Defines the struct of the data reported by the device orientation sensor.
 * This sensor measures the direction of rotation of the device, in rad.
 */
typedef struct DeviceOrientationData {
    float scalar = 0.0;
} DeviceOrientationData;

/**
 * @brief Defines the struct of the data reported by the orientation sensor.
 * This sensor measures the angle of rotation of the device around all three physical axes (z, x, y), in rad.
 */
typedef struct OrientationData {
    float alpha = 0.0; /**< The device rotates at an angle around the Z axis. */
    float beta = 0.0;  /**< The device rotates at an angle around the X axis. */
    float gamma = 0.0; /**< The device rotates at an angle around the Y axis. */
} OrientationData;

/**
 * @brief Defines the struct of the data reported by the rotation vector sensor.
 * This sensor measures the rotation vector of the device.
 * It is synthesized by the acceleration sensor and gyroscope sensor.
 */
typedef struct RotationVectorData {
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
    float w = 0.0;
} RotationVectorData;

/**
 * @brief Defines the struct of the data reported by the game rotation vector sensor.
 * This sensor measures the game rotation vector of the device.
 * It is synthesized by the acceleration sensor and gyroscope sensor.
 */
typedef struct GameRotationVectorData {
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
    float w = 0.0;
} GameRotationVectorData;

/**
 * @brief Defines the struct of the data reported by the geomagnetic rotation vector sensor.
 * This sensor measures the geomagnetic rotation vector of the device.
 * It is synthesized by the acceleration sensor and magnetic field sensor.
 */
typedef struct GeomagneticRotaVectorData {
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
    float w = 0.0;
} GeomagneticRotaVectorData;

/**
 * @brief Defines the struct of the data reported by the proximity light sensor.
 * This sensor measures the proximity or distance of visible objects relative to the device display,
 * where 0 indicates proximity and 1 indicates distance.
 */
typedef struct ProximityData {
    float distance = 0.0;
} ProximityData;

/**
 * @brief Defines the struct of the data reported by the ambient light sensor.
 * This sensor measures the intensity of light around the device, in lux.
 */
typedef struct AmbientLightData {
    float intensity = 0.0;
    float colorTemperature = 0.0;
    float infraredLuminance = 0.0;
} AmbientLightData;

/**
 * @brief Defines the struct of the data reported by the hall effect sensor.
 * This sensor measures whether there is magnetic attraction around the device.
 * The value <b>1</b> means that there is magnet attraction, and <b>0</b> means the opposite.
 */
typedef struct HallData {
    float status = 0.0;
} HallData;

/**
 * @brief Defines the struct of the data reported by the heart rate sensor.
 * This sensor measures a user's heart rate, in bpm.
 */
typedef struct HeartRateData {
    float heartRate = 0.0;
} HeartRateData;

/**
 * @brief Defines the struct of the data reported by the wear detection sensor.
 * This sensor detects whether a user is wearing a wearable device.
 * The value <b>1</b> means that the user is wearing a wearable device, and <b>0</b> means the opposite.
 */
typedef struct WearDetectionData {
    float value = 0.0;
} WearDetectionData;

/**
 * @brief Defines the struct of the data reported by the color sensor.
 * This sensor is used to measure the luminous intensity (in lux) and color temperature (in Kelvin).
 */
typedef struct ColorData {
    float lightIntensity = 0.0;
    float colorTemperature = 0.0;
} ColorData;

/**
 * @brief Defines the struct of the data reported by the SAR sensor.
 * This sensor measures the SAR, in W/kg.
 */
typedef struct SarData {
    float absorptionRatio = 0.0;
} SarData;

/**
 * @brief Defines the struct of the data reported by the RPC sensor.
 * This sensor measures the radio power control.
 */
typedef struct RPCData {
    float absorptionRatio = 0.0;
    float threshold = 0.0;
    float offset = 0.0;
} RPCData;

/**
 * @brief Defines the struct of the data reported by the posture sensor.
 * This sensor measures the angle between two screens, in degrees. The angle ranges from 0 to 180.
 */
typedef struct PostureData {
    float gxm = 0.0; /**< The main screen acceleration on the x axis */
    float gym = 0.0; /**< The main screen acceleration on the y axis */
    float gzm = 0.0; /**< The main screen acceleration on the z axis */
    float gxs = 0.0; /**< The second screen acceleration on the x axis */
    float gys = 0.0; /**< The second screen acceleration on the y axis */
    float gzs = 0.0; /**< The second screen acceleration on the z axis */
    float angle = 0.0; /**< The angle between two screens. The angle ranges from 0 to 180 degrees. */
} PostureData;

/**
 * @brief Defines the struct of the data reported by the head posture sensor.
 * This sensor measures the head posture of user.
 */
typedef struct HeadPostureData {
    int32_t order = 0;
    float w = 0.0;
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
} HeadPostureData;

typedef struct DropDetectionData {
    float status = 0.0F;
} DropDetectionData;

typedef struct SensorActiveInfo {
    int32_t pid = -1;        /**< PID */
    int32_t sensorId = -1;   /**< Sensor ID */
    int64_t samplingPeriodNs = -1;  /**< Sample period, in ns */
    int64_t maxReportDelayNs = -1;  /**< Maximum Report Delay, in ns */
} SensorActiveInfo;

typedef void (*SensorActiveInfoCB)(SensorActiveInfo &sensorActiveInfo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* SENSOR_AGENT_TYPE_H */
/**< @} */
