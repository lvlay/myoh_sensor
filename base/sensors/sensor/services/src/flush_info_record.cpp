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

#include "flush_info_record.h"

#include "sensor_errors.h"

#undef LOG_TAG
#define LOG_TAG "FlushInfoRecord"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

namespace {
constexpr int32_t CHANNEL_NO_FLUSH = -1;
} // namespace

std::unordered_map<int32_t, std::vector<FlushInfo>> FlushInfoRecord::GetFlushInfo()
{
    std::lock_guard<std::mutex> flushLock(flushInfoMutex_);
    return flushInfo_;
}

void FlushInfoRecord::ClearFlushInfoItem(int32_t sensorId)
{
    std::lock_guard<std::mutex> flushLock(flushInfoMutex_);
    auto it = flushInfo_.find(sensorId);
    if (it != flushInfo_.end()) {
        it->second.erase(it->second.begin());
    }
}

ErrCode FlushInfoRecord::SetFlushInfo(int32_t sensorId, const sptr<SensorBasicDataChannel> &channel, bool isFirstFlush)
{
    SEN_HILOGD("sensorId:%{public}u", sensorId);
    CHKPR(channel, INVALID_POINTER);
    FlushInfo flush(channel, isFirstFlush);
    std::lock_guard<std::mutex> flushLock(flushInfoMutex_);
    /* If the sensorId can be found, it indicates that other processes have flushed on this sensor,
    so need to insert this flush command to the end of the vector */
    auto it = flushInfo_.find(sensorId);
    if (it != flushInfo_.end()) {
        it->second.push_back(flush);
    } else {
        std::vector<FlushInfo> vec { flush };
        flushInfo_.insert(std::make_pair(sensorId, vec));
    }
    return ERR_OK;
}

bool FlushInfoRecord::IsFlushChannelValid(const std::vector<sptr<SensorBasicDataChannel>> &currChannelList,
                                          const sptr<SensorBasicDataChannel> &flushChannel)
{
    SEN_HILOGD("Channel list size:%{public}u", static_cast<uint32_t>(currChannelList.size()));
    for (const auto &channel : currChannelList) {
        if (channel == flushChannel) {
            return true;
        }
    }
    return false;
}

int32_t FlushInfoRecord::GetFlushChannelIndex(const std::vector<FlushInfo> &flushInfoList,
                                              const sptr<SensorBasicDataChannel> &channel)
{
    for (size_t i = 0; i < flushInfoList.size(); i++) {
        if (flushInfoList[i].flushChannel.promote() == channel) {
            return i;
        }
    }
    return CHANNEL_NO_FLUSH;
}

ErrCode FlushInfoRecord::FlushProcess(const int32_t sensorId, const uint32_t flag, const int32_t pid,
                                      const bool isEnableFlush)
{
    sptr<SensorBasicDataChannel> channel = clientInfo_.GetSensorChannelByPid(pid);
    CHKPR(channel, ERROR);
    int32_t ret = SetFlushInfo(sensorId, channel, false);
    if (ret != ERR_OK) {
        SEN_HILOGE("Set flush info failed");
        return ret;
    }
    return ERR_OK;
}
} // namespace Sensors
} // namespace OHOS
