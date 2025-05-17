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

#ifndef COMPATIBLE_CONNECTION_H
#define COMPATIBLE_CONNECTION_H

#include "hdi_service_impl.h"
#include "i_sensor_hdi_connection.h"

namespace OHOS {
namespace Sensors {
class CompatibleConnection : public ISensorHdiConnection {
public:
    CompatibleConnection() = default;

    virtual ~CompatibleConnection() {}

    int32_t ConnectHdi() override;

    int32_t GetSensorList(std::vector<MedicalSensor>& sensorList) override;

    int32_t EnableSensor(int32_t sensorId) override;

    int32_t DisableSensor(int32_t sensorId)  override;

    int32_t SetBatch(int32_t sensorId, int64_t samplingInterval, int64_t reportInterval) override;

    int32_t SetMode(int32_t sensorId, int32_t mode) override;

    int32_t SetOption(int32_t sensorId, int32_t option) override;

    int32_t RunCommand(int32_t sensorId, int32_t cmd, int32_t params) override;

    int32_t RegisteDataReport(DataCacheFunc cacheData, sptr<ReportDataCache> reportDataCache) override;

    int32_t DestroyHdiConnection() override;

private:
    DISALLOW_COPY_AND_MOVE(CompatibleConnection);
    static int32_t SensorDataCallback(const struct SensorEvents *event);
    static DataCacheFunc cacheData_;
    static sptr<ReportDataCache> reportDataCache_;
    HdiServiceImpl &hdiServiceImpl_ = HdiServiceImpl::GetInstance();
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // COMPATIBLE_CONNECTION_H