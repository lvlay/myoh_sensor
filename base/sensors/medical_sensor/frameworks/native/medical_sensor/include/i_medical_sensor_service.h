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

#ifndef I_SENSOR_SERVICE_H
#define I_SENSOR_SERVICE_H

#include <vector>

#include "errors.h"
#include "i_medical_sensor_client.h"
#include "iremote_broker.h"
#include "medical_sensor.h"
#include "medical_sensor_basic_data_channel.h"

namespace OHOS {
namespace Sensors {
class IMedicalSensorService : public IRemoteBroker {
public:
    IMedicalSensorService() = default;

    virtual ~IMedicalSensorService() = default;

    DECLARE_INTERFACE_DESCRIPTOR(u"IMedicalSensorService");

    virtual ErrCode EnableSensor(uint32_t sensorId, int64_t samplingPeriodNs,
                                 int64_t maxReportDelayNs) = 0;

    virtual ErrCode DisableSensor(uint32_t sensorId) = 0;

    virtual ErrCode SetOption(uint32_t sensorId, uint32_t opt) = 0;

    virtual int32_t GetSensorState(uint32_t sensorId) = 0;

    virtual ErrCode RunCommand(uint32_t sensorId, uint32_t cmdType, uint32_t params) = 0;

    virtual std::vector<MedicalSensor> GetSensorList() = 0;

    virtual ErrCode TransferDataChannel(const sptr<MedicalSensorBasicDataChannel> &sensorBasicDataChannel,
                                        const sptr<IRemoteObject> &sensorClient) = 0;

    virtual ErrCode DestroySensorChannel(sptr<IRemoteObject> sensorClient) = 0;

    enum {
        ENABLE_SENSOR = 0,
        DISABLE_SENSOR,
        GET_SENSOR_STATE,
        RUN_COMMAND,
        GET_SENSOR_LIST,
        TRANSFER_DATA_CHANNEL,
        DESTROY_SENSOR_CHANNEL,
        SET_OPTION,
    };
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // I_SENSOR_SERVICE_H
