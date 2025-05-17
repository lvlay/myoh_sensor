/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef SENSOR_POWER_POLICY_H
#define SENSOR_POWER_POLICY_H

#include <mutex>
#include <unordered_map>
#include <vector>

#include "nocopyable.h"

#include "active_info.h"
#include "client_info.h"
#include "sensor_errors.h"
#include "sensor_manager.h"
#include "stream_session.h"
#ifdef HDF_DRIVERS_INTERFACE_SENSOR
#include "sensor_hdi_connection.h"
#endif // HDF_DRIVERS_INTERFACE_SENSOR

namespace OHOS {
namespace Sensors {
class SensorPowerPolicy : public Singleton<SensorPowerPolicy> {
public:
    ErrCode SuspendSensors(int32_t pid);
    ErrCode ResumeSensors(int32_t pid);
    ErrCode ResetSensors();
    std::vector<ActiveInfo> GetActiveInfoList(int32_t pid);
    void ReportActiveInfo(const ActiveInfo &activeInfo, const std::vector<SessionPtr> &sessionList);

private:
    bool CheckFreezingSensor(int32_t sensorId);
    bool Suspend(int32_t pid, const std::vector<int32_t> &sensorIdList,
        std::unordered_map<int32_t, SensorBasicInfo> &SensorInfoMap);
    bool Resume(int32_t pid, int32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs);
    ErrCode RestoreSensorInfo(int32_t pid, int32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs);
    std::vector<int32_t> GetSuspendPidList();
    std::mutex pidSensorInfoMutex_;
    std::unordered_map<int32_t, std::unordered_map<int32_t, SensorBasicInfo>> pidSensorInfoMap_;
};
} // namespace Sensors
} // namespace OHOS
#endif // SENSOR_POWER_POLICY_H