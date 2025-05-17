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

#ifndef SENSORS_DATA_PROCESSER_H
#define SENSORS_DATA_PROCESSER_H

#include <unordered_map>
#include <vector>

#include "refbase.h"

#include "nocopyable.h"

#include "client_info.h"
#include "fifo_cache_data.h"
#include "flush_info_record.h"
#include "report_data_callback.h"
#include "sensor.h"
#include "sensor_hdi_connection.h"
#include "sensor_data_event.h"

namespace OHOS {
namespace Sensors {
class SensorDataProcesser : public RefBase {
public:
    explicit SensorDataProcesser(const std::unordered_map<int32_t, Sensor> &sensorMap);
    virtual ~SensorDataProcesser();
    int32_t ProcessEvents(sptr<ReportDataCallback> dataCallback);
    int32_t SendEvents(sptr<SensorBasicDataChannel> &channel, SensorData &data);
    static int DataThread(sptr<SensorDataProcesser> dataProcesser, sptr<ReportDataCallback> dataCallback);
    int32_t CacheSensorEvent(const SensorData &data, sptr<SensorBasicDataChannel> &channel);

private:
    DISALLOW_COPY_AND_MOVE(SensorDataProcesser);
    void ReportData(sptr<SensorBasicDataChannel> &channel, SensorData &data);
    bool ReportNotContinuousData(std::unordered_map<int32_t, SensorData> &cacheBuf,
                                 sptr<SensorBasicDataChannel> &channel, SensorData &data);
    void SendNoneFifoCacheData(std::unordered_map<int32_t, SensorData> &cacheBuf,
                               sptr<SensorBasicDataChannel> &channel, SensorData &data, uint64_t periodCount);
    void SendFifoCacheData(std::unordered_map<int32_t, SensorData> &cacheBuf,
                           sptr<SensorBasicDataChannel> &channel, SensorData &data, uint64_t periodCount,
                           uint64_t fifoCount);
    void SendRawData(std::unordered_map<int32_t, SensorData> &cacheBuf, sptr<SensorBasicDataChannel> channel,
                     std::vector<SensorData> events);
    void EventFilter(CircularEventBuf &eventsBuf);
    ClientInfo &clientInfo_ = ClientInfo::GetInstance();
    FlushInfoRecord &flushInfo_ = FlushInfoRecord::GetInstance();
    std::mutex dataCountMutex_;
    std::unordered_map<int32_t, std::vector<sptr<FifoCacheData>>> dataCountMap_;
    std::mutex sensorMutex_;
    std::unordered_map<int32_t, Sensor> sensorMap_;
};
} // namespace Sensors
} // namespace OHOS
#endif // endif SENSORS_DATA_PROCESSER_H
