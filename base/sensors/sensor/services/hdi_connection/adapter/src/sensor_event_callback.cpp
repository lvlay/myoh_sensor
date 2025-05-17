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
#include "sensor_event_callback.h"

#include <set>

#include "hdi_connection.h"
#include "print_sensor_data.h"
#include "sensor_agent_type.h"
#include "sensor_errors.h"

#undef LOG_TAG
#define LOG_TAG "HdiConnection"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;
namespace {
std::unique_ptr<HdiConnection> HdiConnection_ = std::make_unique<HdiConnection>();
constexpr int32_t HEADPOSTURE_DATA_SIZE = 20;
const std::set<int32_t> g_sensorTypeTrigger = {
    SENSOR_TYPE_ID_PROXIMITY,
    SENSOR_TYPE_ID_DROP_DETECTION,
    SENSOR_TYPE_ID_HALL,
    SENSOR_TYPE_ID_HALL_EXT,
    SENSOR_TYPE_ID_PROXIMITY1
};
} // namespace

int32_t SensorEventCallback::OnDataEvent(const HdfSensorEvents &event)
{
    ReportDataCb reportDataCb_ = HdiConnection_->GetReportDataCb();
    sptr<ReportDataCallback> reportDataCallback_ = HdiConnection_->GetReportDataCallback();
    CHKPR(reportDataCb_, ERR_NO_INIT);
    CHKPR(reportDataCallback_, ERR_NO_INIT);
    int32_t dataSize = static_cast<int32_t>(event.data.size());
    if (dataSize == 0) {
        SEN_HILOGI("Data is empty");
        return ERR_INVALID_VALUE;
    }
    SensorData sensorData = {
        .sensorTypeId = event.sensorId,
        .version = event.version,
        .timestamp = event.timestamp,
        .option = event.option,
        .mode = event.mode,
        .dataLen = event.dataLen
    };
    if (g_sensorTypeTrigger.find(sensorData.sensorTypeId) != g_sensorTypeTrigger.end()) {
        sensorData.mode = SENSOR_ON_CHANGE;
    }
    CHKPR(sensorData.data, ERR_NO_INIT);
    if (sensorData.sensorTypeId == SENSOR_TYPE_ID_HEADPOSTURE) {
        sensorData.dataLen = HEADPOSTURE_DATA_SIZE;
        const float *inputFloatPtr = reinterpret_cast<const float *>(event.data.data());
        float *outputFloatPtr = reinterpret_cast<float *>(sensorData.data);
        int32_t *outputIntPtr = reinterpret_cast<int32_t *>(sensorData.data);
        outputIntPtr[0] = static_cast<int32_t>(*(inputFloatPtr + 1));
        if (outputIntPtr[0] < 0) {
            SEN_HILOGE("The order of head posture sensor is invalid");
        }
        outputFloatPtr[1] = *(inputFloatPtr + 3);
        outputFloatPtr[2] = *(inputFloatPtr + 4);
        outputFloatPtr[3] = *(inputFloatPtr + 5);
        outputFloatPtr[4] = *(inputFloatPtr + 6);
    } else {
        for (int32_t i = 0; i < dataSize; i++) {
            sensorData.data[i] = event.data[i];
        }
    }
    PrintSensorData::GetInstance().ControlSensorHdiPrint(sensorData);
    std::unique_lock<std::mutex> lk(ISensorHdiConnection::dataMutex_);
    (void)(reportDataCallback_->*(reportDataCb_))(&sensorData, reportDataCallback_);
    ISensorHdiConnection::dataCondition_.notify_one();
    return ERR_OK;
}
} // namespace Sensors
} // namespace OHOS