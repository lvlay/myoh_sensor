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

#include "medical_sensor_service_client.h"

#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "death_recipient_template.h"
#include "dmd_report.h"
#include "ipc_skeleton.h"
#include "medical_sensor_service_proxy.h"
#include "medical_errors.h"
#include "medical_log_domain.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = {
    LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_FRAMEWORK, "MedicalSensorServiceClient"
};
constexpr int32_t GET_SERVICE_MAX_COUNT = 30;
constexpr uint32_t WAIT_MS = 200;
}  // namespace

int32_t MedicalSensorServiceClient::InitServiceClient()
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    if (afeServer_ != nullptr) {
        HiLog::Debug(LABEL, "%{public}s already init", __func__);
        return ERR_OK;
    }
    if (afeClientStub_ == nullptr) {
        afeClientStub_ = new (std::nothrow) MedicalSensorClientStub();
    }
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        HiLog::Error(LABEL, "%{public}s systemAbilityManager cannot be null", __func__);
        return SENSOR_NATIVE_SAM_ERR;
    }
    int32_t retry = 0;
    while (retry < GET_SERVICE_MAX_COUNT) {
        afeServer_ = iface_cast<IMedicalSensorService>(
            systemAbilityManager->GetSystemAbility(MEDICAL_SENSOR_SERVICE_ABILITY_ID));
        if (afeServer_ != nullptr) {
            HiLog::Debug(LABEL, "%{public}s get service success, retry : %{public}d", __func__, retry);
            serviceDeathObserver_ =
                new (std::nothrow) DeathRecipientTemplate(*const_cast<MedicalSensorServiceClient *>(this));
            if (serviceDeathObserver_ != nullptr) {
                afeServer_->AsObject()->AddDeathRecipient(serviceDeathObserver_);
            }
            afeList_ = afeServer_->GetSensorList();
            return ERR_OK;
        }
        HiLog::Warn(LABEL, "%{public}s get service failed, retry : %{public}d", __func__, retry);
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
        retry++;
    }
    DmdReport::ReportException(SENSOR_SERVICE_EXCEPTION, "InitServiceClient", SENSOR_NATIVE_GET_SERVICE_ERR);
    HiLog::Error(LABEL, "%{public}s get service failed", __func__);
    return SENSOR_NATIVE_GET_SERVICE_ERR;
}

bool MedicalSensorServiceClient::IsValidSensorId(uint32_t sensorId)
{
    if (afeList_.empty()) {
        HiLog::Error(LABEL, "%{public}s afeList_ cannot be empty", __func__);
        return false;
    }
    for (auto &sensor : afeList_) {
        HiLog::Debug(LABEL, "%{public}s sensor.GetSensorId() = %{public}u", __func__, sensor.GetSensorId());
        if (sensor.GetSensorId() == sensorId) {
            return true;
        }
    }
    return false;
}

int32_t MedicalSensorServiceClient::EnableSensor(uint32_t sensorId, int64_t samplingPeriod, int64_t maxReportDelay)
{
    HiLog::Debug(LABEL, "%{public}s begin, sensorId = %{public}u", __func__, sensorId);
    if (!IsValidSensorId(sensorId)) {
        HiLog::Error(LABEL, "%{public}s sensorId is invalid", __func__);
        return SENSOR_NATIVE_SAM_ERR;
    }
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s InitServiceClient failed, ret : %{public}d", __func__, ret);
        return ret;
    }
    ret = afeServer_->EnableSensor(sensorId, samplingPeriod, maxReportDelay);
    if (ret == ERR_OK) {
        UpdateSensorInfoMap(sensorId, samplingPeriod, maxReportDelay);
    }
    return ret;
}

int32_t MedicalSensorServiceClient::DisableSensor(uint32_t sensorId)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (!IsValidSensorId(sensorId)) {
        HiLog::Error(LABEL, "%{public}s sensorId is invalid", __func__);
        return SENSOR_NATIVE_SAM_ERR;
    }
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s InitServiceClient failed, ret : %{public}d", __func__, ret);
        return ret;
    }
    ret = afeServer_->DisableSensor(sensorId);
    if (ret == ERR_OK) {
        DeleteSensorInfoItem(sensorId);
    }
    return ret;
}

int32_t MedicalSensorServiceClient::RunCommand(uint32_t sensorId, int32_t cmdType, int32_t params)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (!IsValidSensorId(sensorId)) {
        HiLog::Error(LABEL, "%{public}s sensorId is invalid", __func__);
        return SENSOR_NATIVE_SAM_ERR;
    }
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s InitServiceClient failed, ret : %{public}d", __func__, ret);
        return ret;
    }
    ret = afeServer_->RunCommand(sensorId, cmdType, params);
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s RunCommand failed", __func__);
        return ret;
    }
    return ret;
}

std::vector<MedicalSensor> MedicalSensorServiceClient::GetSensorList()
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s InitServiceClient failed, ret : %{public}d", __func__, ret);
        return {};
    }
    if (afeList_.empty()) {
        HiLog::Error(LABEL, "%{public}s afeList_ cannot be empty", __func__);
    }
    return afeList_;
}

int32_t MedicalSensorServiceClient::TransferDataChannel(sptr<MedicalSensorDataChannel> afeDataChannel)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    dataChannel_ = afeDataChannel;
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s InitServiceClient failed, ret : %{public}d", __func__, ret);
        return ret;
    }
    return afeServer_->TransferDataChannel(afeDataChannel, afeClientStub_);
}

int32_t MedicalSensorServiceClient::DestroyDataChannel()
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s InitServiceClient failed, ret : %{public}d", __func__, ret);
        return ret;
    }
    return afeServer_->DestroySensorChannel(afeClientStub_);
}

void MedicalSensorServiceClient::ProcessDeathObserver(const wptr<IRemoteObject> &object)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    (void)object;
    if (dataChannel_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s dataChannel_ cannot be null", __func__);
        return;
    }
    // STEP1 : Destroy revious data channel
    dataChannel_->DestroySensorDataChannel();

    // STEP2 : Restore data channel
    dataChannel_->RestoreSensorDataChannel();

    // STEP3 : Clear sensorlist and afeServer_
    afeList_.clear();
    afeServer_ = nullptr;

    // STEP4 : ReGet hsensors  3601 service
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s InitServiceClient failed, ret : %{public}d", __func__, ret);
        dataChannel_->DestroySensorDataChannel();
        return;
    }

    // STEP5 : Retransfer new channel to hsensors
    afeServer_->TransferDataChannel(dataChannel_, afeClientStub_);

    // STEP6 : Restore MedicalSensor status
    std::lock_guard<std::mutex> mapLock(mapMutex_);
    for (const auto &it : sensorInfoMap_) {
        afeServer_->EnableSensor(it.first, it.second.GetSamplingPeriodNs(), it.second.GetMaxReportDelayNs());
    }
    HiLog::Debug(LABEL, "%{public}s end", __func__);
}

void MedicalSensorServiceClient::UpdateSensorInfoMap(uint32_t sensorId, int64_t samplingPeriod, int64_t maxReportDelay)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    std::lock_guard<std::mutex> mapLock(mapMutex_);
    MedicalSensorBasicInfo sensorInfo;
    sensorInfo.SetSamplingPeriodNs(samplingPeriod);
    sensorInfo.SetMaxReportDelayNs(maxReportDelay);
    sensorInfo.SetSensorState(MedicalSensorState::SENSOR_ENABLED);
    sensorInfoMap_[sensorId] = sensorInfo;
    return;
}

void MedicalSensorServiceClient::DeleteSensorInfoItem(uint32_t sensorId)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    std::lock_guard<std::mutex> mapLock(mapMutex_);
    auto it = sensorInfoMap_.find(sensorId);
    if (it != sensorInfoMap_.end()) {
        sensorInfoMap_.erase(it);
    }
    return;
}

int32_t MedicalSensorServiceClient::SetOption(uint32_t sensorId, uint32_t opt)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (!IsValidSensorId(sensorId)) {
        HiLog::Error(LABEL, "%{public}s sensorId is invalid", __func__);
        return SENSOR_NATIVE_SAM_ERR;
    }
    int32_t ret = InitServiceClient();
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s InitServiceClient failed, ret : %{public}d", __func__, ret);
        return ret;
    }

    return afeServer_->SetOption(sensorId, opt);
}
}  // namespace Sensors
}  // namespace OHOS
