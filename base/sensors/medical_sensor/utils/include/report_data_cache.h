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

#ifndef REPORT_DATA_CALLBACK_H
#define REPORT_DATA_CALLBACK_H

#include <vector>

#include "refbase.h"
#include "medical_native_type.h"

namespace OHOS {
namespace Sensors {
constexpr int32_t CIRCULAR_BUF_LEN = 1024;
constexpr int32_t SENSOR_DATA_LENGHT = 1024;

struct CircularEventBuf {
    struct  SensorEvent *circularBuf;
    int32_t readPosition;
    int32_t writePosition;
    int32_t eventNum;
};

class ReportDataCache : public RefBase {
public:
    ReportDataCache();
    ~ReportDataCache();
    int32_t CacheData(const struct SensorEvent *event, sptr<ReportDataCache> cache);
    struct CircularEventBuf &GetEventData();
    struct CircularEventBuf eventsBuf_;
};

using DataCacheFunc = int32_t (ReportDataCache::*)(const struct SensorEvent *event, sptr<ReportDataCache> cb);
}  // namespace Sensors
}  // namespace OHOS
#endif  // REPORT_DATA_CALLBACK_H
