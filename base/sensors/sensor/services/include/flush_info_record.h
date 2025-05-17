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

#ifndef FLUSH_INFO_RECORD_H
#define FLUSH_INFO_RECORD_H

#include <mutex>
#include <unordered_map>
#include <vector>

#include "nocopyable.h"
#include "refbase.h"
#include "singleton.h"

#include "client_info.h"
#include "sensor_agent_type.h"
#include "sensor_basic_data_channel.h"
#include "sensor_errors.h"
#ifdef HDF_DRIVERS_INTERFACE_SENSOR
#include "sensor_hdi_connection.h"
#endif // HDF_DRIVERS_INTERFACE_SENSOR

namespace OHOS {
namespace Sensors {
struct FlushInfo {
    wptr<SensorBasicDataChannel> flushChannel;
    bool flushFromEnable;
    FlushInfo(const sptr<SensorBasicDataChannel> &channel, bool enableFlush)
        : flushChannel(channel), flushFromEnable(enableFlush){};
};

class FlushInfoRecord : public Singleton<FlushInfoRecord> {
public:
    FlushInfoRecord() = default;
    ~FlushInfoRecord()
    {
        flushInfo_.clear();
    }
    std::unordered_map<int32_t, std::vector<FlushInfo>> GetFlushInfo();
    void ClearFlushInfoItem(int32_t sensorId);
    ErrCode SetFlushInfo(int32_t sensorId, const sptr<SensorBasicDataChannel> &channel, bool isFirstFlush);
    bool IsFlushChannelValid(const std::vector<sptr<SensorBasicDataChannel>> &currChannelList,
                           const sptr<SensorBasicDataChannel> &flushChannel);
    int32_t GetFlushChannelIndex(const std::vector<FlushInfo> &flushInfoList,
                           const sptr<SensorBasicDataChannel> &channel);
    ErrCode FlushProcess(const int32_t sensorId, const uint32_t flag, const int32_t pid, const bool isEnableFlush);

private:
    DISALLOW_COPY_AND_MOVE(FlushInfoRecord);
    ClientInfo &clientInfo_ = ClientInfo::GetInstance();
    // sensorId, channel pointer for pending flush.
    std::unordered_map<int32_t, std::vector<FlushInfo>> flushInfo_;
    std::mutex flushInfoMutex_;
};
} // namespace Sensors
} // namespace OHOS
#endif // FLUSH_INFO_RECORD_H
