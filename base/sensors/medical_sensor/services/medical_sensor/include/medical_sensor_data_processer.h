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

#ifndef SENSORS_DATA_PROCESSER_H
#define SENSORS_DATA_PROCESSER_H

#include <unordered_map>
#include <vector>

#include "refbase.h"

#include "client_info.h"
#include "fifo_cache_data.h"
#include "flush_info_record.h"
#include "nocopyable.h"
#include "report_data_cache.h"
#include "medical_sensor.h"
#include "medical_log_domain.h"
#include "sensor_hdi_connection.h"
#include "medical_native_type.h"

namespace OHOS {
namespace Sensors {
class MedicalSensorDataProcesser : public RefBase {
public:
    explicit MedicalSensorDataProcesser(const std::unordered_map<uint32_t, MedicalSensor> &sensorMap);
    virtual ~MedicalSensorDataProcesser();
    int32_t ProcessEvents(sptr<ReportDataCache> dataCallback);
    int32_t SendEvents(sptr<MedicalSensorBasicDataChannel> &channel, struct SensorEvent &event);
    static int DataThread(sptr<MedicalSensorDataProcesser> dataProcesser, sptr<ReportDataCache> dataCache);
    int32_t CacheSensorEvent(const struct SensorEvent &event, sptr<MedicalSensorBasicDataChannel> &channel);

private:
    DISALLOW_COPY_AND_MOVE(MedicalSensorDataProcesser);
    void ReportData(sptr<MedicalSensorBasicDataChannel> &channel, struct SensorEvent &event);
    bool ReportNotContinuousData(std::unordered_map<uint32_t, struct SensorEvent> &cacheBuf,
                                 sptr<MedicalSensorBasicDataChannel> &channel, struct SensorEvent &event);
    void SendNoneFifoCacheData(std::unordered_map<uint32_t, struct SensorEvent> &cacheBuf,
                               sptr<MedicalSensorBasicDataChannel> &channel, struct SensorEvent &event,
                               uint64_t periodCount);
    void SendFifoCacheData(std::unordered_map<uint32_t, struct SensorEvent> &cacheBuf,
                           sptr<MedicalSensorBasicDataChannel> &channel, struct SensorEvent &event,
                           uint64_t periodCount, uint64_t fifoCount);
    void SendRawData(std::unordered_map<uint32_t, struct SensorEvent> &cacheBuf,
                     sptr<MedicalSensorBasicDataChannel> channel,
                     std::vector<struct SensorEvent> event);
    void EventFilter(struct CircularEventBuf &eventsBuf);
    bool CheckSendDataPermission(sptr<MedicalSensorBasicDataChannel> channel, uint32_t sensorId);
    ClientInfo &clientInfo_ = ClientInfo::GetInstance();
    FlushInfoRecord &flushInfo_ = FlushInfoRecord::GetInstance();
    std::mutex dataCountMutex_;
    std::unordered_map<uint32_t, std::vector<sptr<FifoCacheData>>> dataCountMap_;
    std::mutex sensorMutex_;
    std::unordered_map<uint32_t, MedicalSensor> sensorMap_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // endif SENSORS_DATA_PROCESSER_H
