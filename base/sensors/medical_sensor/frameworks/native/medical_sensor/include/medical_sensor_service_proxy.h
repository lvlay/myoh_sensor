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

#ifndef SENSOR_SERVICE_PROXY_H
#define SENSOR_SERVICE_PROXY_H

#include "errors.h"
#include "i_medical_sensor_service.h"
#include "iremote_proxy.h"
#include "nocopyable.h"
#include "medical_native_type.h"

namespace OHOS {
namespace Sensors {
class MedicalSensorServiceProxy : public IRemoteProxy<IMedicalSensorService> {
public:
    explicit MedicalSensorServiceProxy(const sptr<IRemoteObject> &impl);
    virtual ~MedicalSensorServiceProxy() = default;
    ErrCode EnableSensor(uint32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs) override;
    ErrCode DisableSensor(uint32_t sensorId) override;
    ErrCode SetOption(uint32_t sensorId, uint32_t opt) override;
    int32_t GetSensorState(uint32_t sensorId) override;
    ErrCode RunCommand(uint32_t sensorId, uint32_t cmdType, uint32_t params) override;
    std::vector<MedicalSensor> GetSensorList() override;
    ErrCode TransferDataChannel(const sptr<MedicalSensorBasicDataChannel> &sensorBasicDataChannel,
                                const sptr<IRemoteObject> &sensorClient) override;
    ErrCode DestroySensorChannel(sptr<IRemoteObject> sensorClient) override;

private:
    DISALLOW_COPY_AND_MOVE(MedicalSensorServiceProxy);
    static inline BrokerDelegator<MedicalSensorServiceProxy> delegator_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // SENSOR_SERVICE_PROXY_H
