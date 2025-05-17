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

#ifndef SENSOR_FILE_DESCRIPTOR_LISTENER_H
#define SENSOR_FILE_DESCRIPTOR_LISTENER_H

#include <cstdint>

#include "file_descriptor_listener.h"
#include "sensor_data_channel.h"

namespace OHOS {
namespace Sensors {
class SensorFileDescriptorListener : public AppExecFwk::FileDescriptorListener {
public:
    SensorFileDescriptorListener();
    ~SensorFileDescriptorListener();
    void OnReadable(int32_t fileDescriptor) override;
    void OnShutdown(int32_t fileDescriptor) override;
    void OnException(int32_t fileDescriptor) override;
    void SetChannel(SensorDataChannel *channel);

private:
    SensorDataChannel *channel_ = nullptr;
    SensorData *receiveDataBuff_ = nullptr;
};
}  // namespace Sensors
}  // namespace OHOS
#endif // SENSOR_FILE_DESCRIPTOR_LISTENER_H
