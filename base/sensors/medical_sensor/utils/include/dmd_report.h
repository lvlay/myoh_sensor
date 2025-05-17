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

#ifndef DMD_REPORT_H
#define DMD_REPORT_H

#include <map>
#include <mutex>
#include <string>

namespace OHOS {
namespace Sensors {
enum SensorEventId {
    JNI_ENV_VAR_EXCEPTION = 951000106,
    CLASS_NOT_FOUND = 951000107,
    NATIVE_METHOD_REGISTER_EXCEPTION = 951000108,
    JAVA_VM_THREAD_ATTACH_EXCEPTION = 951000109,
    SENSOR_SERVICE_EXCEPTION = 951000110,
    MISC_SERVICE_EXCEPTION = 951000111,
    SENSOR_SERVICE_IPC_EXCEPTION = 951000112,
    MISC_SERVICE_IPC_EXCEPTION = 951000113,
    SENSOR_HDF_SERVICE_EXCEPTION = 951000114,
    SENSOR_DATA_CHANNEL_EXCEPTION = 951000117,
};

class DmdReport {
public:
    DmdReport() = default;
    virtual ~DmdReport() = default;
    static void ReportException(int32_t eventId, const std::string &interfaceName, int32_t error);

private:
    static std::mutex eventMutex_;
    static std::map<int32_t, int64_t> eventMap_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // DMD_REPORT_H
