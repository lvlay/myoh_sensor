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

#ifndef FIFO_CACHE_DATA_H
#define FIFO_CACHE_DATA_H

#include <vector>

#include "refbase.h"
#include "medical_native_type.h"
#include "medical_sensor_basic_data_channel.h"

#include "nocopyable.h"

namespace OHOS {
namespace Sensors {
class FifoCacheData : public RefBase {
public:
    FifoCacheData();
    virtual ~FifoCacheData();
    void SetPeriodCount(uint64_t periodCount);
    uint64_t GetPeriodCount() const;
    void SetFifoCacheData(const std::vector<struct SensorEvent> &fifoCacheData);
    std::vector<struct SensorEvent> GetFifoCacheData() const;
    void SetChannel(const sptr<MedicalSensorBasicDataChannel> &channel);
    sptr<MedicalSensorBasicDataChannel> GetChannel() const;
    void InitFifoCache();

private:
    DISALLOW_COPY_AND_MOVE(FifoCacheData);
    uint64_t periodCount_;
    sptr<MedicalSensorBasicDataChannel> channel_;
    std::vector<struct SensorEvent> fifoCacheData_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // FIFO_CACHE_DATA_H