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

#ifndef FLUSH_INFO_RECORD_H
#define FLUSH_INFO_RECORD_H

#include <mutex>
#include <unordered_map>
#include <vector>

#include "client_info.h"
#include "refbase.h"
#include "medical_sensor_basic_data_channel.h"
#include "sensor_hdi_connection.h"
#include "medical_errors.h"
#include "singleton.h"
#include "nocopyable.h"
#include "medical_native_type.h"

namespace OHOS {
namespace Sensors {
struct FlushInfo {
    sptr<MedicalSensorBasicDataChannel> flushChannel;
    bool flushFromEnable;

    FlushInfo(const sptr<MedicalSensorBasicDataChannel> &channel, bool enableFlush)
        : flushChannel(channel), flushFromEnable(enableFlush) {};
};

class FlushInfoRecord : public Singleton<FlushInfoRecord> {
public:
    FlushInfoRecord() = default;
    ~FlushInfoRecord()
    {
        flushInfo_.clear();
    }

    std::unordered_map<uint32_t, std::vector<struct FlushInfo>> GetFlushInfo();
    void ClearFlushInfoItem(uint32_t sensorId);
    ErrCode SetFlushInfo(uint32_t sensorId, const sptr<MedicalSensorBasicDataChannel> &channel, bool isFirstFlush);
    bool IsFlushChannelValid(const std::vector<sptr<MedicalSensorBasicDataChannel>> &currChannelList,
                           const sptr<MedicalSensorBasicDataChannel> &flushChannel);
    int32_t GetFlushChannelIndex(const std::vector<struct FlushInfo> &flushInfoList,
                           const sptr<MedicalSensorBasicDataChannel> &channel);
    ErrCode FlushProcess(const uint32_t sensorId, const uint32_t flag, const int32_t pid, const bool isEnableFlush);

private:
    DISALLOW_COPY_AND_MOVE(FlushInfoRecord);
    SensorHdiConnection &sensorHdiConnection_ = SensorHdiConnection::GetInstance();
    ClientInfo &clientInfo_ = ClientInfo::GetInstance();
    // sensorId, channel pointer for pending flush.
    std::unordered_map<uint32_t, std::vector<struct FlushInfo>> flushInfo_;
    std::mutex flushInfoMutex_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // FLUSH_INFO_RECORD_H
