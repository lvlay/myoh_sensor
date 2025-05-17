/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef SENSOR_LOG_H
#define SENSOR_LOG_H

#include "hilog/log.h"

namespace OHOS {
namespace Sensors {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002700

#ifndef SENSOR_FUNC_FMT
#define SENSOR_FUNC_FMT "in %{public}s, "
#endif

#ifndef SENSOR_FUNC_INFO
#define SENSOR_FUNC_INFO __FUNCTION__
#endif

#define SEN_HILOGD(fmt, ...) do { \
    HILOG_DEBUG(LOG_CORE, SENSOR_FUNC_FMT fmt, SENSOR_FUNC_INFO, ##__VA_ARGS__); \
} while (0)
#define SEN_HILOGI(fmt, ...) do { \
    HILOG_INFO(LOG_CORE, SENSOR_FUNC_FMT fmt, SENSOR_FUNC_INFO, ##__VA_ARGS__); \
} while (0)
#define SEN_HILOGW(fmt, ...) do { \
    HILOG_WARN(LOG_CORE, SENSOR_FUNC_FMT fmt, SENSOR_FUNC_INFO, ##__VA_ARGS__); \
} while (0)
#define SEN_HILOGE(fmt, ...) do { \
    HILOG_ERROR(LOG_CORE, SENSOR_FUNC_FMT fmt, SENSOR_FUNC_INFO, ##__VA_ARGS__); \
} while (0)
#define SENSOR_LOGF(fmt, ...) do { \
    HILOG_FATAL(LOG_CORE, SENSOR_FUNC_FMT fmt, SENSOR_FUNC_INFO, ##__VA_ARGS__); \
} while (0)
} // namespace Sensors
} // namespace OHOS
#endif // SENSOR_LOG_H