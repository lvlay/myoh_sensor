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

#ifndef SENSOR_DATA_CHANNEL_H
#define SENSOR_DATA_CHANNEL_H

#include <memory>
#include <thread>
#include "medical_sensor_basic_data_channel.h"
#include "medical_native_type.h"
#include "my_event_handler.h"

namespace OHOS {
namespace Sensors {
typedef void (*DataChannelCB)(struct SensorEvent *events, int32_t num, void *data);
class MedicalSensorDataChannel : public MedicalSensorBasicDataChannel {
public:
    MedicalSensorDataChannel();
    ~MedicalSensorDataChannel();
    static int32_t HandleEvent(int32_t fd, int32_t events, void *data);
    int32_t CreateSensorDataChannel(DataChannelCB callBack, void *data);
    int32_t DestroySensorDataChannel();
    bool IsThreadExit();
    bool IsThreadStart();
    int32_t RestoreSensorDataChannel();
    int32_t test = 10;
    DataChannelCB dataCB_;
    void *privateData_ = nullptr;

private:
    static void threadProcessTask(MedicalSensorDataChannel *afeChannel);
    int32_t InnerSensorDataChannel();
    std::mutex eventRunnerMutex_;
    static std::shared_ptr<MyEventHandler> eventHandler_;
    static std::shared_ptr<AppExecFwk::EventRunner> eventRunner_;
    static int32_t receiveFd_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // SENSOR_DATA_CHANNEL_H
