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

#include "dmd_report.h"

#include "datetime_ex.h"
#include "hisysevent.h"

#include "medical_errors.h"
#include "medical_log_domain.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_UTILS, "DmdReport" };
constexpr int32_t SECONDS_HALF_HOUR = 1800;
}  // namespace

std::map<int32_t, int64_t> DmdReport::eventMap_ = {
    { JNI_ENV_VAR_EXCEPTION, 0 },
    { CLASS_NOT_FOUND, 0 },
    { NATIVE_METHOD_REGISTER_EXCEPTION, 0 },
    { JAVA_VM_THREAD_ATTACH_EXCEPTION, 0 },
    { SENSOR_SERVICE_EXCEPTION, 0 },
    { MISC_SERVICE_EXCEPTION, 0 },
    { SENSOR_SERVICE_IPC_EXCEPTION, 0 },
    { MISC_SERVICE_IPC_EXCEPTION, 0 },
    { SENSOR_HDF_SERVICE_EXCEPTION, 0 },
    { SENSOR_DATA_CHANNEL_EXCEPTION, 0 },
};

std::mutex DmdReport::eventMutex_;

static std::string GetEventName(int32_t eventId)
{
    switch (eventId) {
        case JNI_ENV_VAR_EXCEPTION:
            return "JniEnvVarException";
        case CLASS_NOT_FOUND:
            return "ClassNotFound";
        case NATIVE_METHOD_REGISTER_EXCEPTION:
            return "NativeMethodRegisterException";
        case JAVA_VM_THREAD_ATTACH_EXCEPTION:
            return "JavaVmThreadAttachException";
        case SENSOR_SERVICE_EXCEPTION:
            return "SensorServiceException";
        case MISC_SERVICE_EXCEPTION:
            return "MiscServiceException";
        case SENSOR_SERVICE_IPC_EXCEPTION:
            return "SensorServiceIpcException";
        case MISC_SERVICE_IPC_EXCEPTION:
            return "MiscServiceIpcException";
        case SENSOR_HDF_SERVICE_EXCEPTION:
            return "SensorHidlServiceException";
        case SENSOR_DATA_CHANNEL_EXCEPTION:
            return "SensorDataChannelException";
        default:
            return "";
    }
}

void DmdReport::ReportException(int32_t eventId, const std::string &interfaceName, int32_t error)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    std::lock_guard<std::mutex> eventLock(eventMutex_);
    auto eventIt = eventMap_.find(eventId);
    if (eventIt == eventMap_.end()) {
        HiLog::Error(LABEL, "%{public}s eventId : %{public}d is not supported", __func__, eventId);
        return;
    }
    int64_t curTime = GetSecondsSince1970ToNow();
    if ((curTime - eventIt->second) > SECONDS_HALF_HOUR) {
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::MEDICAL_SENSOR, GetEventName(eventId),
            HiviewDFX::HiSysEvent::EventType::FAULT, interfaceName, error);
        HiLog::Error(LABEL, "%{public}s eventId : %{public}d, interfaceName=%{public}s, error=%{public}d",
            __func__, eventId, interfaceName.c_str(), error);
        eventMap_[eventId] = curTime;
        HiLog::Debug(LABEL, "%{public}s end", __func__);
        return;
    }
    HiLog::Warn(LABEL, "%{public}s eventId is reported every half an hour", __func__);
}
}  // namespace Sensors
}  // namespace OHOS
