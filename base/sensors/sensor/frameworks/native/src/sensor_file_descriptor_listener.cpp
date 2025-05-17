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

#include "sensor_file_descriptor_listener.h"
#include "sensor_agent_type.h"
#include "sensor_basic_data_channel.h"
#include "sensor_errors.h"
#include "sys/socket.h"

#undef LOG_TAG
#define LOG_TAG "SensorFileDescriptorListener"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;
using namespace OHOS::AppExecFwk;

namespace {
constexpr int32_t RECEIVE_DATA_SIZE = 100;
}  // namespace

SensorFileDescriptorListener::SensorFileDescriptorListener()
{
    receiveDataBuff_ = new (std::nothrow) SensorData[RECEIVE_DATA_SIZE];
    CHKPL(receiveDataBuff_);
}

SensorFileDescriptorListener::~SensorFileDescriptorListener()
{
    CALL_LOG_ENTER;
    if (receiveDataBuff_ != nullptr) {
        delete[] receiveDataBuff_;
        receiveDataBuff_ = nullptr;
    }
}

void SensorFileDescriptorListener::OnReadable(int32_t fileDescriptor)
{
    if (fileDescriptor < 0) {
        SEN_HILOGE("fileDescriptor:%{public}d", fileDescriptor);
        return;
    }
    CHKPV(channel_);
    if (receiveDataBuff_ == nullptr) {
        SEN_HILOGE("Receive data buff_ is null");
        return;
    }
    int32_t len =
        recv(fileDescriptor, receiveDataBuff_, sizeof(SensorData) * RECEIVE_DATA_SIZE, 0);
    int32_t eventSize = static_cast<int32_t>(sizeof(SensorData));
    while (len > 0) {
        int32_t num = len / eventSize;
        for (int i = 0; i < num; i++) {
            SensorEvent event = {
                .sensorTypeId = receiveDataBuff_[i].sensorTypeId,
                .version = receiveDataBuff_[i].version,
                .timestamp = receiveDataBuff_[i].timestamp,
                .option = receiveDataBuff_[i].option,
                .mode = receiveDataBuff_[i].mode,
                .data = receiveDataBuff_[i].data,
                .dataLen = receiveDataBuff_[i].dataLen
            };
            channel_->dataCB_(&event, 1, channel_->privateData_);
        }
        len = recv(fileDescriptor, receiveDataBuff_, sizeof(SensorData) * RECEIVE_DATA_SIZE, 0);
    }
}

void SensorFileDescriptorListener::SetChannel(SensorDataChannel *channel)
{
    channel_ = channel;
}

void SensorFileDescriptorListener::OnShutdown(int32_t fileDescriptor)
{
    if (fileDescriptor < 0) {
        SEN_HILOGE("Invalid fd:%{public}d", fileDescriptor);
    }
    if (receiveDataBuff_ != nullptr) {
        delete[] receiveDataBuff_;
        receiveDataBuff_ = nullptr;
    }
    CHKPV(channel_);
    channel_->DestroySensorDataChannel();
}

void SensorFileDescriptorListener::OnException(int32_t fileDescriptor)
{
    if (fileDescriptor < 0) {
        SEN_HILOGE("Invalid fd:%{public}d", fileDescriptor);
    }
    if (receiveDataBuff_ != nullptr) {
        delete[] receiveDataBuff_;
        receiveDataBuff_ = nullptr;
    }
    CHKPV(channel_);
    channel_->DestroySensorDataChannel();
}
}  // namespace Sensors
}  // namespace OHOS