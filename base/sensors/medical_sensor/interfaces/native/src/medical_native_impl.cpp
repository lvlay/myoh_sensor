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

#include "medical_native_impl.h"

#include "securec.h"
#include "medical_errors.h"
#include "medical_log_domain.h"
#include "medical_sensor_service_client.h"
#include "medical_sensor_data_channel.h"

using OHOS::HiviewDFX::HiLog;
using OHOS::HiviewDFX::HiLogLabel;
using OHOS::Sensors::MedicalSensorDataChannel;
using OHOS::Sensors::MedicalSensorServiceClient;
using OHOS::Sensors::INVALID_POINTER;
using OHOS::Sensors::SUCCESS;

static const HiLogLabel LABEL = {LOG_CORE, OHOS::MedicalSensorLogDomain::MEDICAL_SENSOR_NATIVE, "AfeNativeAPI"};
static bool g_isChannelCreated;
static int64_t g_samplingInterval;
static int64_t g_reportInterval;
static std::mutex subscribeMutex_;
static std::mutex chanelMutex_;
static std::map<int32_t, const MedicalSensorUser *> g_subscribeMap;
static std::map<int32_t, const MedicalSensorUser *> g_unsubscribeMap;
static OHOS::sptr<MedicalSensorDataChannel> g_dataChannel;

static void HandleSensorData(struct SensorEvent *events, int32_t num, void *data)
{
    if (events == nullptr || num <= 0) {
        HiLog::Error(LABEL, "%{public}s events is null or num is invalid", __func__);
        return;
    }
    SensorEvent eventStream;
    for (int32_t i = 0; i < num; ++i) {
        eventStream = events[i];
        if (eventStream.data == nullptr || g_subscribeMap[eventStream.sensorTypeId] == nullptr) {
            HiLog::Error(LABEL, "%{public}s data or sensorUser is nullptr", __func__);
            return;
        }
        if (g_subscribeMap.find(eventStream.sensorTypeId) == g_subscribeMap.end()) {
            HiLog::Error(LABEL, "%{public}s sensorTypeId not in g_subscribeMap", __func__);
            return;
        }
        g_subscribeMap[eventStream.sensorTypeId]->callback(&eventStream);
    }
}

static int32_t CreateSensorDataChannel()
{
    HiLog::Debug(LABEL, "%{public}s", __func__);
    std::lock_guard<std::mutex> chanelLock(chanelMutex_);
    if (g_isChannelCreated) {
        HiLog::Info(LABEL, "%{public}s the channel has already been created", __func__);
        return SUCCESS;
    }
    if (g_dataChannel == nullptr) {
        g_dataChannel = new (std::nothrow) MedicalSensorDataChannel();
    }
    auto ret = g_dataChannel->CreateSensorDataChannel(HandleSensorData, nullptr);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "%{public}s create data channel failed, ret : %{public}d", __func__, ret);
        return ret;
    }
    auto &client = MedicalSensorServiceClient::GetInstance();
    ret = client.TransferDataChannel(g_dataChannel);
    if (ret != SUCCESS) {
        auto destoryRet = g_dataChannel->DestroySensorDataChannel();
        HiLog::Error(LABEL, "%{public}s transfer data channel failed, ret : %{public}d, destoryRet : %{public}d",
                     __func__, ret, destoryRet);
        return ret;
    }
    g_isChannelCreated = true;
    return SUCCESS;
}

static int32_t DestroyAfeDataChannel()
{
    HiLog::Debug(LABEL, "%{public}s", __func__);
    std::lock_guard<std::mutex> chanelLock(chanelMutex_);
    if (!g_isChannelCreated) {
        HiLog::Info(LABEL, "%{public}s channel has been destroyed", __func__);
        return SUCCESS;
    }
    if (g_dataChannel == nullptr) {
        HiLog::Error(LABEL, "%{public}s data channel cannot be null", __func__);
        return INVALID_POINTER;
    }
    int32_t ret = g_dataChannel->DestroySensorDataChannel();
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "%{public}s destory data channel failed, ret : %{public}d", __func__, ret);
        return ret;
    }
    MedicalSensorServiceClient &client = MedicalSensorServiceClient::GetInstance();
    ret = client.DestroyDataChannel();
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "%{public}s destory service data channel fail, ret : %{public}d", __func__, ret);
        return ret;
    }
    g_isChannelCreated = false;
    delete g_dataChannel;
    g_dataChannel = nullptr;
    return SUCCESS;
}

int32_t GetAllSensors(MedicalSensorInfo **sensorInfo, int32_t *count)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    if (sensorInfo == nullptr || count == nullptr) {
        HiLog::Error(LABEL, "%{public}s sensorInfo or count is null", __func__);
        return OHOS::Sensors::ERROR;
    }
    MedicalSensorServiceClient &client = MedicalSensorServiceClient::GetInstance();
    std::vector<OHOS::Sensors::MedicalSensor> sensorList_ = client.GetSensorList();
    if (sensorList_.empty()) {
        HiLog::Error(LABEL, "%{public}s get sensor lists failed", __func__);
        return OHOS::Sensors::ERROR;
    }
    *count = sensorList_.size();
    *sensorInfo = (MedicalSensorInfo *)malloc(sizeof(MedicalSensorInfo) * (*count));
    if (*sensorInfo == nullptr) {
        HiLog::Error(LABEL, "%{public}s malloc sensorInfo failed", __func__);
        return OHOS::Sensors::ERROR;
    }
    for (int32_t index = 0; index < *count; index++) {
        errno_t ret = strcpy_s((*sensorInfo + index)->sensorName, AFE_NAME_MAX_LEN2,
            sensorList_[index].GetName().c_str());
        if (ret != EOK) {
            HiLog::Error(LABEL, "%{public}s strcpy sensorName failed", __func__);
            return OHOS::Sensors::ERROR;
        }
        ret = strcpy_s((*sensorInfo + index)->vendorName, AFE_NAME_MAX_LEN2, sensorList_[index].GetVendor().c_str());
        if (ret != EOK) {
            HiLog::Error(LABEL, "%{public}s strcpy vendorName failed", __func__);
            return OHOS::Sensors::ERROR;
        }
        const char *version = std::to_string(sensorList_[index].GetVersion()).c_str();
        ret = strcpy_s((*sensorInfo + index)->hardwareVersion, AFE_NAME_MAX_LEN2, version);
        if (ret != EOK) {
            HiLog::Error(LABEL, "%{public}s strcpy hardwareVersion failed", __func__);
            return OHOS::Sensors::ERROR;
        }
        (*sensorInfo + index)->sensorId = sensorList_[index].GetSensorId();
        (*sensorInfo + index)->sensorTypeId = sensorList_[index].GetSensorId();
    }
    return OHOS::Sensors::SUCCESS;
}

int32_t ActivateSensor(int32_t sensorId, const MedicalSensorUser *user)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    if (user == nullptr || sensorId < 0 || user->callback == nullptr) {
        HiLog::Error(LABEL, "%{public}s user is null or sensorId is invalid", __func__);
        return OHOS::Sensors::ERROR;
    }
    if (g_samplingInterval < 0 || g_reportInterval < 0) {
        HiLog::Error(LABEL, "%{public}s samplingPeroid or g_reportInterval is invalid", __func__);
        return OHOS::Sensors::ERROR;
    }
    std::lock_guard<std::mutex> subscribeLock(subscribeMutex_);
    if ((g_subscribeMap.find(sensorId) == g_subscribeMap.end()) || (g_subscribeMap.at(sensorId) != user)) {
        HiLog::Error(LABEL, "%{public}s subscribe sensorId first", __func__);
        return OHOS::Sensors::ERROR;
    }
    MedicalSensorServiceClient &client = MedicalSensorServiceClient::GetInstance();
    int32_t ret = client.EnableSensor(sensorId, g_samplingInterval, g_reportInterval);
    g_samplingInterval = -1;
    g_reportInterval = -1;
    if (ret != 0) {
        HiLog::Error(LABEL, "%{public}s enable sensor failed, ret: %{public}d", __func__, ret);
        g_subscribeMap.erase(sensorId);
        return OHOS::Sensors::ERROR;
    }
    return OHOS::Sensors::SUCCESS;
}

int32_t DeactivateSensor(int32_t sensorId, const MedicalSensorUser *user)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    if (user == nullptr || sensorId < 0 || user->callback == nullptr) {
        HiLog::Error(LABEL, "%{public}s user is null or sensorId is invalid", __func__);
        return OHOS::Sensors::ERROR;
    }
    std::lock_guard<std::mutex> subscribeLock(subscribeMutex_);
    if ((g_subscribeMap.find(sensorId) == g_subscribeMap.end()) || (g_subscribeMap[sensorId] != user)) {
        HiLog::Error(LABEL, "%{public}s subscribe sensorId first", __func__);
        return OHOS::Sensors::ERROR;
    }
    g_subscribeMap.erase(sensorId);
    g_unsubscribeMap[sensorId] = user;
    MedicalSensorServiceClient &client = MedicalSensorServiceClient::GetInstance();
    int32_t ret = client.DisableSensor(sensorId);
    if (ret != 0) {
        HiLog::Error(LABEL, "%{public}s disable sensor failed, ret: %{public}d", __func__, ret);
        return OHOS::Sensors::ERROR;
    }
    return OHOS::Sensors::SUCCESS;
}

int32_t SetBatch(int32_t sensorId, const MedicalSensorUser *user, int64_t samplingInterval, int64_t reportInterval)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    if (user == nullptr || sensorId < 0) {
        HiLog::Error(LABEL, "%{public}s user is null or sensorId is invalid", __func__);
        return OHOS::Sensors::ERROR;
    }
    if (samplingInterval < 0 || reportInterval < 0) {
        HiLog::Error(LABEL, "%{public}s samplingInterval or reportInterval is invalid", __func__);
        return OHOS::Sensors::ERROR;
    }
    std::lock_guard<std::mutex> subscribeLock(subscribeMutex_);
    if ((g_subscribeMap.find(sensorId) == g_subscribeMap.end()) || (g_subscribeMap.at(sensorId) != user)) {
        HiLog::Error(LABEL, "%{public}s subscribe sensorId first", __func__);
        return OHOS::Sensors::ERROR;
    }
    g_samplingInterval = samplingInterval;
    g_reportInterval = reportInterval;
    return OHOS::Sensors::SUCCESS;
}

int32_t SubscribeSensor(int32_t sensorId, const MedicalSensorUser *user)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    if (user == nullptr || sensorId < 0 || user->callback == nullptr) {
        HiLog::Error(LABEL, "%{public}s user or sensorId is invalid", __func__);
        return OHOS::Sensors::ERROR;
    }
    int32_t ret = CreateSensorDataChannel();
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "%{public}s create sensor data chanel failed", __func__);
        return OHOS::Sensors::ERROR;
    }
    std::lock_guard<std::mutex> subscribeLock(subscribeMutex_);
    g_subscribeMap[sensorId] = user;
    return OHOS::Sensors::SUCCESS;
}

int32_t UnsubscribeSensor(int32_t sensorId, const MedicalSensorUser *user)
{
    HiLog::Info(LABEL, "%{public}s in, sensorId: %{public}d", __func__, sensorId);
    if (user == nullptr || sensorId < 0  || user->callback == nullptr) {
        HiLog::Error(LABEL, "%{public}s user is null or sensorId is invalid", __func__);
        return OHOS::Sensors::ERROR;
    }
    std::lock_guard<std::mutex> subscribeLock(subscribeMutex_);
    if (g_unsubscribeMap.find(sensorId) == g_unsubscribeMap.end() || g_unsubscribeMap[sensorId] != user) {
        HiLog::Error(LABEL, "%{public}s deactivate sensorId first", __func__);
        return OHOS::Sensors::ERROR;
    }
    if (g_subscribeMap.empty()) {
        int32_t ret = DestroyAfeDataChannel();
        if (ret != SUCCESS) {
            HiLog::Error(LABEL, "%{public}s destory data channel fail, ret : %{public}d", __func__, ret);
            return ret;
        }
    }
    g_unsubscribeMap.erase(sensorId);
    return OHOS::Sensors::SUCCESS;
}

int32_t SetMode(int32_t sensorId, const MedicalSensorUser *user, int32_t mode)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    if (user == nullptr || sensorId < 0 || user->callback == nullptr) {
        HiLog::Error(LABEL, "%{public}s user is null or sensorId is invalid", __func__);
        return OHOS::Sensors::ERROR;
    }
    std::lock_guard<std::mutex> subscribeLock(subscribeMutex_);
    if ((g_subscribeMap.find(sensorId) == g_subscribeMap.end()) || (g_subscribeMap.at(sensorId) != user)) {
        HiLog::Error(LABEL, "%{public}s subscribe sensorId first", __func__);
        return OHOS::Sensors::ERROR;
    }
    return OHOS::Sensors::SUCCESS;
}

int32_t SetOption(int32_t sensorId, const MedicalSensorUser *user, int32_t option)
{
    HiLog::Info(LABEL, "%{public}s begin, sensorId: %{public}d, opt: %{public}d", __func__, sensorId, option);
    if (user == nullptr || sensorId < 0 || user->callback == nullptr) {
        HiLog::Error(LABEL, "%{public}s user is null or sensorId is invalid", __func__);
        return OHOS::Sensors::ERROR;
    }
    std::lock_guard<std::mutex> subscribeLock(subscribeMutex_);
    if ((g_subscribeMap.find(sensorId) == g_subscribeMap.end()) || (g_subscribeMap.at(sensorId) != user)) {
        HiLog::Error(LABEL, "%{public}s subscribe sensorId first", __func__);
        return OHOS::Sensors::ERROR;
    }
    return OHOS::Sensors::SUCCESS;
}