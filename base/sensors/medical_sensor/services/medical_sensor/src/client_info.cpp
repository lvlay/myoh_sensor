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

#include "client_info.h"

#include <mutex>

#include "permission_util.h"
#include "securec.h"
#include "sensor_hdi_connection.h"
#include "medical_errors.h"
#include "medical_log_domain.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_SERVICE, "ClientInfo" };
constexpr uint32_t INVALID_SENSOR_ID = -1;
constexpr int32_t INVALID_PID = -1;
constexpr int32_t INVALID_UID = -1;
constexpr int32_t MIN_MAP_SIZE = 0;
constexpr uint32_t NO_STROE_EVENT = -2;
constexpr uint32_t MAX_SUPPORT_CHANNEL = 200;
constexpr uint32_t MAX_DUMP_DATA_SIZE = 10;
constexpr uint32_t HEART_RATE_SENSOR_ID = 83886336;
}  // namespace

MedicalSensorState ClientInfo::GetSensorState(uint32_t sensorId)
{
    HiLog::Debug(LABEL, "%{public}s begin, sensorId : %{public}u", __func__, sensorId);
    if (sensorId == INVALID_SENSOR_ID) {
        HiLog::Error(LABEL, "%{public}s sensorId is invalid", __func__);
        return SENSOR_DISABLED;
    }

    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        HiLog::Error(LABEL, "%{public}s cannot find sensorId : %{public}u", __func__, sensorId);
        return SENSOR_DISABLED;
    }
    for (const auto &pidIt : it->second) {
        if (pidIt.second.GetSensorState() == SENSOR_ENABLED) {
            return SENSOR_ENABLED;
        }
    }
    HiLog::Error(LABEL, "%{public}s cannot find sensorinfo, sensorId : %{public}u", __func__, sensorId);
    return SENSOR_DISABLED;
}

MedicalSensorBasicInfo ClientInfo::GetBestSensorInfo(uint32_t sensorId)
{
    HiLog::Debug(LABEL, "%{public}s begin, sensorId : %{public}u", __func__, sensorId);
    int64_t minSamplingPeriodNs = LLONG_MAX;
    int64_t minReportDelayNs = LLONG_MAX;
    MedicalSensorBasicInfo sensorInfo;
    sensorInfo.SetSamplingPeriodNs(minSamplingPeriodNs);
    sensorInfo.SetMaxReportDelayNs(minReportDelayNs);
    if (sensorId == INVALID_SENSOR_ID) {
        HiLog::Error(LABEL, "%{public}s sensorId is invalid", __func__);
        return sensorInfo;
    }

    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        HiLog::Error(LABEL, "%{public}s cannot find sensorId : %{public}u", __func__, sensorId);
        return sensorInfo;
    }

    for (const auto &pidIt : it->second) {
        int64_t curSamplingPeriodNs = pidIt.second.GetSamplingPeriodNs();
        int64_t curReportDelayNs = pidIt.second.GetMaxReportDelayNs();
        minSamplingPeriodNs = (curSamplingPeriodNs < minSamplingPeriodNs) ? curSamplingPeriodNs : minSamplingPeriodNs;
        minReportDelayNs = (curReportDelayNs < minReportDelayNs) ? curReportDelayNs : minReportDelayNs;
    }
    sensorInfo.SetSamplingPeriodNs(minSamplingPeriodNs);
    sensorInfo.SetMaxReportDelayNs(minReportDelayNs);
    return sensorInfo;
}

bool ClientInfo::OnlyCurPidSensorEnabled(uint32_t sensorId, int32_t pid)
{
    HiLog::Debug(LABEL, "%{public}s begin, sensorId : %{public}u, pid : %{public}d", __func__, sensorId, pid);
    if ((sensorId == INVALID_SENSOR_ID) || (pid <= INVALID_PID)) {
        HiLog::Error(LABEL, "%{public}s sensorId or pid is invalid", __func__);
        return false;
    }

    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        HiLog::Error(LABEL, "%{public}s cannot find sensorId : %{public}u", __func__, sensorId);
        return false;
    }
    bool ret = false;
    for (const auto &pidIt : it->second) {
        if (pidIt.second.GetSensorState() != SENSOR_ENABLED) {
            continue;
        }
        if (pidIt.first != pid) {
            return false;
        }
        ret = true;
    }
    HiLog::Debug(LABEL, "%{public}s end", __func__);
    return ret;
}

bool ClientInfo::UpdateUid(int32_t pid, int32_t uid, AccessTokenID callerToken)
{
    HiLog::Debug(LABEL, "%{public}s begin, pid : %{public}d, uid : %{public}d", __func__, pid, uid);
    if ((uid == INVALID_UID) || (pid <= INVALID_PID)) {
        HiLog::Error(LABEL, "%{public}s uid or pid is invalid", __func__);
        return false;
    }
    std::lock_guard<std::mutex> uidLock(uidMutex_);
    AppThreadInfo appThreadInfo(pid, uid, callerToken);
    auto uidIt = appThreadInfoMap_.find(pid);
    if (uidIt == appThreadInfoMap_.end()) {
        if (appThreadInfoMap_.size() == MAX_SUPPORT_CHANNEL) {
            HiLog::Error(LABEL, "%{public}s max support channel size is %{public}u", __func__, MAX_SUPPORT_CHANNEL);
            return false;
        }
        auto ret = appThreadInfoMap_.insert(std::make_pair(pid, appThreadInfo));
        HiLog::Debug(LABEL, "%{public}s insert uid ret.second : %{public}d", __func__, ret.second);
        return ret.second;
    }
    appThreadInfoMap_[pid] = appThreadInfo;
    HiLog::Debug(LABEL, "%{public}s end", __func__);
    return true;
}

bool ClientInfo::DestroyUid(int32_t pid)
{
    HiLog::Debug(LABEL, "%{public}s begin, pid : %{public}d", __func__, pid);
    if (pid == INVALID_PID) {
        HiLog::Error(LABEL, "%{public}s pid is invalid", __func__);
        return false;
    }
    std::lock_guard<std::mutex> uidLock(uidMutex_);
    auto uidIt = appThreadInfoMap_.find(pid);
    if (uidIt == appThreadInfoMap_.end()) {
        HiLog::Debug(LABEL, "%{public}s uid not exist, no need to destroy it", __func__);
        return true;
    }

    appThreadInfoMap_.erase(uidIt);

    HiLog::Debug(LABEL, "%{public}s end", __func__);
    return true;
}

std::vector<sptr<MedicalSensorBasicDataChannel>> ClientInfo::GetSensorChannelByUid(int32_t uid)
{
    HiLog::Debug(LABEL, "%{public}s begin, uid : %{public}d", __func__, uid);
    if (uid == INVALID_UID) {
        HiLog::Error(LABEL, "%{public}s uid is invalid", __func__);
        return {};
    }
    std::vector<sptr<MedicalSensorBasicDataChannel>> afeChannel;
    std::lock_guard<std::mutex> uidLock(uidMutex_);
    for (const auto &uidIt : appThreadInfoMap_) {
        if (uid != uidIt.second.uid) {
            continue;
        }
        std::lock_guard<std::mutex> channelLock(channelMutex_);
        auto channelIt = channelMap_.find(uidIt.first);
        if (channelIt == channelMap_.end()) {
            continue;
        }
        afeChannel.push_back(channelIt->second);
    }
    HiLog::Debug(LABEL, "%{public}s end", __func__);
    return afeChannel;
}

sptr<MedicalSensorBasicDataChannel> ClientInfo::GetSensorChannelByPid(int32_t pid)
{
    HiLog::Debug(LABEL, "%{public}s begin, pid : %{public}d", __func__, pid);
    if (pid == INVALID_PID) {
        HiLog::Error(LABEL, "%{public}s pid is invalid", __func__);
        return nullptr;
    }
    std::lock_guard<std::mutex> channelLock(channelMutex_);
    auto channelIt = channelMap_.find(pid);
    if (channelIt == channelMap_.end()) {
        HiLog::Error(LABEL, "%{public}s there is no channel belong to the pid", __func__);
        return nullptr;
    }
    HiLog::Debug(LABEL, "%{public}s end", __func__);
    return channelIt->second;
}

std::vector<sptr<MedicalSensorBasicDataChannel>> ClientInfo::GetSensorChannel(uint32_t sensorId)
{
    if (sensorId == INVALID_SENSOR_ID) {
        HiLog::Error(LABEL, "%{public}s sensorId is invalid", __func__);
        return {};
    }

    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto clientIt = clientMap_.find(sensorId);
    if (clientIt == clientMap_.end()) {
        HiLog::Debug(LABEL, "%{public}s there is no channel belong to sensorId : %{public}u", __func__, sensorId);
        return {};
    }
    std::vector<sptr<MedicalSensorBasicDataChannel>> afeChannel;
    for (const auto &sensorInfoIt : clientIt->second) {
        std::lock_guard<std::mutex> channelLock(channelMutex_);
        auto channelIt = channelMap_.find(sensorInfoIt.first);
        if (channelIt == channelMap_.end()) {
            continue;
        }
        afeChannel.push_back(channelIt->second);
    }
    return afeChannel;
}

bool ClientInfo::UpdateSensorInfo(uint32_t sensorId, int32_t pid, const MedicalSensorBasicInfo &sensorInfo)
{
    HiLog::Debug(LABEL, "%{public}s begin, sensorId : %{public}u, pid : %{public}d", __func__, sensorId, pid);
    if ((sensorId == INVALID_SENSOR_ID) || (pid <= INVALID_PID) || (sensorInfo.GetSensorState() != SENSOR_ENABLED)) {
        HiLog::Error(LABEL, "%{public}s params are invalid", __func__);
        return false;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        std::unordered_map<int32_t, MedicalSensorBasicInfo> pidMap;
        auto pidRet = pidMap.insert(std::make_pair(pid, sensorInfo));
        auto clientRet = clientMap_.insert(std::make_pair(sensorId, pidMap));
        return pidRet.second && clientRet.second;
    }
    auto pidIt = it->second.find(pid);
    if (pidIt == it->second.end()) {
        auto ret = it->second.insert(std::make_pair(pid, sensorInfo));
        return ret.second;
    }
    it->second[pid] = sensorInfo;
    return true;
}

void ClientInfo::RemoveSubscriber(uint32_t sensorId, uint32_t pid)
{
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        HiLog::Warn(LABEL, "%{public}s sensorId not exist", __func__);
        return;
    }
    auto pidIt = it->second.find(pid);
    if (pidIt != it->second.end()) {
        it->second.erase(pidIt);
    }
}

bool ClientInfo::UpdateSensorChannel(int32_t pid, const sptr<MedicalSensorBasicDataChannel> &channel)
{
    HiLog::Debug(LABEL, "%{public}s begin, pid : %{public}d", __func__, pid);
    if (pid <= INVALID_PID || channel == nullptr) {
        HiLog::Error(LABEL, "%{public}s pid or channel is invalid or channel cannot be null", __func__);
        return false;
    }
    std::lock_guard<std::mutex> channelLock(channelMutex_);
    auto it = channelMap_.find(pid);
    if (it == channelMap_.end()) {
        if (channelMap_.size() == MAX_SUPPORT_CHANNEL) {
            HiLog::Error(LABEL, "%{public}s max support channel size : %{public}u", __func__, MAX_SUPPORT_CHANNEL);
            return false;
        }
        auto ret = channelMap_.insert(std::make_pair(pid, channel));
        HiLog::Debug(LABEL, "%{public}s ret.second : %{public}d", __func__, ret.second);
        return ret.second;
    }
    channelMap_[pid] = channel;
    HiLog::Debug(LABEL, "%{public}s end", __func__);
    return true;
}

bool ClientInfo::ClearSensorInfo(uint32_t sensorId)
{
    HiLog::Debug(LABEL, "%{public}s begin, sensorId : %{public}d", __func__, sensorId);
    if (sensorId == INVALID_SENSOR_ID) {
        HiLog::Error(LABEL, "%{public}s sensorId is invalid", __func__);
        return false;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        HiLog::Debug(LABEL, "%{public}s sensorId not exist, no need to clear it", __func__);
        return true;
    }
    clientMap_.erase(it);
    return true;
}

void ClientInfo::ClearCurPidSensorInfo(uint32_t sensorId, int32_t pid)
{
    HiLog::Debug(LABEL, "%{public}s begin, sensorId : %{public}d", __func__, sensorId);
    if ((sensorId == INVALID_SENSOR_ID) || (pid <= INVALID_PID)) {
        HiLog::Error(LABEL, "%{public}s sensorId or pid is invalid", __func__);
        return;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        HiLog::Debug(LABEL, "%{public}s sensorId not exist, no need to clear it", __func__);
        return;
    }
    auto pidIt = it->second.find(pid);
    if (pidIt == it->second.end()) {
        HiLog::Debug(LABEL, "%{public}s pid not exist, no need to clear it", __func__);
        return;
    }
    pidIt = it->second.erase(pidIt);
    if (it->second.size() == MIN_MAP_SIZE) {
        it = clientMap_.erase(it);
    }
}

bool ClientInfo::DestroySensorChannel(int32_t pid)
{
    HiLog::Debug(LABEL, "%{public}s start, pid : %{public}d", __func__, pid);
    if (pid <= INVALID_PID) {
        HiLog::Error(LABEL, "%{public}s pid is invalid", __func__);
        return false;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    for (auto it = clientMap_.begin(); it != clientMap_.end();) {
        auto pidIt = it->second.find(pid);
        if (pidIt == it->second.end()) {
            it++;
            continue;
        }
        pidIt = it->second.erase(pidIt);
        if (it->second.size() != MIN_MAP_SIZE) {
            it++;
            continue;
        }
        it = clientMap_.erase(it);
    }
    DestroyUid(pid);
    std::lock_guard<std::mutex> channelLock(channelMutex_);
    auto it = channelMap_.find(pid);
    if (it == channelMap_.end()) {
        HiLog::Debug(LABEL, "%{public}s there is no channel belong to pid, no need to destroy", __func__);
        return true;
    }
    it = channelMap_.erase(it);
    HiLog::Debug(LABEL, "%{public}s end", __func__);
    return true;
}

MedicalSensorBasicInfo ClientInfo::GetCurPidSensorInfo(uint32_t sensorId, int32_t pid)
{
    HiLog::Debug(LABEL, "%{public}s begin, sensorId : %{public}u", __func__, sensorId);
    int64_t minSamplingPeriodNs = LLONG_MAX;
    int64_t minReportDelayNs = LLONG_MAX;
    MedicalSensorBasicInfo sensorInfo;
    sensorInfo.SetSamplingPeriodNs(minSamplingPeriodNs);
    sensorInfo.SetMaxReportDelayNs(minReportDelayNs);
    if ((sensorId == INVALID_SENSOR_ID) || (pid <= INVALID_PID)) {
        HiLog::Error(LABEL, "%{public}s sensorId or channel is invalid", __func__);
        return sensorInfo;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        HiLog::Error(LABEL, "%{public}s cannot find sensorId : %{public}u", __func__, sensorId);
        return sensorInfo;
    }
    auto pidIt = it->second.find(pid);
    if (pidIt == it->second.end()) {
        HiLog::Error(LABEL, "%{public}s cannot find pid : %{public}d", __func__, pid);
        return sensorInfo;
    }
    sensorInfo.SetSamplingPeriodNs(pidIt->second.GetSamplingPeriodNs());
    sensorInfo.SetMaxReportDelayNs(pidIt->second.GetMaxReportDelayNs());
    HiLog::Debug(LABEL, "%{public}s end", __func__);
    return sensorInfo;
}

uint64_t ClientInfo::ComputeBestPeriodCount(uint32_t sensorId, sptr<MedicalSensorBasicDataChannel> &channel)
{
    if (sensorId == INVALID_SENSOR_ID || channel == nullptr) {
        HiLog::Error(LABEL, "%{public}s sensorId is invalid or channel cannot be null", __func__);
        return 0UL;
    }
    int32_t pid = INVALID_PID;
    {
        std::lock_guard<std::mutex> channelLock(channelMutex_);
        for (const auto &channelIt : channelMap_) {
            if (channelIt.second == channel) {
                pid = channelIt.first;
            }
        }
    }

    int64_t bestSamplingPeriod = GetBestSensorInfo(sensorId).GetSamplingPeriodNs();
    int64_t curSamplingPeriod = GetCurPidSensorInfo(sensorId, pid).GetSamplingPeriodNs();
    if (bestSamplingPeriod == 0L) {
        HiLog::Error(LABEL, "%{public}s best MedicalSensor Sampling Period is 0", __func__);
        return 0UL;
    }
    int64_t ret = curSamplingPeriod / bestSamplingPeriod;
    return (ret <= 0L) ? 0UL : ret;
}

uint64_t ClientInfo::ComputeBestFifoCount(uint32_t sensorId, sptr<MedicalSensorBasicDataChannel> &channel)
{
    if (channel == nullptr || sensorId == INVALID_SENSOR_ID) {
        HiLog::Error(LABEL, "%{public}s sensorId is invalid or channel cannot be null", __func__);
        return 0UL;
    }
    int32_t pid = INVALID_PID;
    {
        std::lock_guard<std::mutex> channelLock(channelMutex_);
        for (const auto &channelIt : channelMap_) {
            if (channelIt.second == channel) {
                pid = channelIt.first;
            }
        }
    }
    int64_t curReportDelay = GetCurPidSensorInfo(sensorId, pid).GetMaxReportDelayNs();
    int64_t curSamplingPeriod = GetCurPidSensorInfo(sensorId, pid).GetSamplingPeriodNs();
    if (curSamplingPeriod == 0L) {
        HiLog::Error(LABEL, "%{public}s best sensor fifo count is 0", __func__);
        return 0UL;
    }
    int64_t ret = curReportDelay / curSamplingPeriod;
    return (ret <= 0L) ? 0UL : ret;
}

int32_t ClientInfo::GetStoreEvent(int32_t sensorId, struct SensorEvent &event)
{
    std::lock_guard<std::mutex> lock(eventMutex_);
    auto storedEvent = storedEvent_.find(sensorId);
    if (storedEvent != storedEvent_.end()) {
        errno_t ret = memcpy_s(&event, sizeof(struct SensorEvent), &storedEvent->second, sizeof(struct SensorEvent));
        if (ret != EOK) {
            HiLog::Error(LABEL, "%{public}s memcpy_s failed, sensorId : %{public}d", __func__, sensorId);
            return ret;
        }
        return ERR_OK;
    }

    HiLog::Error(LABEL, "%{public}s can't get store event, sensorId : %{public}u", __func__, sensorId);
    return NO_STROE_EVENT;
}

void ClientInfo::StoreEvent(const struct SensorEvent &event)
{
    bool foundSensor = false;
    SensorEvent storedEvent;
    auto sensorHdiConnection = &SensorHdiConnection::GetInstance();
    if (sensorHdiConnection == nullptr) {
        HiLog::Error(LABEL, "%{public}s sensorHdiConnection cannot be null", __func__);
        return;
    }
    std::vector<MedicalSensor> sensors;
    int32_t ret = sensorHdiConnection->GetSensorList(sensors);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s GetSensorList failed", __func__);
        return;
    }

    if (memcpy_s(&storedEvent, sizeof(storedEvent), &event, sizeof(event)) != EOK) {
        HiLog::Error(LABEL, "%{public}s memcpy_s failed", __func__);
        return;
    }
    for (size_t i = 0; i < sensors.size(); i++) {
        if ((int32_t)(sensors[i].GetSensorId()) == storedEvent.sensorTypeId) {
            HiLog::Debug(LABEL, "%{public}s sensorFlags : %{public}u", __func__, sensors[i].GetFlags());
            foundSensor = true;
            break;
        }
    }

    if (foundSensor) {
        std::lock_guard<std::mutex> lock(eventMutex_);
        storedEvent_[storedEvent.sensorTypeId] = storedEvent;
    }
}

bool ClientInfo::SaveClientPid(const sptr<IRemoteObject> &sensorClient, int32_t pid)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (sensorClient == nullptr) {
        HiLog::Error(LABEL, "%{public}s sensorClient cannot be null", __func__);
        return false;
    }
    std::lock_guard<std::mutex> lock(clientPidMutex_);
    auto it = clientPidMap_.find(sensorClient);
    if (it == clientPidMap_.end()) {
        clientPidMap_.insert(std::make_pair(sensorClient, pid));
        HiLog::Debug(LABEL, "%{public}s end", __func__);
        return true;
    }
    clientPidMap_.insert(std::make_pair(sensorClient, pid));
    HiLog::Debug(LABEL, "%{public}s end", __func__);
    return true;
}

int32_t ClientInfo::FindClientPid(const sptr<IRemoteObject> &sensorClient)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (sensorClient == nullptr) {
        HiLog::Error(LABEL, "%{public}s sensorClient cannot be null", __func__);
        return INVALID_PID;
    }
    std::lock_guard<std::mutex> lock(clientPidMutex_);
    auto it = clientPidMap_.find(sensorClient);
    if (it == clientPidMap_.end()) {
        HiLog::Error(LABEL, "%{public}s cannot find client pid", __func__);
        return INVALID_PID;
    }
    HiLog::Debug(LABEL, "%{public}s end, pid : %{public}d", __func__, it->second);
    return it->second;
}

void ClientInfo::DestroyClientPid(const sptr<IRemoteObject> &sensorClient)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    if (sensorClient == nullptr) {
        HiLog::Error(LABEL, "%{public}s sensorClient cannot be null", __func__);
        return;
    }
    std::lock_guard<std::mutex> lock(clientPidMutex_);
    auto it = clientPidMap_.find(sensorClient);
    if (it == clientPidMap_.end()) {
        HiLog::Error(LABEL, "%{public}s cannot find client pid", __func__);
        return;
    }
    HiLog::Debug(LABEL, "%{public}s end, pid : %{public}d", __func__, it->second);
    clientPidMap_.erase(it);
}

void ClientInfo::ClearEvent()
{
    std::lock_guard<std::mutex> lock(eventMutex_);
    storedEvent_.clear();
}

std::vector<uint32_t> ClientInfo::GetSensorIdByPid(int32_t pid)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    std::vector<uint32_t> sensorIdVec;
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    for (const auto &itClientMap : clientMap_) {
        auto it = itClientMap.second.find(pid);
        if (it != itClientMap.second.end()) {
            sensorIdVec.push_back(itClientMap.first);
        }
    }
    return sensorIdVec;
}

AppThreadInfo ClientInfo::GetAppInfoByChannel(const sptr<MedicalSensorBasicDataChannel> &channel)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    AppThreadInfo appThreadInfo;
    {
        std::lock_guard<std::mutex> channelLock(channelMutex_);
        for (auto channelIt = channelMap_.begin(); channelIt != channelMap_.end(); channelIt++) {
            if (channelIt->second == channel) {
                appThreadInfo.pid = channelIt->first;
            }
        }
    }
    {
        std::lock_guard<std::mutex> uidLock(uidMutex_);
        auto it = appThreadInfoMap_.find(appThreadInfo.pid);
        if (it != appThreadInfoMap_.end()) {
            appThreadInfo.uid = it->second.uid;
            appThreadInfo.callerToken = it->second.callerToken;
        }
    }
    return appThreadInfo;
}

void ClientInfo::GetSensorChannelInfo(std::vector<MedicalSensorChannelInfo> &channelInfo)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    for (const auto &sensorIt : clientMap_) {
        for (const auto &pidIt : sensorIt.second) {
            MedicalSensorChannelInfo channel;
            int32_t uid = GetUidByPid(pidIt.first);
            if (uid == INVALID_UID) {
                continue;
            }
            channel.SetUid(uid);
            std::string packageName("");
            channel.SetSensorId(sensorIt.first);
            channel.SetPackageName(packageName);
            int64_t samplingPeriodNs = pidIt.second.GetSamplingPeriodNs();
            int64_t maxReportDelayNs = pidIt.second.GetMaxReportDelayNs();
            channel.SetSamplingPeriodNs(samplingPeriodNs);
            uint32_t fifoCount = (samplingPeriodNs == 0) ? 0 : (uint32_t)(maxReportDelayNs / samplingPeriodNs);
            channel.SetFifoCount(fifoCount);
            channel.SetCmdType(GetCmdList(sensorIt.first, uid));
            channelInfo.push_back(channel);
        }
    }
}

int32_t ClientInfo::GetUidByPid(int32_t pid)
{
    std::lock_guard<std::mutex> uidLock(uidMutex_);
    auto uidIt = appThreadInfoMap_.find(pid);
    if (uidIt == appThreadInfoMap_.end()) {
        return INVALID_UID;
    }
    return uidIt->second.uid;
}

void ClientInfo::UpdateCmd(uint32_t sensorId, int32_t uid, int32_t cmdType)
{
    std::lock_guard<std::mutex> cmdLock(cmdMutex_);
    auto cmdIt = cmdMap_.find(sensorId);
    if (cmdIt == cmdMap_.end()) {
        std::unordered_map<int32_t, std::vector<int32_t>> cmds;
        std::vector<int32_t> tmp;
        tmp.push_back(cmdType);
        cmds.insert(std::make_pair(uid, tmp));
        cmdMap_.insert(std::make_pair(sensorId, cmds));
        return;
    }
    auto tmpIt = cmdIt->second.find(uid);
    if (tmpIt == cmdIt->second.end()) {
        std::vector<int32_t> tmp;
        tmp.push_back(cmdType);
        cmdIt->second.insert(std::make_pair(uid, tmp));
        return;
    }
    auto tmp = tmpIt->second;
    tmp.push_back(cmdType);
    cmdIt->second.insert(std::make_pair(uid, tmp));
}

void ClientInfo::DestroyCmd(int32_t uid)
{
    std::lock_guard<std::mutex> cmdLock(cmdMutex_);
    cmdMap_.erase(uid);
}

std::vector<int32_t> ClientInfo::GetCmdList(uint32_t sensorId, int32_t uid)
{
    std::lock_guard<std::mutex> cmdLock(cmdMutex_);
    auto cmdIt = cmdMap_.find(sensorId);
    if (cmdIt == cmdMap_.end()) {
        return {};
    }
    auto uidIt = cmdIt->second.find(uid);
    if (uidIt == cmdIt->second.end()) {
        return {};
    }
    return uidIt->second;
}

void ClientInfo::UpdateDataQueue(int32_t sensorId, struct SensorEvent &event)
{
    if (sensorId == HEART_RATE_SENSOR_ID) {
        return;
    }
    std::lock_guard<std::mutex> queueLock(dataQueueMutex_);
    auto it = dataQueue_.find(sensorId);
    if (it == dataQueue_.end()) {
        std::queue<struct SensorEvent> q;
        q.push(event);
        dataQueue_.insert(std::make_pair(sensorId, q));
        return;
    }
    it->second.push(event);
    if (it->second.size() > MAX_DUMP_DATA_SIZE) {
        it->second.pop();
    }
}

std::unordered_map<uint32_t, std::queue<struct SensorEvent>> ClientInfo::GetDataQueue()
{
    return dataQueue_;
}

void ClientInfo::ClearDataQueue(int32_t sensorId)
{
    std::lock_guard<std::mutex> queueLock(dataQueueMutex_);
    auto it = dataQueue_.find(sensorId);
    if (it != dataQueue_.end()) {
        dataQueue_.erase(it);
    }
}
}  // namespace Sensors
}  // namespace OHOS
