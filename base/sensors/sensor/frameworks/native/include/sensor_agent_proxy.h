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

#ifndef SENSOR_PROXY_H
#define SENSOR_PROXY_H

#include <atomic>
#include <map>
#include <set>
#include <thread>
#include "refbase.h"
#include "singleton.h"

#include "sensor_agent_type.h"
#include "sensor_data_channel.h"

namespace OHOS {
namespace Sensors {
struct SensorNativeData;
struct SensorIdList;
typedef int32_t (*SensorDataCallback)(struct SensorNativeData *events, uint32_t num);

class SensorAgentProxy {
    DECLARE_DELAYED_SINGLETON(SensorAgentProxy);
public:
    int32_t ActivateSensor(int32_t sensorId, const SensorUser *user);
    int32_t DeactivateSensor(int32_t sensorId, const SensorUser *user);
    int32_t SetBatch(int32_t sensorId, const SensorUser *user, int64_t samplingInterval, int64_t reportInterval);
    int32_t SubscribeSensor(int32_t sensorId, const SensorUser *user);
    int32_t UnsubscribeSensor(int32_t sensorId, const SensorUser *user);
    int32_t SetMode(int32_t sensorId, const SensorUser *user, int32_t mode);
    int32_t SetOption(int32_t sensorId, const SensorUser *user, int32_t option);
    void SetIsChannelCreated(bool isChannelCreated);
    int32_t GetAllSensors(SensorInfo **sensorInfo, int32_t *count) const;
    int32_t SuspendSensors(int32_t pid);
    int32_t ResumeSensors(int32_t pid);
    int32_t GetSensorActiveInfos(int32_t pid, SensorActiveInfo **sensorActiveInfos, int32_t *count) const;
    int32_t Register(SensorActiveInfoCB callback);
    int32_t Unregister(SensorActiveInfoCB callback);
    void HandleSensorData(SensorEvent *events, int32_t num, void *data);
    int32_t ResetSensors() const;

private:
    int32_t CreateSensorDataChannel();
    int32_t DestroySensorDataChannel();
    int32_t ConvertSensorInfos() const;
    void ClearSensorInfos() const;
    std::set<RecordSensorCallback> GetSubscribeUserCallback(int32_t sensorId);
    static std::recursive_mutex subscribeMutex_;
    static std::mutex chanelMutex_;
    OHOS::sptr<OHOS::Sensors::SensorDataChannel> dataChannel_ = nullptr;
    std::atomic_bool isChannelCreated_ = false;
    int64_t samplingInterval_ = -1;
    int64_t reportInterval_ = -1;
    std::map<int32_t, std::set<const SensorUser *>> subscribeMap_;
    std::map<int32_t, std::set<const SensorUser *>> unsubscribeMap_;
};

#define SENSOR_AGENT_IMPL OHOS::DelayedSingleton<SensorAgentProxy>::GetInstance()
}  // namespace Sensors
}  // namespace OHOS
#endif  // endif SENSOR_PROXY_H