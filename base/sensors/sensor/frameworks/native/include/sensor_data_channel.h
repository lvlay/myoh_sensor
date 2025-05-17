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

#ifndef SENSOR_DATA_CHANNEL_H
#define SENSOR_DATA_CHANNEL_H

#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_set>

#include "sensor_agent_type.h"
#include "sensor_basic_data_channel.h"
#include "sensor_event_handler.h"

namespace OHOS {
namespace Sensors {
using DataChannelCB = std::function<void(SensorEvent *, int32_t, void *)>;
using ReceiveMessageFun = std::function<void(const char *, size_t)>;
using DisconnectFun = std::function<void()>;
class SensorDataChannel : public SensorBasicDataChannel {
public:
    SensorDataChannel() = default;
    ~SensorDataChannel();
    int32_t CreateSensorDataChannel(DataChannelCB callBack, void *data);
    int32_t DestroySensorDataChannel();
    int32_t RestoreSensorDataChannel();
    DataChannelCB dataCB_ = nullptr;
    void *privateData_ = nullptr;
    int32_t AddFdListener(int32_t fd, ReceiveMessageFun receiveMessage, DisconnectFun disconnect);
    int32_t DelFdListener(int32_t fd);
    ReceiveMessageFun GetReceiveMessageFun() const;
    DisconnectFun GetDisconnectFun() const;

private:
    int32_t InnerSensorDataChannel();
    std::mutex eventRunnerMutex_;
    std::shared_ptr<SensorEventHandler> eventHandler_ = nullptr;
    std::unordered_set<int32_t> listenedFdSet_;
    ReceiveMessageFun receiveMessage_;
    DisconnectFun disconnect_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif // SENSOR_DATA_CHANNEL_H
