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

#include "permission_util.h"

#include "accesstoken_kit.h"
#include "privacy_kit.h"
#include "sensor_agent_type.h"
#include "sensor_log.h"

#undef LOG_TAG
#define LOG_TAG "PermissionUtil"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

std::unordered_map<int32_t, std::string> PermissionUtil::sensorPermissions_ = {
    { SENSOR_TYPE_ID_ACCELEROMETER, ACCELEROMETER_PERMISSION },
    { SENSOR_TYPE_ID_ACCELEROMETER_UNCALIBRATED, ACCELEROMETER_PERMISSION },
    { SENSOR_TYPE_ID_LINEAR_ACCELERATION, ACCELEROMETER_PERMISSION },
    { SENSOR_TYPE_ID_GYROSCOPE, GYROSCOPE_PERMISSION },
    { SENSOR_TYPE_ID_GYROSCOPE_UNCALIBRATED, GYROSCOPE_PERMISSION },
    { SENSOR_TYPE_ID_PEDOMETER_DETECTION, ACTIVITY_MOTION_PERMISSION },
    { SENSOR_TYPE_ID_PEDOMETER, ACTIVITY_MOTION_PERMISSION },
    { SENSOR_TYPE_ID_HEART_RATE, READ_HEALTH_DATA_PERMISSION }
};

int32_t PermissionUtil::CheckSensorPermission(AccessTokenID callerToken, int32_t sensorTypeId)
{
    if (sensorPermissions_.find(sensorTypeId) == sensorPermissions_.end()) {
        return PERMISSION_GRANTED;
    }
    std::string permissionName = sensorPermissions_[sensorTypeId];
    int32_t ret = AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if ((permissionName == ACTIVITY_MOTION_PERMISSION)
        || (permissionName == READ_HEALTH_DATA_PERMISSION)) {
        AddPermissionRecord(callerToken, permissionName, (ret == PERMISSION_GRANTED));
    }
    return ret;
}

void PermissionUtil::AddPermissionRecord(AccessTokenID tokenID, const std::string &permissionName, bool status)
{
    int32_t successCount = status ? 1 : 0;
    int32_t failCount = status ? 0 : 1;
    int32_t ret = PrivacyKit::AddPermissionUsedRecord(tokenID, permissionName, successCount, failCount);
    if (ret != 0) {
        SEN_HILOGE("AddPermissionUsedRecord fail, permissionName:%{public}s, successCount:%{public}d, failCount:%{public}d",
            permissionName.c_str(), successCount, failCount);
    }
}

bool PermissionUtil::IsNativeToken(AccessTokenID callerToken)
{
    int32_t tokenType = AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType != ATokenTypeEnum::TOKEN_NATIVE) {
        SEN_HILOGE("TokenType is not TOKEN_NATIVE, tokenType:%{public}d", tokenType);
        return false;
    }
    return true;
}

int32_t PermissionUtil::CheckManageSensorPermission(AccessTokenID callerToken)
{
    int32_t ret = AccessTokenKit::VerifyAccessToken(callerToken, MANAGE_SENSOR_PERMISSION);
    if (ret != PERMISSION_GRANTED) {
        SEN_HILOGE("Verify MANAGE_SENSOR permission failed, ret:%{public}d", ret);
    }
    return ret;
}
} // namespace Sensors
} // namespace OHOS
