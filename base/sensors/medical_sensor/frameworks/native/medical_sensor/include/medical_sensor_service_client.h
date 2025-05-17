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

#ifndef SENSOR_SERVICE_CLIENT_H
#define SENSOR_SERVICE_CLIENT_H

#include <map>
#include <vector>
#include "iservice_registry.h"
#include "medical_sensor.h"
#include "medical_sensor_basic_data_channel.h"
#include "medical_sensor_basic_info.h"
#include "medical_sensor_client_stub.h"
#include "medical_sensor_data_channel.h"
#include "medical_sensor_service_proxy.h"
#include "singleton.h"
#include "medical_native_type.h"

namespace OHOS {
namespace Sensors {
class MedicalSensorServiceClient : public Singleton<MedicalSensorServiceClient> {
public:
    std::vector<MedicalSensor> GetSensorList();
    int32_t EnableSensor(uint32_t sensorId, int64_t samplingPeroid, int64_t maxReportDelay);
    int32_t DisableSensor(uint32_t sensorId);
    int32_t RunCommand(uint32_t sensorId, int32_t cmdType, int32_t parms);
    int32_t TransferDataChannel(sptr<MedicalSensorDataChannel> sensorDataChannel);
    int32_t DestroyDataChannel();
    void ProcessDeathObserver(const wptr<IRemoteObject> &object);
    int32_t SetOption(uint32_t sensorId, uint32_t opt);

private:
    int32_t InitServiceClient();
    void UpdateSensorInfoMap(uint32_t sensorId, int64_t samplingPeroid, int64_t maxReportDelay);
    void DeleteSensorInfoItem(uint32_t sensorId);
    bool IsValidSensorId(uint32_t sensorId);
    std::mutex clientMutex_;
    sptr<IRemoteObject::DeathRecipient> serviceDeathObserver_;
    sptr<IMedicalSensorService> afeServer_;
    std::vector<MedicalSensor> afeList_;
    sptr<MedicalSensorDataChannel> dataChannel_;
    sptr<MedicalSensorClientStub> afeClientStub_;
    std::mutex mapMutex_;
    std::map<uint32_t, MedicalSensorBasicInfo> sensorInfoMap_;
};
}  // namespace Sensors
}  // namespace OHOS

#endif  // SENSOR_SERVICE_CLIENT_H
