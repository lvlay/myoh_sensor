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

#include "sensor_data_channel.h"

#include "errors.h"

#include "fd_listener.h"
#include "sensor_errors.h"
#include "sensor_file_descriptor_listener.h"

#undef LOG_TAG
#define LOG_TAG "SensorDataChannel"
namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;
using namespace OHOS::AppExecFwk;

int32_t SensorDataChannel::CreateSensorDataChannel(DataChannelCB callBack, void *data)
{
    CHKPR(callBack, SENSOR_NATIVE_REGSITER_CB_ERR);
    dataCB_ = callBack;
    privateData_ = data;
    return InnerSensorDataChannel();
}

int32_t SensorDataChannel::RestoreSensorDataChannel()
{
    CHKPR(dataCB_, SENSOR_NATIVE_REGSITER_CB_ERR);
    if (GetReceiveDataFd() != -1) {
        SEN_HILOGW("Restore sensor data channel failed, please destroy sensor data channel first.");
        return SENSOR_CHANNEL_RESTORE_FD_ERR;
    }
    return InnerSensorDataChannel();
}

int32_t SensorDataChannel::InnerSensorDataChannel()
{
    std::lock_guard<std::mutex> eventRunnerLock(eventRunnerMutex_);
    // create basic data channel
    int32_t ret = CreateSensorBasicChannel();
    if (ret != ERR_OK) {
        SEN_HILOGE("Create basic channel failed, ret:%{public}d", ret);
        return ret;
    }
    auto listener = std::make_shared<SensorFileDescriptorListener>();
    listener->SetChannel(this);
    if (eventHandler_ == nullptr) {
        auto myRunner = AppExecFwk::EventRunner::Create(true, AppExecFwk::ThreadMode::FFRT);
        CHKPR(myRunner, ERROR);
        eventHandler_ = std::make_shared<SensorEventHandler>(myRunner);
    }
    int32_t receiveFd = GetReceiveDataFd();
    auto inResult = eventHandler_->AddFileDescriptorListener(receiveFd,
        AppExecFwk::FILE_DESCRIPTOR_INPUT_EVENT, listener, "SensorTask");
    if (inResult != 0) {
        SEN_HILOGE("AddFileDescriptorListener fail");
        return ERROR;
    }
    auto pairRet = listenedFdSet_.insert(receiveFd);
    if (!pairRet.second) {
        SEN_HILOGE("ListenedFdSet insert fd fail, fd:%{public}d", receiveFd);
        return ERROR;
    }
    return ERR_OK;
}

int32_t SensorDataChannel::DestroySensorDataChannel()
{
    DelFdListener(GetReceiveDataFd());
    return DestroySensorBasicChannel();
}

SensorDataChannel::~SensorDataChannel()
{
    DestroySensorDataChannel();
}

int32_t SensorDataChannel::AddFdListener(int32_t fd, ReceiveMessageFun receiveMessage, DisconnectFun disconnect)
{
    receiveMessage_ = receiveMessage;
    disconnect_ = disconnect;
    std::lock_guard<std::mutex> eventRunnerLock(eventRunnerMutex_);
    if (eventHandler_ == nullptr) {
        auto myRunner = AppExecFwk::EventRunner::Create(true, AppExecFwk::ThreadMode::FFRT);
        CHKPR(myRunner, ERROR);
        eventHandler_ = std::make_shared<SensorEventHandler>(myRunner);
    }
    auto listener = std::make_shared<FdListener>();
    listener->SetChannel(this);
    auto errCode = eventHandler_->AddFileDescriptorListener(fd, AppExecFwk::FILE_DESCRIPTOR_INPUT_EVENT, listener,
        "SensorTask");
    if (errCode != ERR_OK) {
        SEN_HILOGE("Add fd listener failed, fd:%{public}d, errCode:%{public}u", fd, errCode);
        return ERROR;
    }
    auto pairRet = listenedFdSet_.insert(fd);
    if (!pairRet.second) {
        SEN_HILOGE("ListenedFdSet insert fd fail, fd:%{public}d", fd);
        return ERROR;
    }
    return ERR_OK;
}

int32_t SensorDataChannel::DelFdListener(int32_t fd)
{
    std::lock_guard<std::mutex> eventRunnerLock(eventRunnerMutex_);
    CHKPR(eventHandler_, ERROR);
    eventHandler_->RemoveFileDescriptorListener(fd);
    auto it = listenedFdSet_.find(fd);
    if (it == listenedFdSet_.end()) {
        SEN_HILOGE("ListenedFdSet not find fd, fd:%{public}d", fd);
        return ERROR;
    }
    listenedFdSet_.erase(it);
    if (listenedFdSet_.empty() && eventHandler_ != nullptr) {
        eventHandler_ = nullptr;
        SEN_HILOGD("Set eventHandler_ nullptr");
    }
    return ERR_OK;
}

ReceiveMessageFun SensorDataChannel::GetReceiveMessageFun() const
{
    return receiveMessage_;
}

DisconnectFun SensorDataChannel::GetDisconnectFun() const
{
    return disconnect_;
}
}  // namespace Sensors
}  // namespace OHOS
