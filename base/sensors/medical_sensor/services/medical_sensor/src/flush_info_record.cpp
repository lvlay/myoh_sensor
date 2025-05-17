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

#include "flush_info_record.h"

#include "medical_errors.h"
#include "medical_log_domain.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

enum class FlushCmdId {
    FLUSH = 0,
    SET_MODE,
    RESERVED,
};

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_SERVICE, "FlushInfoRecord" };
constexpr int32_t CHANNEL_NO_FLUSH = -1;
}  // namespace

std::unordered_map<uint32_t, std::vector<struct FlushInfo>> FlushInfoRecord::GetFlushInfo()
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    std::lock_guard<std::mutex> flushLock(flushInfoMutex_);
    return flushInfo_;
}

void FlushInfoRecord::ClearFlushInfoItem(uint32_t sensorId)
{
    std::lock_guard<std::mutex> flushLock(flushInfoMutex_);
    auto it = flushInfo_.find(sensorId);
    if (it != flushInfo_.end()) {
        it->second.erase(it->second.begin());
    }
}

ErrCode FlushInfoRecord::SetFlushInfo(uint32_t sensorId,
    const sptr<MedicalSensorBasicDataChannel> &channel, bool isFirstFlush)
{
    HiLog::Debug(LABEL, "%{public}s, sensorId : %{public}u", __func__, sensorId);
    if (channel == nullptr) {
        HiLog::Error(LABEL, "%{public}s failed, channel cannot be null", __func__);
        return INVALID_POINTER;
    }
    FlushInfo flush(channel, isFirstFlush);
    std::lock_guard<std::mutex> flushLock(flushInfoMutex_);
    /* If the sensorId can be found, it indicates that other processes have flushed on this sensor,
    so need to insert this flush command to the end of the vector */
    auto it = flushInfo_.find(sensorId);
    if (it != flushInfo_.end()) {
        it->second.push_back(flush);
    } else {
        std::vector<struct FlushInfo> vec { flush };
        flushInfo_.insert(std::make_pair(sensorId, vec));
    }
    return ERR_OK;
}

bool FlushInfoRecord::IsFlushChannelValid(const std::vector<sptr<MedicalSensorBasicDataChannel>> &currChannelList,
                                          const sptr<MedicalSensorBasicDataChannel> &flushChannel)
{
    HiLog::Debug(LABEL, "%{public}s channel list size : %{public}u", __func__,
        static_cast<uint32_t>(currChannelList.size()));
    for (const auto &channel : currChannelList) {
        if (channel == flushChannel) {
            return true;
        }
    }
    return false;
}

int32_t FlushInfoRecord::GetFlushChannelIndex(const std::vector<struct FlushInfo> &flushInfoList,
                                              const sptr<MedicalSensorBasicDataChannel> &channel)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    for (size_t i = 0; i < flushInfoList.size(); i++) {
        if (flushInfoList[i].flushChannel == channel) {
            return i;
        }
    }
    return CHANNEL_NO_FLUSH;
}

ErrCode FlushInfoRecord::FlushProcess(const uint32_t sensorId, const uint32_t flag, const int32_t pid,
                                      const bool isEnableFlush)
{
    auto ret = sensorHdiConnection_.RunCommand(sensorId, static_cast<int32_t>(FlushCmdId::FLUSH), 0);
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s flush command failed", __func__);
        return ret;
    }
    sptr<MedicalSensorBasicDataChannel> channel = clientInfo_.GetSensorChannelByPid(pid);
    if (channel == nullptr) {
        HiLog::Error(LABEL, "%{public}s channel cannot be null", __func__);
        return ERROR;
    }
    ret = SetFlushInfo(sensorId, channel, false);
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s set flush info failed", __func__);
        return ret;
    }
    return ERR_OK;
}
}  // namespace Sensors
}  // namespace OHOS
