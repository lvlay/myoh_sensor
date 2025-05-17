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

#ifndef I_SENSOR_HDI_CONNECTION_H
#define I_SENSOR_HDI_CONNECTION_H

#include <condition_variable>
#include <mutex>
#include "report_data_callback.h"
#include "sensor.h"

namespace OHOS {
namespace Sensors {
class ISensorHdiConnection {
public:
    ISensorHdiConnection() = default;
    virtual ~ISensorHdiConnection() = default;
    virtual int32_t ConnectHdi() = 0;
    virtual int32_t GetSensorList(std::vector<Sensor> &sensorList) = 0;
    virtual int32_t EnableSensor(int32_t sensorId) = 0;
    virtual int32_t DisableSensor(int32_t sensorId)  = 0;
    virtual int32_t SetBatch(int32_t sensorId, int64_t samplingInterval, int64_t reportInterval) = 0;
    virtual int32_t SetMode(int32_t sensorId, int32_t mode) = 0;
    virtual int32_t RegisterDataReport(ReportDataCb cb, sptr<ReportDataCallback> reportDataCallback) = 0;
    virtual int32_t DestroyHdiConnection() = 0;
    static std::mutex dataMutex_;
    static std::condition_variable dataCondition_;

private:
    DISALLOW_COPY_AND_MOVE(ISensorHdiConnection);
};
} // namespace Sensors
} // namespace OHOS
#endif // I_SENSOR_HDI_CONNECTION_H