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

#ifndef PERMISSION_UTIL_H
#define PERMISSION_UTIL_H

#include <string>
#include <unordered_map>

#include "access_token.h"
#include "singleton.h"

namespace OHOS {
namespace Sensors {
const std::string ACCELEROMETER_PERMISSION = "ohos.permission.ACCELEROMETER";
const std::string GYROSCOPE_PERMISSION = "ohos.permission.GYROSCOPE";
const std::string ACTIVITY_MOTION_PERMISSION = "ohos.permission.ACTIVITY_MOTION";
const std::string READ_HEALTH_DATA_PERMISSION = "ohos.permission.READ_HEALTH_DATA";
const std::string MANAGE_SENSOR_PERMISSION = "ohos.permission.MANAGE_SENSOR";

using namespace Security::AccessToken;
class PermissionUtil : public Singleton<PermissionUtil> {
public:
    PermissionUtil() = default;
    virtual ~PermissionUtil() {};
    int32_t CheckSensorPermission(AccessTokenID callerToken, int32_t sensorTypeId);
    bool IsNativeToken(AccessTokenID callerToken);
    int32_t CheckManageSensorPermission(AccessTokenID callerToken);

private:
    void AddPermissionRecord(AccessTokenID tokenID, const std::string &permissionName, bool status);
    static std::unordered_map<int32_t, std::string> sensorPermissions_;
};
} // namespace Sensors
} // namespace OHOS
#endif // PERMISSION_UTIL_H
