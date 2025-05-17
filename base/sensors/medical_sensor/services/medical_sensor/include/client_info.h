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

#ifndef CLIENT_INFO_H
#define CLIENT_INFO_H

#include <map>
#include <queue>
#include <unordered_map>
#include <vector>

#include "refbase.h"
#include "singleton.h"

#include "app_thread_info.h"
#include "iremote_object.h"
#include "nocopyable.h"
#include "medical_sensor_basic_data_channel.h"
#include "medical_sensor_basic_info.h"
#include "medical_sensor_channel_info.h"
#include "medical_native_type.h"

namespace OHOS {
namespace Sensors {
class ClientInfo : public Singleton<ClientInfo> {
public:
    ClientInfo() = default;
    virtual ~ClientInfo() = default;

    MedicalSensorState GetSensorState(uint32_t sensorId);
    MedicalSensorBasicInfo GetBestSensorInfo(uint32_t sensorId);
    bool OnlyCurPidSensorEnabled(uint32_t sensorId, int32_t pid);
    std::vector<sptr<MedicalSensorBasicDataChannel>> GetSensorChannel(uint32_t sensorId);
    std::vector<sptr<MedicalSensorBasicDataChannel>> GetSensorChannelByUid(int32_t uid);
    sptr<MedicalSensorBasicDataChannel> GetSensorChannelByPid(int32_t pid);
    bool UpdateSensorInfo(uint32_t sensorId, int32_t pid, const MedicalSensorBasicInfo &sensorInfo);
    void RemoveSubscriber(uint32_t sensorId, uint32_t pid);
    bool UpdateSensorChannel(int32_t pid, const sptr<MedicalSensorBasicDataChannel> &channel);
    bool UpdateUid(int32_t pid, int32_t uid, AccessTokenID callerToken);
    bool ClearSensorInfo(uint32_t sensorId);
    void ClearCurPidSensorInfo(uint32_t sensorId, int32_t pid);
    bool DestroySensorChannel(int32_t pid);
    bool DestroyUid(int32_t pid);
    MedicalSensorBasicInfo GetCurPidSensorInfo(uint32_t sensorId, int32_t pid);
    uint64_t ComputeBestPeriodCount(uint32_t sensorId, sptr<MedicalSensorBasicDataChannel> &channel);
    uint64_t ComputeBestFifoCount(uint32_t sensorId, sptr<MedicalSensorBasicDataChannel> &channel);
    int32_t GetStoreEvent(int32_t sensorId, struct SensorEvent &event);
    void StoreEvent(const struct SensorEvent &event);
    void ClearEvent();
    AppThreadInfo GetAppInfoByChannel(const sptr<MedicalSensorBasicDataChannel> &channel);
    bool SaveClientPid(const sptr<IRemoteObject> &sensorClient, int32_t pid);
    int32_t FindClientPid(const sptr<IRemoteObject> &sensorClient);
    void DestroyClientPid(const sptr<IRemoteObject> &sensorClient);
    std::vector<uint32_t> GetSensorIdByPid(int32_t pid);
    void GetSensorChannelInfo(std::vector<MedicalSensorChannelInfo> &channelInfo);
    void UpdateCmd(uint32_t sensorId, int32_t uid, int32_t cmdType);
    void DestroyCmd(int32_t uid);
    void UpdateDataQueue(int32_t sensorId, struct SensorEvent &event);
    std::unordered_map<uint32_t, std::queue<struct SensorEvent>> GetDataQueue();
    void ClearDataQueue(int32_t sensorId);

private:
    DISALLOW_COPY_AND_MOVE(ClientInfo);
    int32_t GetUidByPid(int32_t pid);
    std::vector<int32_t> GetCmdList(uint32_t sensorId, int32_t uid);
    std::mutex clientMutex_;
    std::mutex channelMutex_;
    std::mutex eventMutex_;
    std::mutex uidMutex_;
    std::mutex clientPidMutex_;
    std::mutex cmdMutex_;
    std::mutex dataQueueMutex_;
    std::unordered_map<uint32_t, std::unordered_map<int32_t, MedicalSensorBasicInfo>> clientMap_;
    std::unordered_map<int32_t, sptr<MedicalSensorBasicDataChannel>> channelMap_;
    std::unordered_map<int32_t, struct SensorEvent> storedEvent_;
    std::unordered_map<int32_t, AppThreadInfo> appThreadInfoMap_;
    std::map<sptr<IRemoteObject>, int32_t> clientPidMap_;
    std::unordered_map<uint32_t, std::unordered_map<int32_t, std::vector<int32_t>>> cmdMap_;
    std::unordered_map<uint32_t, std::queue<struct SensorEvent>> dataQueue_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // CLIENT_INFO_H
