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

#include "sensor_service_client.h"

#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "death_recipient_template.h"
#include "hisysevent.h"
#include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "sensor_agent_proxy.h"
#include "sensor_errors.h"
#include "sensor_service_proxy.h"
#include "system_ability_definition.h"
#include "rust_binding.h"

#undef LOG_TAG
#define LOG_TAG "SensorServiceClient"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

namespace {
constexpr int32_t GET_SERVICE_MAX_COUNT = 3;
constexpr uint32_t WAIT_MS = 200;
#ifdef OHOS_BUILD_ENABLE_RUST
extern "C" {
    void ReadClientPackets(RustStreamBuffer *, OHOS::Sensors::SensorServiceClient *,
        void(*)(OHOS::Sensors::SensorServiceClient *, RustNetPacket *));
    void OnPacket(SensorServiceClient *object, RustNetPacket *cPkt)
    {
        NetPacket pkt(cPkt->msgId);
        pkt.streamBufferPtr_.reset(cPkt->streamBuffer);
        object->HandleNetPacke(pkt);
    }
}
#endif // OHOS_BUILD_ENABLE_RUST
}  // namespace

SensorServiceClient::~SensorServiceClient()
{
    if (sensorServer_ != nullptr && serviceDeathObserver_ != nullptr) {
        auto remoteObject = sensorServer_->AsObject();
        if (remoteObject != nullptr) {
            remoteObject->RemoveDeathRecipient(serviceDeathObserver_);
        }
    }
    Disconnect();
}

int32_t SensorServiceClient::InitServiceClient()
{
    CALL_LOG_ENTER;
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    if (sensorServer_ != nullptr) {
        SEN_HILOGD("Already init");
        return ERR_OK;
    }
    if (sensorClientStub_ == nullptr) {
        sensorClientStub_ = new (std::nothrow) SensorClientStub();
    }
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHKPR(systemAbilityManager, SENSOR_NATIVE_SAM_ERR);
    int32_t retry = 0;
    while (retry < GET_SERVICE_MAX_COUNT) {
        auto object = systemAbilityManager->GetSystemAbility(SENSOR_SERVICE_ABILITY_ID);
        CHKPR(object, SENSOR_NATIVE_GET_SERVICE_ERR);
        sensorServer_ = iface_cast<ISensorService>(object);
        if (sensorServer_ != nullptr) {
            SEN_HILOGD("Get service success, retry:%{public}d", retry);
            serviceDeathObserver_ = new (std::nothrow) DeathRecipientTemplate(*const_cast<SensorServiceClient *>(this));
            CHKPR(serviceDeathObserver_, SENSOR_NATIVE_GET_SERVICE_ERR);
            auto remoteObject = sensorServer_->AsObject();
            CHKPR(remoteObject, SENSOR_NATIVE_GET_SERVICE_ERR);
            remoteObject->AddDeathRecipient(serviceDeathObserver_);
            sensorList_ = sensorServer_->GetSensorList();
            return ERR_OK;
        }
        SEN_HILOGW("Get service failed, retry:%{public}d", retry);
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
        retry++;
    }
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::SENSOR, "SERVICE_EXCEPTION",
        HiSysEvent::EventType::FAULT, "PKG_NAME", "InitServiceClient", "ERROR_CODE", SENSOR_NATIVE_GET_SERVICE_ERR);
    SEN_HILOGE("Get service failed");
    return SENSOR_NATIVE_GET_SERVICE_ERR;
}

bool SensorServiceClient::IsValid(int32_t sensorId)
{
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        SEN_HILOGE("InitServiceClient failed, ret:%{public}d", ret);
        return false;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    if (sensorList_.empty()) {
        SEN_HILOGE("sensorList_ cannot be empty");
        return false;
    }
    for (auto &sensor : sensorList_) {
        if (sensor.GetSensorId() == sensorId) {
            return true;
        }
    }
    return false;
}

int32_t SensorServiceClient::EnableSensor(int32_t sensorId, int64_t samplingPeriod, int64_t maxReportDelay)
{
    CALL_LOG_ENTER;
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        SEN_HILOGE("InitServiceClient failed, ret:%{public}d", ret);
        return ret;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    CHKPR(sensorServer_, ERROR);
    StartTrace(HITRACE_TAG_SENSORS, "EnableSensor");
    ret = sensorServer_->EnableSensor(sensorId, samplingPeriod, maxReportDelay);
    FinishTrace(HITRACE_TAG_SENSORS);
    if (ret == ERR_OK) {
        UpdateSensorInfoMap(sensorId, samplingPeriod, maxReportDelay);
    }
    return ret;
}

int32_t SensorServiceClient::DisableSensor(int32_t sensorId)
{
    CALL_LOG_ENTER;
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        SEN_HILOGE("InitServiceClient failed, ret:%{public}d", ret);
        return ret;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    CHKPR(sensorServer_, ERROR);
    StartTrace(HITRACE_TAG_SENSORS, "DisableSensor");
    ret = sensorServer_->DisableSensor(sensorId);
    FinishTrace(HITRACE_TAG_SENSORS);
    if (ret == ERR_OK) {
        DeleteSensorInfoItem(sensorId);
    }
    return ret;
}

std::vector<Sensor> SensorServiceClient::GetSensorList()
{
    CALL_LOG_ENTER;
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        SEN_HILOGE("InitServiceClient failed, ret:%{public}d", ret);
        return {};
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    if (sensorList_.empty()) {
        SEN_HILOGE("sensorList_ cannot be empty");
    }
    return sensorList_;
}

int32_t SensorServiceClient::TransferDataChannel(sptr<SensorDataChannel> sensorDataChannel)
{
    CALL_LOG_ENTER;
    CHKPR(sensorDataChannel, INVALID_POINTER);
    {
        std::lock_guard<std::mutex> channelLock(channelMutex_);
        dataChannel_ = sensorDataChannel;
    }
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        SEN_HILOGE("InitServiceClient failed, ret:%{public}d", ret);
        return ret;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    CHKPR(sensorServer_, ERROR);
    StartTrace(HITRACE_TAG_SENSORS, "TransferDataChannel");
    CHKPR(sensorClientStub_, INVALID_POINTER);
    auto remoteObject = sensorClientStub_->AsObject();
    CHKPR(remoteObject, INVALID_POINTER);
    ret = sensorServer_->TransferDataChannel(sensorDataChannel, remoteObject);
    FinishTrace(HITRACE_TAG_SENSORS);
    return ret;
}

int32_t SensorServiceClient::DestroyDataChannel()
{
    CALL_LOG_ENTER;
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        SEN_HILOGE("InitServiceClient failed, ret:%{public}d", ret);
        return ret;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    CHKPR(sensorServer_, ERROR);
    StartTrace(HITRACE_TAG_SENSORS, "DestroyDataChannel");
    CHKPR(sensorClientStub_, INVALID_POINTER);
    auto remoteObject = sensorClientStub_->AsObject();
    CHKPR(remoteObject, INVALID_POINTER);
    ret = sensorServer_->DestroySensorChannel(remoteObject);
    FinishTrace(HITRACE_TAG_SENSORS);
    return ret;
}

void SensorServiceClient::ReenableSensor()
{
    CALL_LOG_ENTER;
    std::lock_guard<std::mutex> mapLock(mapMutex_);
    for (const auto &it : sensorInfoMap_) {
        if (sensorServer_ != nullptr) {
            sensorServer_->EnableSensor(it.first, it.second.GetSamplingPeriodNs(), it.second.GetMaxReportDelayNs());
        }
    }
    if (!isConnected_) {
        SEN_HILOGD("Previous socket channel status is false, not need retry creat socket channel");
        return;
    }
    Disconnect();
    CreateSocketChannel();
}

void SensorServiceClient::ProcessDeathObserver(const wptr<IRemoteObject> &object)
{
    CALL_LOG_ENTER;
    {
        std::lock_guard<std::mutex> channelLock(channelMutex_);
        if (dataChannel_ == nullptr) {
            SEN_HILOGI("dataChannel_ is nullptr");
            {
                std::lock_guard<std::mutex> clientLock(clientMutex_);
                sensorList_.clear();
                sensorServer_ = nullptr;
            }
            if (InitServiceClient() != ERR_OK) {
                SEN_HILOGE("InitServiceClient failed");
                return;
            }
        } else {
            SEN_HILOGI("dataChannel_ is not nullptr");
            dataChannel_->DestroySensorDataChannel();
            dataChannel_->RestoreSensorDataChannel();
            {
                std::lock_guard<std::mutex> clientLock(clientMutex_);
                sensorList_.clear();
                sensorServer_ = nullptr;
            }
            if (InitServiceClient() != ERR_OK) {
                SEN_HILOGE("InitServiceClient failed");
                dataChannel_->DestroySensorDataChannel();
                SENSOR_AGENT_IMPL->SetIsChannelCreated(false);
                return;
            }
            if (sensorServer_ != nullptr && sensorClientStub_ != nullptr) {
                auto remoteObject = sensorClientStub_->AsObject();
                if (remoteObject != nullptr) {
                    sensorServer_->TransferDataChannel(dataChannel_, remoteObject);
                }
            }
        }
    }
    ReenableSensor();
}

void SensorServiceClient::UpdateSensorInfoMap(int32_t sensorId, int64_t samplingPeriod, int64_t maxReportDelay)
{
    CALL_LOG_ENTER;
    SensorBasicInfo sensorInfo;
    sensorInfo.SetSamplingPeriodNs(samplingPeriod);
    sensorInfo.SetMaxReportDelayNs(maxReportDelay);
    sensorInfo.SetSensorState(true);
    std::lock_guard<std::mutex> mapLock(mapMutex_);
    sensorInfoMap_[sensorId] = sensorInfo;
    return;
}

void SensorServiceClient::DeleteSensorInfoItem(int32_t sensorId)
{
    CALL_LOG_ENTER;
    std::lock_guard<std::mutex> mapLock(mapMutex_);
    auto it = sensorInfoMap_.find(sensorId);
    if (it != sensorInfoMap_.end()) {
        sensorInfoMap_.erase(it);
    }
    return;
}

int32_t SensorServiceClient::SuspendSensors(int32_t pid)
{
    CALL_LOG_ENTER;
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        SEN_HILOGE("InitServiceClient failed, ret:%{public}d", ret);
        return ret;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    CHKPR(sensorServer_, ERROR);
    StartTrace(HITRACE_TAG_SENSORS, "SuspendSensors");
    ret = sensorServer_->SuspendSensors(pid);
    FinishTrace(HITRACE_TAG_SENSORS);
    return ret;
}

int32_t SensorServiceClient::ResumeSensors(int32_t pid)
{
    CALL_LOG_ENTER;
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        SEN_HILOGE("InitServiceClient failed, ret:%{public}d", ret);
        return ret;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    CHKPR(sensorServer_, ERROR);
    StartTrace(HITRACE_TAG_SENSORS, "ResumeSensors");
    ret = sensorServer_->ResumeSensors(pid);
    FinishTrace(HITRACE_TAG_SENSORS);
    return ret;
}

int32_t SensorServiceClient::GetActiveInfoList(int32_t pid, std::vector<ActiveInfo> &activeInfoList)
{
    CALL_LOG_ENTER;
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        SEN_HILOGE("InitServiceClient failed, ret:%{public}d", ret);
        return ret;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    CHKPR(sensorServer_, ERROR);
    StartTrace(HITRACE_TAG_SENSORS, "GetActiveInfoList");
    ret = sensorServer_->GetActiveInfoList(pid, activeInfoList);
    FinishTrace(HITRACE_TAG_SENSORS);
    return ret;
}

int32_t SensorServiceClient::Register(SensorActiveInfoCB callback, sptr<SensorDataChannel> sensorDataChannel)
{
    CALL_LOG_ENTER;
    if (!isConnected_) {
        CHKPR(sensorDataChannel, INVALID_POINTER);
        {
            std::lock_guard<std::mutex> channelLock(channelMutex_);
            dataChannel_ = sensorDataChannel;
        }
        int32_t ret = CreateSocketChannel();
        if (ret != ERR_OK) {
            SEN_HILOGE("Register sensor active info callback failed, ret:%{public}d", ret);
            return ret;
        }
    }
    std::lock_guard<std::mutex> activeInfoCBLock(activeInfoCBMutex_);
    activeInfoCBSet_.insert(callback);
    return ERR_OK;
}

int32_t SensorServiceClient::Unregister(SensorActiveInfoCB callback)
{
    CALL_LOG_ENTER;
    {
        std::lock_guard<std::mutex> activeInfoCBLock(activeInfoCBMutex_);
        activeInfoCBSet_.erase(callback);
        if (!activeInfoCBSet_.empty()) {
            return ERR_OK;
        }
    }
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        SEN_HILOGE("InitServiceClient failed, ret:%{public}d", ret);
        return ret;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    CHKPR(sensorServer_, ERROR);
    StartTrace(HITRACE_TAG_SENSORS, "DisableActiveInfoCB");
    ret = sensorServer_->DisableActiveInfoCB();
    FinishTrace(HITRACE_TAG_SENSORS);
    if (ret != ERR_OK) {
        SEN_HILOGE("Disable active info callback failed, ret:%{public}d", ret);
        return ret;
    }
    Disconnect();
    StartTrace(HITRACE_TAG_SENSORS, "DestroySocketChannel");
    CHKPR(sensorClientStub_, INVALID_POINTER);
    auto remoteObject = sensorClientStub_->AsObject();
    CHKPR(remoteObject, INVALID_POINTER);
    ret = sensorServer_->DestroySocketChannel(remoteObject);
    FinishTrace(HITRACE_TAG_SENSORS);
    if (ret != ERR_OK) {
        SEN_HILOGE("Destroy socket channel failed, ret:%{public}d", ret);
        return ret;
    }
    isConnected_ = false;
    return ERR_OK;
}

int32_t SensorServiceClient::ResetSensors()
{
    CALL_LOG_ENTER;
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        SEN_HILOGE("InitServiceClient failed, ret:%{public}d", ret);
        return ret;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    CHKPR(sensorServer_, ERROR);
    StartTrace(HITRACE_TAG_SENSORS, "ResetSensors");
    ret = sensorServer_->ResetSensors();
    FinishTrace(HITRACE_TAG_SENSORS);
    return ret;
}

void SensorServiceClient::ReceiveMessage(const char *buf, size_t size)
{
    CHKPV(buf);
    if (size == 0 || size > MAX_PACKET_BUF_SIZE) {
        SEN_HILOGE("Invalid input param size. size:%{public}zu", size);
        return;
    }
    if (!circBuf_.Write(buf, size)) {
        SEN_HILOGE("Write data failed. size:%{public}zu", size);
    }
#ifdef OHOS_BUILD_ENABLE_RUST
    ReadClientPackets(circBuf_.streamBufferPtr_.get(), this, OnPacket);
#else
    OnReadPackets(circBuf_, [this] (NetPacket &pkt) { this->HandleNetPacke(pkt); });
#endif // OHOS_BUILD_ENABLE_RUST
}

void SensorServiceClient::HandleNetPacke(NetPacket &pkt)
{
    auto id = pkt.GetMsgId();
    if (id != MessageId::ACTIVE_INFO) {
        SEN_HILOGE("NetPacke message id is not ACTIVE_INFO");
        return;
    }
    SensorActiveInfo sensorActiveInfo;
    pkt >> sensorActiveInfo.pid >> sensorActiveInfo.sensorId >> sensorActiveInfo.samplingPeriodNs >>
        sensorActiveInfo.maxReportDelayNs;
#ifdef OHOS_BUILD_ENABLE_RUST
    if (StreamBufferChkRWError(pkt.streamBufferPtr_.get())) {
#else
    if (pkt.ChkRWError()) {
#endif // OHOS_BUILD_ENABLE_RUST
        SEN_HILOGE("Packet read type failed");
        return;
    }
    std::lock_guard<std::mutex> activeInfoCBLock(activeInfoCBMutex_);
    for (auto callback : activeInfoCBSet_) {
        if (callback != nullptr) {
            callback(sensorActiveInfo);
        }
    }
}

void SensorServiceClient::Disconnect()
{
    CALL_LOG_ENTER;
    int32_t fd = GetFd();
    if (fd < 0) {
        return;
    }
    {
        std::lock_guard<std::mutex> channelLock(channelMutex_);
        if (dataChannel_ != nullptr) {
            int32_t ret = dataChannel_->DelFdListener(fd);
            if (ret != ERR_OK) {
                SEN_HILOGE("Delete fd listener failed, fd:%{public}d, ret:%{public}d", fd, ret);
            }
        }
    }
    Close();
}

int32_t SensorServiceClient::CreateSocketChannel()
{
    CALL_LOG_ENTER;
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        SEN_HILOGE("InitServiceClient failed, ret:%{public}d", ret);
        return ret;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    CHKPR(sensorServer_, ERROR);
    int32_t clientFd = -1;
    StartTrace(HITRACE_TAG_SENSORS, "CreateSocketChannel");
    CHKPR(sensorClientStub_, INVALID_POINTER);
    auto remoteObject = sensorClientStub_->AsObject();
    CHKPR(remoteObject, INVALID_POINTER);
    ret = sensorServer_->CreateSocketChannel(remoteObject, clientFd);
    FinishTrace(HITRACE_TAG_SENSORS);
    if (ret != ERR_OK || clientFd < 0) {
        Close();
        SEN_HILOGE("Create socket channel failed, ret:%{public}d", ret);
        return ret;
    }
#ifdef OHOS_BUILD_ENABLE_RUST
    StreamSocketSetFd(streamSocketPtr_.get(), clientFd);
#else
    fd_ = clientFd;
#endif // OHOS_BUILD_ENABLE_RUST
    {
        std::lock_guard<std::mutex> channelLock(channelMutex_);
        if (dataChannel_->AddFdListener(GetFd(),
            [this] (const char *buf, size_t size) { this->ReceiveMessage(buf, size); },
            [this] { this->Disconnect(); })!= ERR_OK) {
            Close();
            SEN_HILOGE("Add fd listener failed, fd:%{public}d", GetFd());
            return ERROR;
        }
    }
    StartTrace(HITRACE_TAG_SENSORS, "EnableActiveInfoCB");
    ret = sensorServer_->EnableActiveInfoCB();
    FinishTrace(HITRACE_TAG_SENSORS);
    if (ret != ERR_OK) {
        SEN_HILOGE("Enable active info callback failed, ret:%{public}d", ret);
        Disconnect();
        return ret;
    }
    isConnected_ = true;
    return ERR_OK;
}
}  // namespace Sensors
}  // namespace OHOS
