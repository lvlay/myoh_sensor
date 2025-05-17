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

#include "medical_sensor_service.h"

#include <cinttypes>
#include <sys/socket.h>
#include <unistd.h>

#include "hisysevent.h"
#include "iservice_registry.h"
#include "permission_util.h"
#include "securec.h"
#include "medical_sensor.h"
#include "medical_sensor_dump.h"
#include "medical_errors.h"
#include "medical_log_domain.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

enum class FlushCmdId {
    FLUSH = 0,
    SET_MODE,
    RESERVED,
};

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_SERVICE, "MedicalSensorService" };
constexpr uint32_t INVALID_SENSOR_ID = -1;
constexpr int32_t INVALID_PID = -1;
constexpr int64_t MAX_EVENT_COUNT = 1000;
constexpr uint32_t REPORT_STATUS_LEN = 20;
int32_t g_sendFd = 0;
}  // namespace

REGISTER_SYSTEM_ABILITY_BY_ID(MedicalSensorService, MEDICAL_SENSOR_SERVICE_ABILITY_ID, true);

MedicalSensorService::MedicalSensorService(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate), state_(MedicalSensorServiceState::STATE_STOPPED)
{}

void MedicalSensorService::OnDump()
{
    HiLog::Info(LABEL, "OnDump");
}

void MedicalSensorService::OnStart()
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    if (state_ == MedicalSensorServiceState::STATE_RUNNING) {
        HiLog::Warn(LABEL, "%{public}s MedicalSensorService has already started", __func__);
        return;
    }
    if (!InitInterface()) {
        HiLog::Error(LABEL, "%{public}s Init interface error", __func__);
        return;
    }
    if (!InitDataCache()) {
        HiLog::Error(LABEL, "%{public}s Init data callback error", __func__);
        return;
    }
    if (!InitSensorList()) {
        HiLog::Error(LABEL, "%{public}s Init sensor list error", __func__);
        return;
    }
    sensorDataProcesser_ = new (std::nothrow) MedicalSensorDataProcesser(sensorMap_);
    if (sensorDataProcesser_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s failed, sensorDataProcesser_ cannot be null", __func__);
        return;
    }

    bool isPublished = SystemAbility::Publish(this);
    if (!isPublished) {
        HiLog::Error(LABEL, "%{public}s publish MedicalSensorService error", __func__);
        return;
    }
    sensorManager_.InitSensorMap(sensorMap_, sensorDataProcesser_, reportDataCache_);

    state_ = MedicalSensorServiceState::STATE_RUNNING;
}

bool MedicalSensorService::InitInterface()
{
    auto ret = sensorHdiConnection_.ConnectHdi();
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s InitSensorServiceImpl failed", __func__);
        return false;
    }
    return true;
}

bool MedicalSensorService::InitDataCache()
{
    HiLog::Error(LABEL, "===%{public}s in", __func__);

    reportDataCache_ = new (std::nothrow) ReportDataCache();
    if (reportDataCache_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s failed, reportDataCache_ cannot be null", __func__);
        return false;
    }
    DataCacheFunc cacheData = &ReportDataCache::CacheData;
    auto ret = sensorHdiConnection_.RegisteDataReport(cacheData, reportDataCache_);
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s RegisterDataReport failed", __func__);
        return false;
    }
    return true;
}

bool MedicalSensorService::InitSensorList()
{
    std::lock_guard<std::mutex> sensorLock(sensorsMutex_);
    int32_t ret = sensorHdiConnection_.GetSensorList(sensors_);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s GetSensorList failed", __func__);
        return false;
    }

    std::lock_guard<std::mutex> sensorMapLock(sensorMapMutex_);
    for (const auto &it : sensors_) {
        sensorMap_.insert(std::make_pair(it.GetSensorId(), it));
        HiLog::Debug(LABEL, "%{public}s sensorId = %{public}d, name = %{public}s",
            __func__, it.GetSensorId(), it.GetName().c_str());
    }

    return true;
}

void MedicalSensorService::OnStop()
{
    if (state_ == MedicalSensorServiceState::STATE_STOPPED) {
        HiLog::Warn(LABEL, "%{public}s already stopped", __func__);
        return;
    }
    state_ = MedicalSensorServiceState::STATE_STOPPED;
    int32_t ret = sensorHdiConnection_.DestroyHdiConnection();
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s destroy hdi connect fail", __func__);
    }
}

void MedicalSensorService::ReportSensorUsedInfo(uint32_t sensorId, bool enable)
{
    char uidChar[REPORT_STATUS_LEN];
    int32_t uid = this->GetCallingUid();
    std::string packageName("");
    sensorManager_.GetPackageNameFromUid(uid, packageName);
    int32_t ret = sprintf_s(uidChar, sizeof(uidChar), "%d", uid);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s sprintf uidChar failed", __func__);
        return;
    }

    const int logLevel = 4;
    std::string message;
    if (enable) {
        // define in LogPower.java, 500 stand for enable sensor
        message.append("uid : ").append(std::to_string(uid)).append(" pkgName : ").append(packageName)
            .append(" type : ");
        HiSysEventWrite(HiSysEvent::Domain::MEDICAL_SENSOR, "EnableAfe", HiSysEvent::EventType::BEHAVIOR,
            "LEVEL", logLevel, "TAG", "DUBAI_TAG_HHEALTH_ENABLE", "MESSAGE", message);
    } else {
        // define in LogPower.java, 501 stand for disable sensor
        message.append("uid : ").append(std::to_string(uid)).append(" pkgName : ").append(packageName)
            .append(" type : ");
        HiSysEventWrite(HiSysEvent::Domain::MEDICAL_SENSOR, "DisableAfe", HiSysEvent::EventType::BEHAVIOR,
            "LEVEL", logLevel, "TAG", "DUBAI_TAG_HHEALTH_DISABLE", "MESSAGE", message);
    }

    HiLog::Info(LABEL, "%{public}s end, packageName : %{public}s", __func__, packageName.c_str());
}

void MedicalSensorService::ReportOnChangeData(uint32_t sensorId)
{
    std::lock_guard<std::mutex> sensorMapLock(sensorMapMutex_);
    auto it = sensorMap_.find(sensorId);
    if (it == sensorMap_.end()) {
        HiLog::Error(LABEL, "%{public}s sensorId is invalid", __func__);
        return;
    }
    if ((SENSOR_ON_CHANGE & it->second.GetFlags()) != SENSOR_ON_CHANGE) {
        HiLog::Warn(LABEL, "%{public}s it is not onchange data, no need to report", __func__);
        return;
    }
    SensorEvent event;
    auto ret = clientInfo_.GetStoreEvent(sensorId, event);
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s there is no data to be reported", __func__);
        return;
    }
    sptr<MedicalSensorBasicDataChannel> channel = clientInfo_.GetSensorChannelByPid(this->GetCallingPid());
    if (channel == nullptr) {
        HiLog::Error(LABEL, "%{public}s there is no channel to be reported", __func__);
        return;
    }
    auto sendRet = channel->SendData(&event, sizeof(event));
    if (sendRet != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s send data failed", __func__);
        return;
    }
}

ErrCode MedicalSensorService::SaveSubscriber(uint32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs)
{
    auto ret = sensorManager_.SaveSubscriber(sensorId, this->GetCallingPid(), samplingPeriodNs, maxReportDelayNs);
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s SaveSubscriber failed", __func__);
        return ret;
    }
    sensorManager_.StartDataReportThread();

    if (!sensorManager_.SetBestSensorParams(sensorId, samplingPeriodNs, maxReportDelayNs)) {
        HiLog::Error(LABEL, "%{public}s SetBestSensorParams failed", __func__);
        clientInfo_.RemoveSubscriber(sensorId, this->GetCallingPid());
        return ENABLE_SENSOR_ERR;
    }
    return ret;
}

ErrCode MedicalSensorService::EnableSensor(uint32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs)
{
    HiLog::Debug(LABEL, "%{public}s begin, sensorId : %{public}u, samplingPeriodNs : %{public}"
        PRId64, __func__, sensorId, samplingPeriodNs);
    if ((sensorId == INVALID_SENSOR_ID) ||
        ((samplingPeriodNs != 0L) && ((maxReportDelayNs / samplingPeriodNs) > MAX_EVENT_COUNT))) {
        HiLog::Error(LABEL, "%{public}s sensorId is 0 or maxReportDelayNs exceeded the maximum value", __func__);
        return ERR_NO_INIT;
    }
    ReportSensorUsedInfo(sensorId, SENSOR_ENABLED);
    std::lock_guard<std::mutex> serviceLock(serviceLock_);
    if (clientInfo_.GetSensorState(sensorId) == SENSOR_ENABLED) {
        HiLog::Warn(LABEL, "%{public}s medical sensor has been enabled already", __func__);
        auto ret = SaveSubscriber(sensorId, samplingPeriodNs, maxReportDelayNs);
        if (ret != ERR_OK) {
            HiLog::Error(LABEL, "%{public}s SaveSubscriber failed", __func__);
            return ret;
        }
        uint32_t flag = sensorManager_.GetSensorFlag(sensorId);
        int32_t pid = this->GetCallingPid();
        ret = flushInfo_.FlushProcess(sensorId, flag, pid, true);
        if (ret != ERR_OK) {
            HiLog::Error(LABEL, "%{public}s ret : %{public}d", __func__, ret);
        }
        ReportOnChangeData(sensorId);
        return ERR_OK;
    }
    auto ret = SaveSubscriber(sensorId, samplingPeriodNs, maxReportDelayNs);
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s SaveSubscriber failed", __func__);
        clientInfo_.RemoveSubscriber(sensorId, this->GetCallingPid());
        return ret;
    }

    ret = sensorHdiConnection_.EnableSensor(sensorId);
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s EnableSensor failed", __func__);
        clientInfo_.RemoveSubscriber(sensorId, this->GetCallingPid());
        return ENABLE_SENSOR_ERR;
    }

    return ret;
}

ErrCode MedicalSensorService::DisableSensor(uint32_t sensorId)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (sensorId == INVALID_SENSOR_ID) {
        HiLog::Error(LABEL, "%{public}s sensorId is invalid", __func__);
        return ERR_NO_INIT;
    }
    ReportSensorUsedInfo(sensorId, SENSOR_DISABLED);
    std::lock_guard<std::mutex> serviceLock(serviceLock_);
    const int32_t clientPid = this->GetCallingPid();
    if (clientPid < 0) {
        HiLog::Error(LABEL, "%{public}s clientPid is invalid, clientPid : %{public}d", __func__, clientPid);
        return CLIENT_PID_INVALID_ERR;
    }
    if (clientInfo_.GetSensorState(sensorId) != SENSOR_ENABLED) {
        HiLog::Error(LABEL, "%{public}s sensor should be enabled first", __func__);
        return DISABLE_SENSOR_ERR;
    }
    if (sensorManager_.IsOtherClientUsingSensor(sensorId, clientPid)) {
        HiLog::Warn(LABEL, "%{public}s other client is using this sensor now, cannot disable", __func__);
        return ERR_OK;
    }
    if (sensorHdiConnection_.DisableSensor(sensorId) != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s DisableSensor failed", __func__);
        return DISABLE_SENSOR_ERR;
    }
    clientInfo_.DestroyCmd(this->GetCallingUid());
    clientInfo_.ClearDataQueue(sensorId);
    return sensorManager_.AfterDisableSensor(sensorId);
}

ErrCode MedicalSensorService::SetOption(uint32_t sensorId, uint32_t opt)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (sensorId == INVALID_SENSOR_ID) {
        HiLog::Error(LABEL, "%{public}s sensorId is invalid", __func__);
        return ERR_NO_INIT;
    }
    if (sensorHdiConnection_.SetOption(sensorId, opt) != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s SetOption failed", __func__);
        return DISABLE_SENSOR_ERR;
    }

    HiLog::Debug(LABEL, "%{public}s end", __func__);
    return ERR_OK;
}


int32_t MedicalSensorService::GetSensorState(uint32_t sensorId)
{
    if (sensorId == INVALID_SENSOR_ID) {
        HiLog::Error(LABEL, "%{public}s sensorId is 0", __func__);
        return ERR_NO_INIT;
    }
    auto state = clientInfo_.GetSensorState(sensorId);
    return static_cast<int32_t>(state);
}

ErrCode MedicalSensorService::RunCommand(uint32_t sensorId, uint32_t cmdType, uint32_t params)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (sensorId == INVALID_SENSOR_ID ||
        ((cmdType != static_cast<uint32_t>(FlushCmdId::FLUSH)) &&
         (cmdType != static_cast<uint32_t>(FlushCmdId::SET_MODE)))) {
        HiLog::Error(LABEL, "%{public}s sensorId or cmd is invalid", __func__);
        return ERR_NO_INIT;
    }
    std::lock_guard<std::mutex> serviceLock(serviceLock_);
    uint32_t flag = sensorManager_.GetSensorFlag(sensorId);
    if (cmdType == static_cast<uint32_t>(FlushCmdId::FLUSH)) {
        int32_t pid = this->GetCallingPid();
        HiLog::Info(LABEL, "%{public}s sensorId : %{public}u, flag : %{public}u", __func__, sensorId, flag);
        auto retFlush = flushInfo_.FlushProcess(sensorId, flag, pid, false);
        if (retFlush != ERR_OK) {
            HiLog::Error(LABEL, "%{public}s ret : %{public}d", __func__, retFlush);
        }
        return retFlush;
    }
    if (sensorHdiConnection_.RunCommand(sensorId, cmdType, params) != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s RunCommand failed", __func__);
        return RUN_COMMAND_ERR;
    }
    auto uid = this->GetCallingUid();
    clientInfo_.UpdateCmd(sensorId, uid, cmdType);
    return ERR_OK;
}

std::vector<MedicalSensor> MedicalSensorService::GetSensorList()
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    std::lock_guard<std::mutex> sensorLock(sensorsMutex_);
    int32_t ret = sensorHdiConnection_.GetSensorList(sensors_);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s GetSensorList failed", __func__);
        return sensors_;
    }
    for (const auto &it : sensors_) {
        std::lock_guard<std::mutex> sensorMapLock(sensorMapMutex_);
        sensorMap_.insert(std::make_pair(it.GetSensorId(), it));
    }
    HiLog::Debug(LABEL, "%{public}s end", __func__);
    return sensors_;
}

ErrCode MedicalSensorService::TransferDataChannel(const sptr<MedicalSensorBasicDataChannel> &sensorBasicDataChannel,
    const sptr<IRemoteObject> &afeClient)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);

    if ((sensorBasicDataChannel == nullptr)) {
        HiLog::Error(LABEL, "%{public}s sensorBasicDataChannel cannot be null", __func__);
        return ERR_NO_INIT;
    }

    g_sendFd = sensorBasicDataChannel->GetSendDataFd();
    auto pid = this->GetCallingPid();
    auto uid = this->GetCallingUid();
    auto callerToken = GetCallingTokenID();
    if (!clientInfo_.UpdateUid(pid, uid, callerToken)) {
        HiLog::Error(LABEL, "%{public}s UpdateUid failed", __func__);
        return UPDATE_UID_ERR;
    }
    if (!clientInfo_.UpdateSensorChannel(pid, sensorBasicDataChannel)) {
        HiLog::Error(LABEL, "%{public}s UpdateSensorChannel failed", __func__);
        return UPDATE_SENSOR_CHANNEL_ERR;
    }
    sensorBasicDataChannel->SetSensorStatus(true);
    RegisterClientDeathRecipient(afeClient, pid);

    HiLog::Debug(LABEL, "%{public}s end", __func__);
    return ERR_OK;
}

ErrCode MedicalSensorService::DestroySensorChannel(sptr<IRemoteObject> afeClient)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    const int32_t clientPid = this->GetCallingPid();
    if (clientPid < 0) {
        HiLog::Error(LABEL, "%{public}s clientPid is invalid, clientPid : %{public}d", __func__, clientPid);
        return CLIENT_PID_INVALID_ERR;
    }
    std::lock_guard<std::mutex> serviceLock(serviceLock_);
    bool destoryRet = clientInfo_.DestroySensorChannel(clientPid);
    if (!destoryRet) {
        HiLog::Error(LABEL, "%{public}s DestroySensorChannel failed", __func__);
        return DESTROY_SENSOR_CHANNEL_ERR;
    }
    clientInfo_.DestroyCmd(this->GetCallingUid());
    UnregisterClientDeathRecipient(afeClient);
    HiLog::Info(LABEL, "%{public}s end", __func__);
    return ERR_OK;
}

void MedicalSensorService::ProcessDeathObserver(const wptr<IRemoteObject> &object)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    sptr<IRemoteObject> client = object.promote();
    if (client == nullptr) {
        HiLog::Error(LABEL, "%{public}s client cannot be null", __func__);
        return;
    }
    int32_t pid = clientInfo_.FindClientPid(client);
    if (pid == INVALID_PID) {
        HiLog::Error(LABEL, "%{public}s pid is -1", __func__);
        return;
    }
    HiLog::Info(LABEL, "%{public}s pid is %d", __func__, pid);
    clientInfo_.DestroySensorChannel(pid);
    clientInfo_.DestroyClientPid(client);
    clientInfo_.DestroyCmd(this->GetCallingUid());
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

void MedicalSensorService::RegisterClientDeathRecipient(sptr<IRemoteObject> afeClient, int32_t pid)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    sptr<IMedicalSensorClient> client = iface_cast<IMedicalSensorClient>(afeClient);

    clientDeathObserver_ = new (std::nothrow) DeathRecipientTemplate(*const_cast<MedicalSensorService *>(this));
    if (clientDeathObserver_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s clientDeathObserver_ cannot be null", __func__);
        return;
    }

    afeClient->AddDeathRecipient(clientDeathObserver_);
    HiLog::Info(LABEL, "%{public}s add clientDeathObserver_", __func__);

    clientInfo_.SaveClientPid(afeClient, pid);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

void MedicalSensorService::UnregisterClientDeathRecipient(sptr<IRemoteObject> afeClient)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    sptr<IMedicalSensorClient> client = iface_cast<IMedicalSensorClient>(afeClient);
    clientDeathObserver_ = new (std::nothrow) DeathRecipientTemplate(*const_cast<MedicalSensorService *>(this));
    if (clientDeathObserver_ == nullptr) {
        HiLog::Error(LABEL, "%{public}s clientDeathObserver_ cannot be null", __func__);
        return;
    }
    afeClient->RemoveDeathRecipient(clientDeathObserver_);
    clientInfo_.DestroyClientPid(afeClient);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

int32_t MedicalSensorService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    MedicalSensorDump &sensorDump = MedicalSensorDump::GetInstance();
    return sensorDump.Dump(fd, args, sensors_, clientInfo_);
}
}  // namespace Sensors
}  // namespace OHOS
