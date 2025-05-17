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

#include "permission_util.h"

#include <thread>

#include "accesstoken_kit.h"
#include "privacy_kit.h"
#include "medical_errors.h"
#include "medical_log_domain.h"
#include "medical_native_type.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_UTILS, "PermissionUtil" };
const std::string READ_HEALTH_DATA_PERMISSION = "ohos.permission.READ_HEALTH_DATA";
}  // namespace

std::unordered_map<uint32_t, std::string> PermissionUtil::sensorPermissions_ = {
    { TYPE_ID_PHOTOPLETHYSMOGRAPH, READ_HEALTH_DATA_PERMISSION },
    { TYPE_ID_HEART_RATE, READ_HEALTH_DATA_PERMISSION }
};

int32_t PermissionUtil::CheckSensorPermission(AccessTokenID callerToken, int32_t sensorTypeId)
{
    if (sensorPermissions_.find(sensorTypeId) == sensorPermissions_.end()) {
        return PERMISSION_GRANTED;
    }
    std::string permissionName = sensorPermissions_[sensorTypeId];
    int32_t ret = AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    AddPermissionRecord(callerToken, permissionName, (ret == PERMISSION_GRANTED));
    return ret;
}

void PermissionUtil::AddPermissionRecord(AccessTokenID tokenID, const std::string& permissionName, bool status)
{
    int32_t successCount = status ? 1 : 0;
    int32_t failCount = status ? 0 : 1;
    int32_t ret = PrivacyKit::AddPermissionUsedRecord(tokenID, permissionName, successCount, failCount);
    if (ret != 0) {
        HiLog::Error(LABEL,
            "AddPermissionUsedRecord fail,permissionName:%{public}s,successCount:%{public}d,failCount:%{public}d",
            permissionName.c_str(), successCount, failCount);
    }
}
}  // namespace Sensors
}  // namespace OHOS
