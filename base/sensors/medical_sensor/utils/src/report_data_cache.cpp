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

#include "report_data_cache.h"

#include <cstdlib>
#include <cstring>

#include "errors.h"
#include "securec.h"
#include "medical_errors.h"
#include "medical_log_domain.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_UTILS, "ReportDataCache"
};
}  // namespace
ReportDataCache::ReportDataCache()
{
    eventsBuf_.circularBuf = new struct SensorEvent[CIRCULAR_BUF_LEN];
    eventsBuf_.readPosition = 0;
    eventsBuf_.writePosition = 0;
    eventsBuf_.eventNum = 0;
}

ReportDataCache::~ReportDataCache()
{
    if (eventsBuf_.circularBuf != nullptr) {
        delete[] eventsBuf_.circularBuf;
    }
    eventsBuf_.circularBuf = nullptr;
    eventsBuf_.readPosition = 0;
    eventsBuf_.writePosition = 0;
    eventsBuf_.eventNum = 0;
}

int32_t ReportDataCache::CacheData(const struct SensorEvent* event, sptr<ReportDataCache> cache)
{
    if (cache == nullptr || cache->eventsBuf_.circularBuf == nullptr || event == nullptr) {
        HiLog::Error(LABEL, "%{public}s callback or circularBuf or event cannot be null", __func__);
        return ERROR;
    }
    SensorEvent eventCopy = {
        .sensorTypeId = event->sensorTypeId,
        .version = event->version,
        .timestamp = event->timestamp,
        .option = event->option,
        .mode = event->mode,
        .dataLen = event->dataLen
    };
    uint32_t len = (event->dataLen < SENSOR_DATA_LENGHT) ? event->dataLen : SENSOR_DATA_LENGHT;
    eventCopy.data = new uint8_t[len];
    HiLog::Info(LABEL, "%{public}s dataLength: %{public}d, len = %{public}d", __func__, event->dataLen, len);
    if (memcpy_s(eventCopy.data, len, event->data, event->dataLen) != EOK) {
        HiLog::Error(LABEL, "%{public}s copy data failed", __func__);
        return COPY_ERR;
    }
    int32_t leftSize = CIRCULAR_BUF_LEN - cache->eventsBuf_.eventNum;
    int32_t toEndLen = CIRCULAR_BUF_LEN - cache->eventsBuf_.writePosition;
    if (toEndLen == 0) {
            cache->eventsBuf_.circularBuf[0] = eventCopy;
            cache->eventsBuf_.writePosition = 1 - toEndLen;
    } else {
            cache->eventsBuf_.circularBuf[cache->eventsBuf_.writePosition] = eventCopy;
            cache->eventsBuf_.writePosition += 1;
    }
    if (leftSize < 1) {
        cache->eventsBuf_.readPosition = cache->eventsBuf_.writePosition;
    }
    cache->eventsBuf_.eventNum += 1;
    if (cache->eventsBuf_.eventNum >= CIRCULAR_BUF_LEN) {
        cache->eventsBuf_.eventNum = CIRCULAR_BUF_LEN;
    }
    if (cache->eventsBuf_.writePosition == CIRCULAR_BUF_LEN) {
        cache->eventsBuf_.writePosition = 0;
    }
    return ERR_OK;
}

struct CircularEventBuf &ReportDataCache::GetEventData()
{
    return eventsBuf_;
}
}  // namespace Sensors
}  // namespace OHOS
