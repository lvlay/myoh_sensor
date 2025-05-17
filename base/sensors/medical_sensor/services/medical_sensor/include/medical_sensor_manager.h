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

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <mutex>
#include <thread>
#include <unordered_map>

#include "client_info.h"
#include "flush_info_record.h"
#include "medical_sensor_data_processer.h"
#include "sensor_hdi_connection.h"
#include "medical_native_type.h"

namespace OHOS {
namespace Sensors {
class MedicalSensorManager : public Singleton<MedicalSensorManager> {
public:
    bool SetBestSensorParams(uint32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs);
    bool ResetBestSensorParams(uint32_t sensorId);
    ErrCode SaveSubscriber(uint32_t sensorId, uint32_t pid, int64_t samplingPeriodNs, int64_t maxReportDelayNs);
    void StartDataReportThread();
    MedicalSensorBasicInfo GetSensorInfo(uint32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs);
    bool IsOtherClientUsingSensor(uint32_t sensorId, int32_t clientPid);
    ErrCode AfterDisableSensor(uint32_t sensorId);
    void InitSensorMap(std::unordered_map<uint32_t, MedicalSensor> &sensorMap,
                       sptr<MedicalSensorDataProcesser> dataProcesser,
                       sptr<ReportDataCache> dataCache);
    uint32_t GetSensorFlag(uint32_t sensorId);
    void GetPackageNameFromUid(int32_t uid, std::string &packageName);

private:
    SensorHdiConnection &sensorHdiConnection_ = SensorHdiConnection::GetInstance();
    ClientInfo &clientInfo_ = ClientInfo::GetInstance();
    std::thread dataThread_;
    sptr<MedicalSensorDataProcesser> sensorDataProcesser_;
    sptr<ReportDataCache> reportDataCache_;
    std::unordered_map<uint32_t, MedicalSensor> sensorMap_;
    std::mutex sensorMapMutex_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // SENSOR_MANAGER_H
