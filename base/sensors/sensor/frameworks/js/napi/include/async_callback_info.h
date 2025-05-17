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
#ifndef ASYNC_CALLBACK_INFO_H
#define ASYNC_CALLBACK_INFO_H
#include <uv.h>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "refbase.h"

#include "sensor_agent_type.h"
#include "sensor_errors.h"
#include "sensor_log.h"

#undef LOG_TAG
#define LOG_TAG "SensorJsAPI"

namespace OHOS {
namespace Sensors {
using std::vector;
using std::string;
using namespace OHOS::HiviewDFX;
constexpr int32_t THREE_DIMENSIONAL_MATRIX_LENGTH = 9;
constexpr static int32_t DATA_LENGTH = 16;
constexpr int32_t CALLBACK_NUM = 3;
enum CallbackDataType {
    SUBSCRIBE_FAIL = -2,
    FAIL = -1,
    OFF_CALLBACK = 0,
    ON_CALLBACK = 1,
    ONCE_CALLBACK = 2,
    GET_GEOMAGNETIC_FIELD = 3,
    GET_ALTITUDE = 4,
    GET_GEOMAGNETIC_DIP = 5,
    GET_ANGLE_MODIFY = 6,
    CREATE_ROTATION_MATRIX = 7,
    TRANSFORM_COORDINATE_SYSTEM = 8,
    CREATE_QUATERNION = 9,
    GET_DIRECTION = 10,
    ROTATION_INCLINATION_MATRIX = 11,
    GET_SENSOR_LIST = 12,
    GET_SINGLE_SENSOR = 13,
    SUBSCRIBE_CALLBACK = 14,
    SUBSCRIBE_COMPASS = 15,
    GET_BODY_STATE = 16,
};

struct GeomagneticData {
    float x;
    float y;
    float z;
    float geomagneticDip;
    float deflectionAngle;
    float levelIntensity;
    float totalIntensity;
};

struct RationMatrixData {
    float rotationMatrix[THREE_DIMENSIONAL_MATRIX_LENGTH];
    float inclinationMatrix[THREE_DIMENSIONAL_MATRIX_LENGTH];
};

struct CallbackSensorData {
    int32_t sensorTypeId;
    uint32_t dataLength;
    float data[DATA_LENGTH];
    int64_t timestamp;
    int32_t sensorAccuracy;
};

struct ReserveData {
    float reserve[DATA_LENGTH];
    int32_t length;
};

union CallbackData {
    CallbackSensorData sensorData;
    GeomagneticData geomagneticData;
    RationMatrixData rationMatrixData;
    ReserveData reserveData;
};

struct BusinessError {
    int32_t code { 0 };
    string message;
    string name;
    string stack;
};

class AsyncCallbackInfo : public RefBase {
public:
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    uv_work_t *work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback[CALLBACK_NUM] = { 0 };
    CallbackData data;
    BusinessError error;
    CallbackDataType type;
    vector<SensorInfo> sensorInfos;
    AsyncCallbackInfo(napi_env env, CallbackDataType type) : env(env), type(type) {}
    ~AsyncCallbackInfo()
    {
        CALL_LOG_ENTER;
        if (type != ONCE_CALLBACK) {
            if (asyncWork != nullptr) {
                SEN_HILOGD("Delete async work");
                napi_delete_async_work(env, asyncWork);
            }
            for (int32_t i = 0; i < CALLBACK_NUM; ++i) {
                if (callback[i] != nullptr) {
                    SEN_HILOGD("Delete reference, i:%{public}d", i);
                    napi_delete_reference(env, callback[i]);
                }
            }
        }
    }

private:
};
}  // namespace Sensors
}  // namespace OHOS
#endif // ASYNC_CALLBACK_INFO_H