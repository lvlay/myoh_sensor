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

#ifndef FIFO_CACHE_DATA_H
#define FIFO_CACHE_DATA_H

#include <vector>

#include "nocopyable.h"
#include "refbase.h"
#include "sensor_data_event.h"
#include "sensor_basic_data_channel.h"

namespace OHOS {
namespace Sensors {
class FifoCacheData : public RefBase {
public:
    FifoCacheData();
    virtual ~FifoCacheData();
    void SetPeriodCount(uint64_t periodCount);
    uint64_t GetPeriodCount() const;
    void SetFifoCacheData(const std::vector<SensorData> &fifoCacheData);
    std::vector<SensorData> GetFifoCacheData() const;
    void SetChannel(const sptr<SensorBasicDataChannel> &channel);
    sptr<SensorBasicDataChannel> GetChannel() const;
    void InitFifoCache();

private:
    DISALLOW_COPY_AND_MOVE(FifoCacheData);
    uint64_t periodCount_;
    wptr<SensorBasicDataChannel> channel_;
    std::vector<SensorData> fifoCacheData_;
};
} // namespace Sensors
} // namespace OHOS
#endif // FIFO_CACHE_DATA_H