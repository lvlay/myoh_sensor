/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef SENSORS_IPC_INTERFACE_CODE_H
#define SENSORS_IPC_INTERFACE_CODE_H

/* SAID:3601 */
namespace OHOS {
namespace Sensors {
enum class SensorInterfaceCode {
    ENABLE_SENSOR = 0,
    DISABLE_SENSOR,
    GET_SENSOR_LIST,
    TRANSFER_DATA_CHANNEL,
    DESTROY_SENSOR_CHANNEL,
    SUSPEND_SENSORS,
    RESUME_SENSORS,
    GET_ACTIVE_INFO_LIST,
    CREATE_SOCKET_CHANNEL,
    DESTROY_SOCKET_CHANNEL,
    ENABLE_ACTIVE_INFO_CB,
    DISABLE_ACTIVE_INFO_CB,
    RESET_SENSORS,
};
}  // namespace Sensors
}  // namespace OHOS
#endif // SENSORS_IPC_INTERFACE_CODE_H
