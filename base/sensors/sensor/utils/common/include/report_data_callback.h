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

#ifndef REPORT_DATA_CALLBACK_H
#define REPORT_DATA_CALLBACK_H

#include "refbase.h"
#include "sensor_data_event.h"

namespace OHOS {
namespace Sensors {
constexpr int32_t CIRCULAR_BUF_LEN = 1024;
constexpr int32_t SENSOR_DATA_LENGTH = 64;

struct CircularEventBuf {
    struct SensorData *circularBuf;
    int32_t readPos;
    int32_t writePosition;
    int32_t eventNum;
};

class ReportDataCallback : public RefBase {
public:
    ReportDataCallback();
    ~ReportDataCallback();
    int32_t ReportEventCallback(SensorData *sensorData, sptr<ReportDataCallback> cb);
    CircularEventBuf &GetEventData();
    CircularEventBuf eventsBuf_;
};

using ReportDataCb = int32_t (ReportDataCallback::*)(SensorData *sensorData, sptr<ReportDataCallback> cb);
} // namespace Sensors
} // namespace OHOS
#endif // REPORT_DATA_CALLBACK_H
