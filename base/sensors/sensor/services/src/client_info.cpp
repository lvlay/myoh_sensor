/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "sensor_errors.h"
#include "sensor_manager.h"
#ifdef HDF_DRIVERS_INTERFACE_SENSOR
#include "sensor_hdi_connection.h"
#endif // HDF_DRIVERS_INTERFACE_SENSOR

#undef LOG_TAG
#define LOG_TAG "ClientInfo"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

namespace {
constexpr int32_t INVALID_SENSOR_ID = -1;
constexpr int32_t INVALID_PID = -1;
constexpr int32_t INVALID_UID = -1;
constexpr int32_t MIN_MAP_SIZE = 0;
constexpr uint32_t NO_STORE_EVENT = -2;
constexpr uint32_t MAX_SUPPORT_CHANNEL = 200;
constexpr uint32_t MAX_DUMP_DATA_SIZE = 10;
} // namespace

std::unordered_map<std::string, std::set<int32_t>> ClientInfo::userGrantPermMap_ = {
    { ACTIVITY_MOTION_PERMISSION, { SENSOR_TYPE_ID_PEDOMETER_DETECTION, SENSOR_TYPE_ID_PEDOMETER } },
    { READ_HEALTH_DATA_PERMISSION, { SENSOR_TYPE_ID_HEART_RATE } }
};

bool ClientInfo::GetSensorState(int32_t sensorId)
{
    CALL_LOG_ENTER;
    if (sensorId == INVALID_SENSOR_ID) {
        SEN_HILOGE("sensorId is invalid");
        return false;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        SEN_HILOGE("Can't find sensorId:%{public}d", sensorId);
        return false;
    }
    for (const auto &pidIt : it->second) {
        if (pidIt.second.GetSensorState()) {
            return true;
        }
    }
    SEN_HILOGE("Can't find sensorInfo, sensorId:%{public}d", sensorId);
    return false;
}

SensorBasicInfo ClientInfo::GetBestSensorInfo(int32_t sensorId)
{
    int64_t minSamplingPeriodNs = LLONG_MAX;
    int64_t minReportDelayNs = LLONG_MAX;
    SensorBasicInfo sensorInfo;
    sensorInfo.SetSamplingPeriodNs(minSamplingPeriodNs);
    sensorInfo.SetMaxReportDelayNs(minReportDelayNs);
    if (sensorId == INVALID_SENSOR_ID) {
        SEN_HILOGE("sensorId is invalid");
        return sensorInfo;
    }

    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        SEN_HILOGE("Can't find sensorId:%{public}d", sensorId);
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

bool ClientInfo::OnlyCurPidSensorEnabled(int32_t sensorId, int32_t pid)
{
    CALL_LOG_ENTER;
    if ((sensorId == INVALID_SENSOR_ID) || (pid <= INVALID_PID)) {
        SEN_HILOGE("sensorId or pid is invalid");
        return false;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        SEN_HILOGE("Can't find sensorId:%{public}d", sensorId);
        return false;
    }
    bool ret = false;
    for (const auto &pidIt : it->second) {
        if (!pidIt.second.GetSensorState()) {
            continue;
        }
        if (pidIt.first != pid) {
            SEN_HILOGE("Current sensor is also used by other pid");
            return false;
        }
        ret = true;
    }
    return ret;
}

bool ClientInfo::UpdateAppThreadInfo(int32_t pid, int32_t uid, AccessTokenID callerToken)
{
    CALL_LOG_ENTER;
    if ((uid == INVALID_UID) || (pid <= INVALID_PID)) {
        SEN_HILOGE("uid or pid is invalid");
        return false;
    }
    std::lock_guard<std::mutex> uidLock(uidMutex_);
    AppThreadInfo appThreadInfo(pid, uid, callerToken);
    auto appThreadInfoItr = appThreadInfoMap_.find(pid);
    if (appThreadInfoItr == appThreadInfoMap_.end()) {
        if (appThreadInfoMap_.size() == MAX_SUPPORT_CHANNEL) {
            SEN_HILOGE("Max support channel size is %{public}d", MAX_SUPPORT_CHANNEL);
            return false;
        }
        auto ret = appThreadInfoMap_.insert(std::make_pair(pid, appThreadInfo));
        return ret.second;
    }
    appThreadInfoMap_[pid] = appThreadInfo;
    return true;
}

void ClientInfo::DestroyAppThreadInfo(int32_t pid)
{
    CALL_LOG_ENTER;
    if (pid == INVALID_PID) {
        SEN_HILOGE("pid is invalid");
        return;
    }
    std::lock_guard<std::mutex> uidLock(uidMutex_);
    auto appThreadInfoItr = appThreadInfoMap_.find(pid);
    if (appThreadInfoItr == appThreadInfoMap_.end()) {
        SEN_HILOGD("pid not exist, no need to destroy it");
        return;
    }
    appThreadInfoMap_.erase(appThreadInfoItr);
}

std::vector<sptr<SensorBasicDataChannel>> ClientInfo::GetSensorChannelByUid(int32_t uid)
{
    CALL_LOG_ENTER;
    if (uid == INVALID_UID) {
        SEN_HILOGE("uid is invalid");
        return {};
    }
    std::vector<sptr<SensorBasicDataChannel>> sensorChannel;
    std::lock_guard<std::mutex> uidLock(uidMutex_);
    for (const auto &appThreadInfoIt : appThreadInfoMap_) {
        if (uid != appThreadInfoIt.second.uid) {
            continue;
        }
        std::lock_guard<std::mutex> channelLock(channelMutex_);
        auto channelIt = channelMap_.find(appThreadInfoIt.first);
        if (channelIt == channelMap_.end()) {
            continue;
        }
        sensorChannel.push_back(channelIt->second);
    }
    return sensorChannel;
}

sptr<SensorBasicDataChannel> ClientInfo::GetSensorChannelByPid(int32_t pid)
{
    CALL_LOG_ENTER;
    if (pid == INVALID_PID) {
        SEN_HILOGE("pid is invalid");
        return nullptr;
    }
    std::lock_guard<std::mutex> channelLock(channelMutex_);
    auto channelIt = channelMap_.find(pid);
    if (channelIt == channelMap_.end()) {
        SEN_HILOGE("There is no channel belong to the pid");
        return nullptr;
    }
    return channelIt->second;
}

std::vector<sptr<SensorBasicDataChannel>> ClientInfo::GetSensorChannel(int32_t sensorId)
{
    if (sensorId == INVALID_SENSOR_ID) {
        SEN_HILOGE("sensorId is invalid");
        return {};
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto clientIt = clientMap_.find(sensorId);
    if (clientIt == clientMap_.end()) {
        SEN_HILOGD("There is no channel belong to sensorId:%{public}d", sensorId);
        return {};
    }
    std::vector<sptr<SensorBasicDataChannel>> sensorChannel;
    for (const auto &sensorInfoIt : clientIt->second) {
        std::lock_guard<std::mutex> channelLock(channelMutex_);
        auto channelIt = channelMap_.find(sensorInfoIt.first);
        if (channelIt == channelMap_.end()) {
            continue;
        }
        if (!sensorInfoIt.second.GetPermState()) {
            continue;
        }
        sensorChannel.push_back(channelIt->second);
    }
    return sensorChannel;
}

bool ClientInfo::UpdateSensorInfo(int32_t sensorId, int32_t pid, const SensorBasicInfo &sensorInfo)
{
    CALL_LOG_ENTER;
    if ((sensorId == INVALID_SENSOR_ID) || (pid <= INVALID_PID) || (!sensorInfo.GetSensorState())) {
        SEN_HILOGE("Params are invalid");
        return false;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        std::unordered_map<int32_t, SensorBasicInfo> pidMap;
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

void ClientInfo::RemoveSubscriber(int32_t sensorId, uint32_t pid)
{
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        SEN_HILOGW("sensorId not exist");
        return;
    }
    auto pidIt = it->second.find(pid);
    if (pidIt != it->second.end()) {
        it->second.erase(pidIt);
    }
}

bool ClientInfo::UpdateSensorChannel(int32_t pid, const sptr<SensorBasicDataChannel> &channel)
{
    CALL_LOG_ENTER;
    CHKPR(channel, false);
    if (pid <= INVALID_PID) {
        SEN_HILOGE("pid is invalid");
        return false;
    }
    std::lock_guard<std::mutex> channelLock(channelMutex_);
    auto it = channelMap_.find(pid);
    if (it == channelMap_.end()) {
        if (channelMap_.size() == MAX_SUPPORT_CHANNEL) {
            SEN_HILOGE("Max support channel size:%{public}d", MAX_SUPPORT_CHANNEL);
            return false;
        }
        auto ret = channelMap_.insert(std::make_pair(pid, channel));
        SEN_HILOGD("ret.second:%{public}d", ret.second);
        return ret.second;
    }
    channelMap_[pid] = channel;
    return true;
}

void ClientInfo::ClearSensorInfo(int32_t sensorId)
{
    CALL_LOG_ENTER;
    if (sensorId == INVALID_SENSOR_ID) {
        SEN_HILOGE("sensorId is invalid");
        return;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        SEN_HILOGD("sensorId not exist, no need to clear it");
        return;
    }
    clientMap_.erase(it);
}

void ClientInfo::ClearCurPidSensorInfo(int32_t sensorId, int32_t pid)
{
    CALL_LOG_ENTER;
    if ((sensorId == INVALID_SENSOR_ID) || (pid <= INVALID_PID)) {
        SEN_HILOGE("sensorId or pid is invalid");
        return;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        SEN_HILOGD("sensorId not exist, no need to clear it");
        return;
    }
    auto pidIt = it->second.find(pid);
    if (pidIt == it->second.end()) {
        SEN_HILOGD("pid not exist, no need to clear it");
        return;
    }
    pidIt = it->second.erase(pidIt);
    if (it->second.size() == MIN_MAP_SIZE) {
        it = clientMap_.erase(it);
    }
}

bool ClientInfo::DestroySensorChannel(int32_t pid)
{
    CALL_LOG_ENTER;
    if (pid <= INVALID_PID) {
        SEN_HILOGE("pid is invalid");
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
    DestroyAppThreadInfo(pid);
    std::lock_guard<std::mutex> channelLock(channelMutex_);
    auto it = channelMap_.find(pid);
    if (it == channelMap_.end()) {
        SEN_HILOGD("There is no channel belong to pid, no need to destroy");
        return true;
    }
    it = channelMap_.erase(it);
    return true;
}

SensorBasicInfo ClientInfo::GetCurPidSensorInfo(int32_t sensorId, int32_t pid)
{
    int64_t minSamplingPeriodNs = LLONG_MAX;
    int64_t minReportDelayNs = LLONG_MAX;
    SensorBasicInfo sensorInfo;
    sensorInfo.SetSamplingPeriodNs(minSamplingPeriodNs);
    sensorInfo.SetMaxReportDelayNs(minReportDelayNs);
    if ((sensorId == INVALID_SENSOR_ID) || (pid <= INVALID_PID)) {
        SEN_HILOGE("sensorId or channel is invalid");
        return sensorInfo;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        SEN_HILOGE("Can't find sensorId:%{public}d", sensorId);
        return sensorInfo;
    }
    auto pidIt = it->second.find(pid);
    if (pidIt == it->second.end()) {
        SEN_HILOGE("Can't find pid:%{public}d", pid);
        return sensorInfo;
    }
    sensorInfo.SetSamplingPeriodNs(pidIt->second.GetSamplingPeriodNs());
    sensorInfo.SetMaxReportDelayNs(pidIt->second.GetMaxReportDelayNs());
    return sensorInfo;
}

uint64_t ClientInfo::ComputeBestPeriodCount(int32_t sensorId, sptr<SensorBasicDataChannel> &channel)
{
    if (sensorId == INVALID_SENSOR_ID || channel == nullptr) {
        SEN_HILOGE("sensorId is invalid or channel cannot be null");
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
        SEN_HILOGE("Best sensor sampling period is 0");
        return 0UL;
    }
    int64_t ret = curSamplingPeriod / bestSamplingPeriod;
    return (ret <= 0L) ? 0UL : ret;
}

uint64_t ClientInfo::ComputeBestFifoCount(int32_t sensorId, sptr<SensorBasicDataChannel> &channel)
{
    if (channel == nullptr || sensorId == INVALID_SENSOR_ID) {
        SEN_HILOGE("sensorId is invalid or channel cannot be null");
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
        SEN_HILOGE("Best sensor fifo count is 0");
        return 0UL;
    }
    int64_t ret = curReportDelay / curSamplingPeriod;
    return (ret <= 0L) ? 0UL : ret;
}

int32_t ClientInfo::GetStoreEvent(int32_t sensorId, SensorData &data)
{
    std::lock_guard<std::mutex> lock(eventMutex_);
    auto storedEvent = storedEvent_.find(sensorId);
    if (storedEvent != storedEvent_.end()) {
        errno_t ret = memcpy_s(&data, sizeof(data), &storedEvent->second, sizeof(storedEvent->second));
        if (ret != EOK) {
            SEN_HILOGE("memcpy_s failed, sensorId:%{public}d", sensorId);
            return ret;
        }
        return ERR_OK;
    }

    SEN_HILOGE("Can't get store event, sensorId:%{public}d", sensorId);
    return NO_STORE_EVENT;
}

void ClientInfo::StoreEvent(const SensorData &data)
{
    bool foundSensor = false;
    SensorData storedEvent;
    std::vector<Sensor> sensors;
#ifdef HDF_DRIVERS_INTERFACE_SENSOR
    auto sensorHdiConnection = &SensorHdiConnection::GetInstance();
    if (sensorHdiConnection == nullptr) {
        SEN_HILOGE("sensorHdiConnection cannot be null");
        return;
    }
    int32_t ret = sensorHdiConnection->GetSensorList(sensors);
    if (ret != 0) {
        SEN_HILOGE("GetSensorList is failed");
        return;
    }
#endif // HDF_DRIVERS_INTERFACE_SENSOR
    errno_t retVal = memcpy_s(&storedEvent, sizeof(storedEvent), &data, sizeof(data));
    if (retVal != EOK) {
        SEN_HILOGE("memcpy_s is failed");
        return;
    }
    for (size_t i = 0; i < sensors.size(); i++) {
        if (sensors[i].GetSensorId() == storedEvent.sensorTypeId) {
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
    CALL_LOG_ENTER;
    CHKPF(sensorClient);
    std::lock_guard<std::mutex> lock(clientPidMutex_);
    auto it = clientPidMap_.find(sensorClient);
    if (it == clientPidMap_.end()) {
        clientPidMap_.insert(std::make_pair(sensorClient, pid));
        return true;
    }
    clientPidMap_.insert(std::make_pair(sensorClient, pid));
    return true;
}

int32_t ClientInfo::FindClientPid(const sptr<IRemoteObject> &sensorClient)
{
    CALL_LOG_ENTER;
    CHKPR(sensorClient, INVALID_PID);
    std::lock_guard<std::mutex> lock(clientPidMutex_);
    auto it = clientPidMap_.find(sensorClient);
    if (it == clientPidMap_.end()) {
        SEN_HILOGE("Cannot find client pid");
        return INVALID_PID;
    }
    return it->second;
}

void ClientInfo::DestroyClientPid(const sptr<IRemoteObject> &sensorClient)
{
    CALL_LOG_ENTER;
    CHKPV(sensorClient);
    std::lock_guard<std::mutex> lock(clientPidMutex_);
    auto it = clientPidMap_.find(sensorClient);
    if (it == clientPidMap_.end()) {
        SEN_HILOGE("Cannot find client pid");
        return;
    }
    clientPidMap_.erase(it);
}

void ClientInfo::ClearEvent()
{
    std::lock_guard<std::mutex> lock(eventMutex_);
    storedEvent_.clear();
}

std::vector<int32_t> ClientInfo::GetSensorIdByPid(int32_t pid)
{
    CALL_LOG_ENTER;
    std::vector<int32_t> sensorIdVec;
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    for (const auto &itClientMap : clientMap_) {
        auto it = itClientMap.second.find(pid);
        if (it != itClientMap.second.end()) {
            sensorIdVec.push_back(itClientMap.first);
        }
    }
    return sensorIdVec;
}

AppThreadInfo ClientInfo::GetAppInfoByChannel(const sptr<SensorBasicDataChannel> &channel)
{
    CALL_LOG_ENTER;
    AppThreadInfo appThreadInfo;
    if (channel == nullptr) {
        SEN_HILOGE("channel is nullptr");
        return appThreadInfo;
    }
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

void ClientInfo::GetSensorChannelInfo(std::vector<SensorChannelInfo> &channelInfo)
{
    CALL_LOG_ENTER;
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    for (const auto &sensorIt : clientMap_) {
        for (const auto &pidIt : sensorIt.second) {
            int32_t pid = pidIt.first;
            int32_t uid = GetUidByPid(pid);
            if (uid == INVALID_UID) {
                SEN_HILOGW("uid is invalid, uid:%{public}d", uid);
                continue;
            }
            SensorChannelInfo channel;
            channel.SetUid(uid);
            channel.SetSensorId(sensorIt.first);
            std::string packageName;
            SensorManager::GetInstance().GetPackageName(GetTokenIdByPid(pid), packageName);
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
    auto appThreadInfoIt = appThreadInfoMap_.find(pid);
    if (appThreadInfoIt == appThreadInfoMap_.end()) {
        return INVALID_UID;
    }
    return appThreadInfoIt->second.uid;
}

AccessTokenID ClientInfo::GetTokenIdByPid(int32_t pid)
{
    std::lock_guard<std::mutex> uidLock(uidMutex_);
    auto appThreadInfoIt = appThreadInfoMap_.find(pid);
    if (appThreadInfoIt == appThreadInfoMap_.end()) {
        return INVALID_UID;
    }
    return appThreadInfoIt->second.callerToken;
}

void ClientInfo::UpdateCmd(int32_t sensorId, int32_t uid, int32_t cmdType)
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

std::vector<int32_t> ClientInfo::GetCmdList(int32_t sensorId, int32_t uid)
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

void ClientInfo::UpdateDataQueue(int32_t sensorId, SensorData &data)
{
    if (sensorId == SENSOR_TYPE_ID_HEART_RATE) {
        return;
    }
    std::lock_guard<std::mutex> queueLock(dataQueueMutex_);
    auto it = dumpQueue_.find(sensorId);
    if (it == dumpQueue_.end()) {
        std::queue<SensorData> q;
        q.push(data);
        dumpQueue_.insert(std::make_pair(sensorId, q));
        return;
    }
    it->second.push(data);
    if (it->second.size() > MAX_DUMP_DATA_SIZE) {
        it->second.pop();
    }
}

std::unordered_map<int32_t, std::queue<SensorData>> ClientInfo::GetDumpQueue()
{
    return dumpQueue_;
}

void ClientInfo::ClearDataQueue(int32_t sensorId)
{
    std::lock_guard<std::mutex> queueLock(dataQueueMutex_);
    auto it = dumpQueue_.find(sensorId);
    if (it != dumpQueue_.end()) {
        dumpQueue_.erase(it);
    }
}

int32_t ClientInfo::AddActiveInfoCBPid(int32_t pid)
{
    std::lock_guard<std::mutex> activeInfoCBPidLock(activeInfoCBPidMutex_);
    auto pairRet = activeInfoCBPidSet_.insert(pid);
    if (!pairRet.second) {
        SEN_HILOGE("Pid is duplicated");
        return ERROR;
    }
    return ERR_OK;
}

int32_t ClientInfo::DelActiveInfoCBPid(int32_t pid)
{
    std::lock_guard<std::mutex> activeInfoCBPidLock(activeInfoCBPidMutex_);
    auto it = activeInfoCBPidSet_.find(pid);
    if (it == activeInfoCBPidSet_.end()) {
        SEN_HILOGE("Pid is not exists");
        return ERROR;
    }
    activeInfoCBPidSet_.erase(it);
    return ERR_OK;
}

std::vector<int32_t> ClientInfo::GetActiveInfoCBPid()
{
    std::vector<int32_t> activeInfoCBPids;
    std::lock_guard<std::mutex> activeInfoCBPidLock(activeInfoCBPidMutex_);
    for (auto it = activeInfoCBPidSet_.begin(); it != activeInfoCBPidSet_.end(); ++it) {
        activeInfoCBPids.push_back(*it);
    }
    return activeInfoCBPids;
}

bool ClientInfo::CallingService(int32_t pid)
{
    std::lock_guard<std::mutex> channelLock(channelMutex_);
    auto channelIt = channelMap_.find(pid);
    if (channelIt != channelMap_.end()) {
        return false;
    }
    SEN_HILOGD("Pid is not exists in channelMap");
    std::lock_guard<std::mutex> activeInfoCBPidLock(activeInfoCBPidMutex_);
    auto pidIt = activeInfoCBPidSet_.find(pid);
    if (pidIt != activeInfoCBPidSet_.end()) {
        return false;
    }
    SEN_HILOGD("Pid is not exists in activeInfoCBPidSet");
    return true;
}


int32_t ClientInfo::GetPidByTokenId(AccessTokenID tokenId)
{
    std::lock_guard<std::mutex> uidLock(uidMutex_);
    int32_t pid = INVALID_PID;
    auto iter = std::find_if(appThreadInfoMap_.begin(), appThreadInfoMap_.end(), [tokenId] (auto appThreadInfo) {
            return appThreadInfo.second.callerToken == tokenId;
        });
    if (iter != appThreadInfoMap_.end()) {
        pid = iter->second.pid;
    }
    return pid;
}

void ClientInfo::UpdatePermState(int32_t pid, int32_t sensorId, bool state)
{
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    auto it = clientMap_.find(sensorId);
    if (it == clientMap_.end()) {
        SEN_HILOGE("Cannot find sensorId:%{public}d", sensorId);
        return;
    }
    auto clientInfo = it->second.find(pid);
    if (clientInfo != it->second.end()) {
        clientInfo->second.SetPermState(state);
    }
}

void ClientInfo::ChangeSensorPerm(AccessTokenID tokenId, const std::string &permName, bool state)
{
    int32_t pid = GetPidByTokenId(tokenId);
    if (pid <= INVALID_PID) {
        SEN_HILOGE("Invalid pid:%{public}d", pid);
        return;
    }
    auto it = userGrantPermMap_.find(permName);
    if (it == userGrantPermMap_.end()) {
        SEN_HILOGE("Invalid permission name:%{public}s", permName.c_str());
        return;
    }
    for (int32_t sensorId : it->second) {
        UpdatePermState(pid, sensorId, state);
    }
}
} // namespace Sensors
} // namespace OHOS
