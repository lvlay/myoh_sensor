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

#ifndef SENSOR_CLIENT_PROXY_H
#define SENSOR_CLIENT_PROXY_H

#include "iremote_proxy.h"
#include "nocopyable.h"

#include "i_sensor_client.h"
#include "sensor_agent_type.h"

namespace OHOS {
namespace Sensors {
class SensorClientProxy : public IRemoteProxy<ISensorClient> {
public:
    explicit SensorClientProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<ISensorClient>(impl)
    {}
    virtual ~SensorClientProxy() = default;

private:
    DISALLOW_COPY_AND_MOVE(SensorClientProxy);
    static inline BrokerDelegator<SensorClientProxy> delegator_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif // SENSOR_CLIENT_PROXY_H
