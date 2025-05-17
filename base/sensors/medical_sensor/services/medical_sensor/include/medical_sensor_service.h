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

#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include <mutex>
#include <thread>
#include <unordered_map>

#include "client_info.h"
#include "death_recipient_template.h"
#include "nocopyable.h"
#include "medical_sensor_manager.h"
#include "medical_sensor_service_stub.h"
#include "system_ability.h"
#include "medical_native_type.h"
#include "sensor_hdi_connection.h"

namespace OHOS {
namespace Sensors {
enum class MedicalSensorServiceState {
    STATE_STOPPED,
    STATE_RUNNING,
};

class MedicalSensorService : public SystemAbility, public MedicalSensorServiceStub {
    DECLARE_SYSTEM_ABILITY(MedicalSensorService)
public:
    explicit MedicalSensorService(int32_t systemAbilityId, bool runOnCreate = false);

    virtual ~MedicalSensorService() = default;

    void OnDump() override;

    void OnStart() override;

    void OnStop() override;

    int Dump(int fd, const std::vector<std::u16string> &args) override;

    ErrCode EnableSensor(uint32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs) override;

    ErrCode DisableSensor(uint32_t sensorId) override;

    ErrCode SetOption(uint32_t sensorId, uint32_t opt) override;

    int32_t GetSensorState(uint32_t sensorId) override;

    ErrCode RunCommand(uint32_t sensorId, uint32_t cmdType, uint32_t params) override;

    std::vector<MedicalSensor> GetSensorList() override;

    ErrCode TransferDataChannel(const sptr<MedicalSensorBasicDataChannel> &sensorBasicDataChannel,
                                const sptr<IRemoteObject> &sensorClient) override;

    ErrCode DestroySensorChannel(sptr<IRemoteObject> sensorClient) override;

    void ProcessDeathObserver(const wptr<IRemoteObject> &object);

private:
    DISALLOW_COPY_AND_MOVE(MedicalSensorService);
    void RegisterClientDeathRecipient(sptr<IRemoteObject> sensorClient, int32_t pid);
    void UnregisterClientDeathRecipient(sptr<IRemoteObject> sensorClient);
    bool InitInterface();
    bool InitDataCache();
    bool InitSensorList();
    void ReportOnChangeData(uint32_t sensorId);
    void ReportSensorUsedInfo(uint32_t sensorId, bool enable);
    MedicalSensorServiceState state_;
    std::mutex serviceLock_;
    std::mutex sensorsMutex_;
    std::mutex sensorMapMutex_;
    std::vector<MedicalSensor> sensors_;
    std::unordered_map<uint32_t, MedicalSensor> sensorMap_;
    SensorHdiConnection &sensorHdiConnection_ = SensorHdiConnection::GetInstance();
    ClientInfo &clientInfo_ = ClientInfo::GetInstance();
    MedicalSensorManager &sensorManager_ = MedicalSensorManager::GetInstance();
    FlushInfoRecord &flushInfo_ = FlushInfoRecord::GetInstance();
    sptr<MedicalSensorDataProcesser> sensorDataProcesser_;
    sptr<ReportDataCache> reportDataCache_;
    std::mutex uidLock_;
    // death recipient of sensor client
    sptr<IRemoteObject::DeathRecipient> clientDeathObserver_;
    ErrCode SaveSubscriber(uint32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs);
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // SENSOR_SERVICE_H
