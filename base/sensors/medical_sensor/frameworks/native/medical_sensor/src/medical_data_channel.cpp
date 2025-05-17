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

#include "medical_sensor_data_channel.h"

#include <cerrno>
#include <unistd.h>

#include <fcntl.h>
#include <sys/socket.h>

#include "my_file_descriptor_listener.h"
#include "medical_errors.h"
#include "medical_log_domain.h"
#include "string_ex.h"

#ifndef O_NONBLOCK
# define O_NONBLOCK	  04000
#endif

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;
using namespace OHOS::AppExecFwk;
std::shared_ptr<MyEventHandler> MedicalSensorDataChannel::eventHandler_;
std::shared_ptr<AppExecFwk::EventRunner> MedicalSensorDataChannel::eventRunner_;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_FRAMEWORK, "MedicalSensorDataChannel" };
// max 100 data in cache buffer
constexpr int32_t SENSOR_READ_DATA_SIZE = sizeof(struct SensorEvent) * 100;
const uint32_t STOP_EVENT_ID = 0;
}  // namespace

MedicalSensorDataChannel::MedicalSensorDataChannel()
    : dataCB_(nullptr),
      privateData_(nullptr)
{}

int32_t MedicalSensorDataChannel::CreateSensorDataChannel(DataChannelCB callBack, void *data)
{
    if (callBack == nullptr) {
        HiLog::Error(LABEL, "%{public}s callBack cannot be null", __func__);
        return SENSOR_NATIVE_REGSITER_CB_ERR;
    }
    dataCB_ = callBack;
    privateData_ = data;
    return InnerSensorDataChannel();
}

int32_t MedicalSensorDataChannel::RestoreSensorDataChannel()
{
    if (dataCB_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s dataCB_ cannot be null", __func__);
        return SENSOR_CHANNEL_RESTORE_CB_ERR;
    }
    if (GetReceiveDataFd() != INVALID_FD) {
        HiLog::Error(LABEL, "%{public}s fd not close", __func__);
        return SENSOR_CHANNEL_RESTORE_FD_ERR;
    }
    return InnerSensorDataChannel();
}

int32_t MedicalSensorDataChannel::InnerSensorDataChannel()
{
    std::lock_guard<std::mutex> eventRunnerLock(eventRunnerMutex_);
    // create basic data channel
    int32_t ret = CreateSensorBasicChannel(SENSOR_READ_DATA_SIZE, SENSOR_READ_DATA_SIZE);
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s create basic channel failed, ret : %{public}d", __func__, ret);
        return ret;
    }
    auto listener = std::make_shared<MyFileDescriptorListener>();
    listener->SetChannel(this);
    auto myRunner = AppExecFwk::EventRunner::Create(true);
    if (myRunner == nullptr) {
        HiLog::Error(LABEL, "%{public}s myRunner is null", __func__);
        return -1;
    }
    auto handler = std::make_shared<MyEventHandler>(myRunner);
    if (handler == nullptr) {
        HiLog::Error(LABEL, "%{public}s handler is null", __func__);
        return -1;
    }

    int32_t receiveFd = GetReceiveDataFd();
    auto inResult = handler->AddFileDescriptorListener(receiveFd, AppExecFwk::FILE_DESCRIPTOR_INPUT_EVENT, listener,
        "MedicalSensorTask");
    if (inResult != 0) {
        HiLog::Error(LABEL, "%{public}s AddFileDescriptorListener fail", __func__);
        return -1;
    }
    eventHandler_ = handler;
    eventRunner_ = myRunner;
    int64_t delayTime = 100;
    int64_t param = 0;
    bool sendEventResult = eventHandler_->SendEvent(STOP_EVENT_ID, param, delayTime);
    if (!sendEventResult) {
        HiLog::Error(LABEL, "%{public}s EventHandler SendEvent fail", __func__);
        return -1;
    }
    int32_t runResult = eventRunner_->Run();
    if (!runResult) {
        HiLog::Error(LABEL, "%{public}s EventRunner run fail", __func__);
        return -1;
    }
    return ERR_OK;
}

int32_t MedicalSensorDataChannel::DestroySensorDataChannel()
{
    std::lock_guard<std::mutex> eventRunnerLock(eventRunnerMutex_);
    if (eventHandler_ == nullptr || eventRunner_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s handler or eventRunner is null", __func__);
        return -1;
    }
    int32_t receiveFd = GetReceiveDataFd();
    eventHandler_->RemoveFileDescriptorListener(receiveFd);
    eventHandler_ = nullptr;
    eventRunner_->Stop();
    eventRunner_ = nullptr;
    // destroy sensor basic channelx
    return DestroySensorBasicChannel();
}

MedicalSensorDataChannel::~MedicalSensorDataChannel()
{
    DestroySensorDataChannel();
}
}  // namespace Sensors
}  // namespace OHOS
