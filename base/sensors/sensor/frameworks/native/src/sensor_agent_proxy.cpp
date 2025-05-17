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

#include "sensor_agent_proxy.h"

#include <cstring>

#include "print_sensor_data.h"
#include "securec.h"
#include "sensor_errors.h"
#include "sensor_service_client.h"
#undef LOG_TAG
#define LOG_TAG "SensorAgentProxy"
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace Sensors {
namespace {
constexpr uint32_t MAX_SENSOR_LIST_SIZE = 0Xffff;
std::mutex sensorInfoMutex_;
SensorInfo *sensorInfos_ = nullptr;
std::mutex sensorActiveInfoMutex_;
SensorActiveInfo *sensorActiveInfos_ = nullptr;
int32_t sensorInfoCount_ = 0;
}  // namespace

#define SEN_CLIENT SensorServiceClient::GetInstance()
std::recursive_mutex SensorAgentProxy::subscribeMutex_;
std::mutex SensorAgentProxy::chanelMutex_;

SensorAgentProxy::SensorAgentProxy()
    : dataChannel_(new (std::nothrow) SensorDataChannel())
{}

SensorAgentProxy::~SensorAgentProxy()
{
    CALL_LOG_ENTER;
    ClearSensorInfos();
}

std::set<RecordSensorCallback> SensorAgentProxy::GetSubscribeUserCallback(int32_t sensorId)
{
    std::lock_guard<std::recursive_mutex> subscribeLock(subscribeMutex_);
    auto iter = subscribeMap_.find(sensorId);
    if (iter == subscribeMap_.end()) {
        SEN_HILOGE("Sensor is not subscribed");
        return {};
    }
    std::set<RecordSensorCallback> callback;
    for (const auto &it : iter->second) {
        auto ret = callback.insert(it->callback);
        if (!ret.second) {
            SEN_HILOGE("callback insert fail");
        }
    }
    return callback;
}

void SensorAgentProxy::HandleSensorData(SensorEvent *events,
    int32_t num, void *data) __attribute__((no_sanitize("cfi")))
{
    CHKPV(events);
    if (num <= 0) {
        SEN_HILOGE("events is null or num is invalid");
        return;
    }
    SensorEvent eventStream;
    for (int32_t i = 0; i < num; ++i) {
        eventStream = events[i];
        auto callbacks = GetSubscribeUserCallback(eventStream.sensorTypeId);
        for (const auto &callback : callbacks) {
            CHKPV(callback);
            callback(&eventStream);
            PrintSensorData::GetInstance().ControlSensorClientPrint(callback, eventStream);
        }
    }
}

void SensorAgentProxy::SetIsChannelCreated(bool isChannelCreated)
{
    CALL_LOG_ENTER;
    std::lock_guard<std::mutex> chanelLock(chanelMutex_);
    isChannelCreated_ = isChannelCreated;
}

int32_t SensorAgentProxy::CreateSensorDataChannel()
{
    CALL_LOG_ENTER;
    std::lock_guard<std::mutex> chanelLock(chanelMutex_);
    if (isChannelCreated_) {
        SEN_HILOGI("The channel has already been created");
        return ERR_OK;
    }
    CHKPR(dataChannel_, INVALID_POINTER);
    auto ret = dataChannel_->CreateSensorDataChannel([this] (SensorEvent *events, int32_t num, void *data) {
        this->HandleSensorData(events, num, data);
    }, nullptr);
    if (ret != ERR_OK) {
        SEN_HILOGE("Create data channel failed, ret:%{public}d", ret);
        return ret;
    }
    ret = SEN_CLIENT.TransferDataChannel(dataChannel_);
    if (ret != ERR_OK) {
        auto destroyRet = dataChannel_->DestroySensorDataChannel();
        SEN_HILOGE("Transfer data channel failed, ret:%{public}d, destroyRet:%{public}d", ret, destroyRet);
        return ret;
    }
    isChannelCreated_ = true;
    return ERR_OK;
}

int32_t SensorAgentProxy::DestroySensorDataChannel()
{
    CALL_LOG_ENTER;
    std::lock_guard<std::mutex> chanelLock(chanelMutex_);
    if (!isChannelCreated_) {
        SEN_HILOGI("Channel has been destroyed");
        return ERR_OK;
    }
    CHKPR(dataChannel_, INVALID_POINTER);
    int32_t ret = dataChannel_->DestroySensorDataChannel();
    if (ret != ERR_OK) {
        SEN_HILOGE("Destroy data channel failed, ret:%{public}d", ret);
        return ret;
    }
    ret = SEN_CLIENT.DestroyDataChannel();
    if (ret != ERR_OK) {
        SEN_HILOGE("Destroy service data channel fail, ret:%{public}d", ret);
        return ret;
    }
    isChannelCreated_ = false;
    return ERR_OK;
}

int32_t SensorAgentProxy::ActivateSensor(int32_t sensorId, const SensorUser *user)
{
    CHKPR(user, OHOS::Sensors::ERROR);
    CHKPR(user->callback, OHOS::Sensors::ERROR);
    if (samplingInterval_ < 0 || reportInterval_ < 0) {
        SEN_HILOGE("SamplingPeriod or reportInterval_ is invalid");
        return ERROR;
    }
    if (!SEN_CLIENT.IsValid(sensorId)) {
        SEN_HILOGE("sensorId is invalid, %{public}d", sensorId);
        return PARAMETER_ERROR;
    }
    std::lock_guard<std::recursive_mutex> subscribeLock(subscribeMutex_);
    if (subscribeMap_.find(sensorId) == subscribeMap_.end()) {
        SEN_HILOGE("Subscribe sensorId first");
        return ERROR;
    }
    auto& subscribeSet = subscribeMap_[sensorId];
    if (subscribeSet.find(user) == subscribeSet.end()) {
        SEN_HILOGE("Subscribe user first");
        return ERROR;
    }
    int32_t ret = SEN_CLIENT.EnableSensor(sensorId, samplingInterval_, reportInterval_);
    samplingInterval_ = -1;
    reportInterval_ = -1;
    if (ret != 0) {
        SEN_HILOGE("Enable sensor failed, ret:%{public}d", ret);
        subscribeSet.erase(user);
        if (subscribeSet.empty()) {
            subscribeMap_.erase(sensorId);
        }
        return ret;
    }
    return ret;
}

int32_t SensorAgentProxy::DeactivateSensor(int32_t sensorId, const SensorUser *user)
{
    CHKPR(user, OHOS::Sensors::ERROR);
    CHKPR(user->callback, OHOS::Sensors::ERROR);
    if (!SEN_CLIENT.IsValid(sensorId)) {
        SEN_HILOGE("sensorId is invalid, %{public}d", sensorId);
        return PARAMETER_ERROR;
    }
    std::lock_guard<std::recursive_mutex> subscribeLock(subscribeMutex_);
    if (subscribeMap_.find(sensorId) == subscribeMap_.end()) {
        SEN_HILOGE("Subscribe sensorId first");
        return OHOS::Sensors::ERROR;
    }
    auto& subscribeSet = subscribeMap_[sensorId];
    if (subscribeSet.find(user) == subscribeSet.end()) {
        SEN_HILOGE("Subscribe user first");
        return OHOS::Sensors::ERROR;
    }
    auto status = unsubscribeMap_[sensorId].insert(user);
    if (!status.second) {
        SEN_HILOGD("User has been unsubscribed");
    }
    subscribeSet.erase(user);
    if (subscribeSet.empty()) {
        subscribeMap_.erase(sensorId);
        int32_t ret = SEN_CLIENT.DisableSensor(sensorId);
        if (ret != 0) {
            SEN_HILOGE("DisableSensor failed, ret:%{public}d", ret);
            return ret;
        }
    }
    return OHOS::Sensors::SUCCESS;
}

int32_t SensorAgentProxy::SetBatch(int32_t sensorId, const SensorUser *user, int64_t samplingInterval,
                                   int64_t reportInterval)
{
    CHKPR(user, OHOS::Sensors::ERROR);
    if (!SEN_CLIENT.IsValid(sensorId)) {
        SEN_HILOGE("sensorId is invalid, %{public}d", sensorId);
        return PARAMETER_ERROR;
    }
    if (samplingInterval < 0 || reportInterval < 0) {
        SEN_HILOGE("samplingInterval or reportInterval is invalid");
        return OHOS::Sensors::ERROR;
    }
    std::lock_guard<std::recursive_mutex> subscribeLock(subscribeMutex_);
    if (subscribeMap_.find(sensorId) == subscribeMap_.end()) {
        SEN_HILOGE("Subscribe sensorId first");
        return OHOS::Sensors::ERROR;
    }
    auto& subscribeSet = subscribeMap_[sensorId];
    if (subscribeSet.find(user) == subscribeSet.end()) {
        SEN_HILOGE("Subscribe user first");
        return OHOS::Sensors::ERROR;
    }
    samplingInterval_ = samplingInterval;
    reportInterval_ = reportInterval;
    return OHOS::Sensors::SUCCESS;
}

int32_t SensorAgentProxy::SubscribeSensor(int32_t sensorId, const SensorUser *user)
{
    SEN_HILOGI("In, sensorId:%{public}d", sensorId);
    CHKPR(user, OHOS::Sensors::ERROR);
    CHKPR(user->callback, OHOS::Sensors::ERROR);
    if (!SEN_CLIENT.IsValid(sensorId)) {
        SEN_HILOGE("sensorId is invalid, %{public}d", sensorId);
        return PARAMETER_ERROR;
    }
    int32_t ret = CreateSensorDataChannel();
    if (ret != ERR_OK) {
        SEN_HILOGE("Create sensor data chanel failed");
        return OHOS::Sensors::ERROR;
    }
    std::lock_guard<std::recursive_mutex> subscribeLock(subscribeMutex_);
    auto status = subscribeMap_[sensorId].insert(user);
    if (!status.second) {
        SEN_HILOGD("User has been subscribed");
    }
    if (PrintSensorData::GetInstance().IsContinuousType(sensorId)) {
        PrintSensorData::GetInstance().SavePrintUserInfo(user->callback);
    }
    return OHOS::Sensors::SUCCESS;
}

int32_t SensorAgentProxy::UnsubscribeSensor(int32_t sensorId, const SensorUser *user)
{
    SEN_HILOGI("In, sensorId:%{public}d", sensorId);
    CHKPR(user, OHOS::Sensors::ERROR);
    CHKPR(user->callback, OHOS::Sensors::ERROR);
    if (!SEN_CLIENT.IsValid(sensorId)) {
        SEN_HILOGE("sensorId is invalid, %{public}d", sensorId);
        return PARAMETER_ERROR;
    }
    std::lock_guard<std::recursive_mutex> subscribeLock(subscribeMutex_);
    if (unsubscribeMap_.find(sensorId) == unsubscribeMap_.end()) {
        SEN_HILOGE("Deactivate sensorId first");
        return OHOS::Sensors::ERROR;
    }
    auto& unsubscribeSet = unsubscribeMap_[sensorId];
    if (unsubscribeSet.find(user) == unsubscribeSet.end()) {
        SEN_HILOGE("Deactivate user first");
        return OHOS::Sensors::ERROR;
    }
    if (subscribeMap_.empty()) {
        int32_t ret = DestroySensorDataChannel();
        if (ret != ERR_OK) {
            SEN_HILOGE("Destroy data channel fail, ret:%{public}d", ret);
            return ret;
        }
    }
    unsubscribeSet.erase(user);
    if (unsubscribeSet.empty()) {
        unsubscribeMap_.erase(sensorId);
    }
    if (PrintSensorData::GetInstance().IsContinuousType(sensorId)) {
        PrintSensorData::GetInstance().RemovePrintUserInfo(user->callback);
    }
    return OHOS::Sensors::SUCCESS;
}

int32_t SensorAgentProxy::SetMode(int32_t sensorId, const SensorUser *user, int32_t mode)
{
    CHKPR(user, OHOS::Sensors::ERROR);
    CHKPR(user->callback, OHOS::Sensors::ERROR);
    if (!SEN_CLIENT.IsValid(sensorId)) {
        SEN_HILOGE("sensorId is invalid, %{public}d", sensorId);
        return ERROR;
    }
    std::lock_guard<std::recursive_mutex> subscribeLock(subscribeMutex_);
    if (subscribeMap_.find(sensorId) == subscribeMap_.end()) {
        SEN_HILOGE("Subscribe sensorId first");
        return OHOS::Sensors::ERROR;
    }
    auto& subscribeSet = subscribeMap_[sensorId];
    if (subscribeSet.find(user) == subscribeSet.end()) {
        SEN_HILOGE("Subscribe user first");
        return OHOS::Sensors::ERROR;
    }
    return OHOS::Sensors::SUCCESS;
}

void SensorAgentProxy::ClearSensorInfos() const
{
    if (sensorActiveInfos_ != nullptr) {
        free(sensorActiveInfos_);
        sensorActiveInfos_ = nullptr;
    }
    CHKPV(sensorInfos_);
    free(sensorInfos_);
    sensorInfos_ = nullptr;
}

int32_t SensorAgentProxy::ConvertSensorInfos() const
{
    CALL_LOG_ENTER;
    std::vector<Sensor> sensorList = SEN_CLIENT.GetSensorList();
    if (sensorList.empty()) {
        SEN_HILOGE("Get sensor lists failed");
        return ERROR;
    }
    size_t count = sensorList.size();
    if (count > MAX_SENSOR_LIST_SIZE) {
        SEN_HILOGE("The number of sensors exceeds the maximum value");
        return ERROR;
    }
    sensorInfos_ = (SensorInfo *)malloc(sizeof(SensorInfo) * count);
    CHKPR(sensorInfos_, ERROR);
    for (size_t i = 0; i < count; ++i) {
        SensorInfo *sensorInfo = sensorInfos_ + i;
        errno_t ret = strcpy_s(sensorInfo->sensorName, NAME_MAX_LEN,
            sensorList[i].GetSensorName().c_str());
        CHKCR(ret == EOK, ERROR);
        ret = strcpy_s(sensorInfo->vendorName, NAME_MAX_LEN,
            sensorList[i].GetVendorName().c_str());
        CHKCR(ret == EOK, ERROR);
        ret = strcpy_s(sensorInfo->hardwareVersion, VERSION_MAX_LEN,
            sensorList[i].GetHardwareVersion().c_str());
        CHKCR(ret == EOK, ERROR);
        ret = strcpy_s(sensorInfo->firmwareVersion, VERSION_MAX_LEN,
            sensorList[i].GetFirmwareVersion().c_str());
        CHKCR(ret == EOK, ERROR);
        sensorInfo->sensorId = sensorList[i].GetSensorId();
        sensorInfo->sensorTypeId = sensorList[i].GetSensorTypeId();
        sensorInfo->maxRange = sensorList[i].GetMaxRange();
        sensorInfo->precision = sensorList[i].GetResolution();
        sensorInfo->power = sensorList[i].GetPower();
        sensorInfo->minSamplePeriod = sensorList[i].GetMinSamplePeriodNs();
        sensorInfo->maxSamplePeriod = sensorList[i].GetMaxSamplePeriodNs();
    }
    sensorInfoCount_ = static_cast<int32_t>(count);
    return SUCCESS;
}

int32_t SensorAgentProxy::GetAllSensors(SensorInfo **sensorInfo, int32_t *count) const
{
    CALL_LOG_ENTER;
    CHKPR(sensorInfo, OHOS::Sensors::ERROR);
    CHKPR(count, OHOS::Sensors::ERROR);
    std::lock_guard<std::mutex> listLock(sensorInfoMutex_);
    if (sensorInfos_ == nullptr) {
        int32_t ret = ConvertSensorInfos();
        if (ret != SUCCESS) {
            SEN_HILOGE("Convert sensor lists failed");
            ClearSensorInfos();
            return ERROR;
        }
    }
    *sensorInfo = sensorInfos_;
    *count = sensorInfoCount_;
    return SUCCESS;
}

int32_t SensorAgentProxy::SuspendSensors(int32_t pid)
{
    CALL_LOG_ENTER;
    if (pid < 0) {
        SEN_HILOGE("Pid is invalid, pid:%{public}d", pid);
        return PARAMETER_ERROR;
    }
    int32_t ret = SEN_CLIENT.SuspendSensors(pid);
    if (ret != ERR_OK) {
        SEN_HILOGD("Suspend sensors failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t SensorAgentProxy::ResumeSensors(int32_t pid)
{
    CALL_LOG_ENTER;
    if (pid < 0) {
        SEN_HILOGE("Pid is invalid, pid:%{public}d", pid);
        return PARAMETER_ERROR;
    }
    int32_t ret = SEN_CLIENT.ResumeSensors(pid);
    if (ret != ERR_OK) {
        SEN_HILOGD("Resume sensors failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t SensorAgentProxy::GetSensorActiveInfos(int32_t pid,
    SensorActiveInfo **sensorActiveInfos, int32_t *count) const
{
    CALL_LOG_ENTER;
    if (pid < 0) {
        SEN_HILOGE("Pid is invalid, pid:%{public}d", pid);
        return PARAMETER_ERROR;
    }
    CHKPR(sensorActiveInfos, OHOS::Sensors::ERROR);
    CHKPR(count, OHOS::Sensors::ERROR);
    std::lock_guard<std::mutex> sensorActiveInfoLock(sensorActiveInfoMutex_);
    if (sensorActiveInfos_ != nullptr) {
        free(sensorActiveInfos_);
        sensorActiveInfos_ = nullptr;
    }
    std::vector<ActiveInfo> activeInfoList;
    int32_t ret = SEN_CLIENT.GetActiveInfoList(pid, activeInfoList);
    if (ret != ERR_OK) {
        SEN_HILOGE("Get active info list failed, ret:%{public}d", ret);
        return ret;
    }
    if (activeInfoList.empty()) {
        SEN_HILOGD("Active info list is empty");
        *sensorActiveInfos = nullptr;
        *count = 0;
        return ERR_OK;
    }
    size_t activeInfoCount = activeInfoList.size();
    if (activeInfoCount > MAX_SENSOR_LIST_SIZE) {
        SEN_HILOGE("The number of active info exceeds the maximum value, count:%{public}zu", activeInfoCount);
        return ERROR;
    }
    sensorActiveInfos_ = (SensorActiveInfo *)malloc(sizeof(SensorActiveInfo) * activeInfoCount);
    CHKPR(sensorActiveInfos_, ERROR);
    for (size_t i = 0; i < activeInfoCount; ++i) {
        SensorActiveInfo *curActiveInfo = sensorActiveInfos_ + i;
        curActiveInfo->pid = activeInfoList[i].GetPid();
        curActiveInfo->sensorId = activeInfoList[i].GetSensorId();
        curActiveInfo->samplingPeriodNs = activeInfoList[i].GetSamplingPeriodNs();
        curActiveInfo->maxReportDelayNs = activeInfoList[i].GetMaxReportDelayNs();
    }
    *sensorActiveInfos = sensorActiveInfos_;
    *count = static_cast<int32_t>(activeInfoCount);
    return ERR_OK;
}

int32_t SensorAgentProxy::Register(SensorActiveInfoCB callback)
{
    CHKPR(callback, OHOS::Sensors::ERROR);
    CHKPR(dataChannel_, INVALID_POINTER);
    int32_t ret = SEN_CLIENT.Register(callback, dataChannel_);
    if (ret != ERR_OK) {
        SEN_HILOGE("Register sensor active info callback failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t SensorAgentProxy::Unregister(SensorActiveInfoCB callback)
{
    CHKPR(callback, OHOS::Sensors::ERROR);
    int32_t ret = SEN_CLIENT.Unregister(callback);
    if (ret != ERR_OK) {
        SEN_HILOGE("Unregister sensor active info callback failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t SensorAgentProxy::ResetSensors() const
{
    int32_t ret = SEN_CLIENT.ResetSensors();
    if (ret != ERR_OK) {
        SEN_HILOGE("Reset sensors failed, ret:%{public}d", ret);
    }
    return ret;
}
}  // namespace Sensors
}  // namespace OHOS