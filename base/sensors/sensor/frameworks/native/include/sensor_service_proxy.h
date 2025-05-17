/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "iremote_proxy.h"
#include "nocopyable.h"

#include "sensor_agent_type.h"
#include "i_sensor_service.h"

namespace OHOS {
namespace Sensors {
class SensorServiceProxy : public IRemoteProxy<ISensorService> {
public:
    explicit SensorServiceProxy(const sptr<IRemoteObject> &impl);
    virtual ~SensorServiceProxy() = default;
    ErrCode EnableSensor(int32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs) override;
    ErrCode DisableSensor(int32_t sensorId) override;
    std::vector<Sensor> GetSensorList() override;
    ErrCode TransferDataChannel(const sptr<SensorBasicDataChannel> &sensorBasicDataChannel,
                                const sptr<IRemoteObject> &sensorClient) override;
    ErrCode DestroySensorChannel(sptr<IRemoteObject> sensorClient) override;
    ErrCode SuspendSensors(int32_t pid) override;
    ErrCode ResumeSensors(int32_t pid) override;
    ErrCode GetActiveInfoList(int32_t pid, std::vector<ActiveInfo> &activeInfoList) override;
    ErrCode CreateSocketChannel(sptr<IRemoteObject> sensorClient, int32_t &clientFd) override;
    ErrCode DestroySocketChannel(sptr<IRemoteObject> sensorClient) override;
    ErrCode EnableActiveInfoCB() override;
    ErrCode DisableActiveInfoCB() override;
    ErrCode ResetSensors() override;

private:
    DISALLOW_COPY_AND_MOVE(SensorServiceProxy);
    static inline BrokerDelegator<SensorServiceProxy> delegator_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif // SENSOR_SERVICE_PROXY_H
