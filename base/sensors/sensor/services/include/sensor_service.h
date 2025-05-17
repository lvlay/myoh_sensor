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

#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include <mutex>
#include <thread>
#include <unordered_map>

#include "nocopyable.h"
#include "system_ability.h"

#include "client_info.h"
#include "death_recipient_template.h"
#include "sensor_data_event.h"
#include "sensor_delayed_sp_singleton.h"
#include "sensor_manager.h"
#include "sensor_power_policy.h"
#include "sensor_service_stub.h"
#include "stream_server.h"
#ifdef HDF_DRIVERS_INTERFACE_SENSOR
#include "sensor_hdi_connection.h"
#endif // HDF_DRIVERS_INTERFACE_SENSOR

namespace OHOS {
namespace Sensors {
enum class SensorServiceState {
    STATE_STOPPED,
    STATE_RUNNING,
};

class SensorService : public SystemAbility, public StreamServer, public SensorServiceStub {
    DECLARE_SYSTEM_ABILITY(SensorService)
    SENSOR_DECLARE_DELAYED_SP_SINGLETON(SensorService);

public:
    void OnDump() override;
    void OnStart() override;
    void OnStop() override;
    int Dump(int fd, const std::vector<std::u16string> &args) override;
    ErrCode EnableSensor(int32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs) override;
    ErrCode DisableSensor(int32_t sensorId) override;
    std::vector<Sensor> GetSensorList() override;
    ErrCode TransferDataChannel(const sptr<SensorBasicDataChannel> &sensorBasicDataChannel,
                                const sptr<IRemoteObject> &sensorClient) override;
    ErrCode DestroySensorChannel(sptr<IRemoteObject> sensorClient) override;
    void ProcessDeathObserver(const wptr<IRemoteObject> &object);
    ErrCode SuspendSensors(int32_t pid) override;
    ErrCode ResumeSensors(int32_t pid) override;
    ErrCode GetActiveInfoList(int32_t pid, std::vector<ActiveInfo> &activeInfoList) override;
    ErrCode CreateSocketChannel(sptr<IRemoteObject> sensorClient, int32_t &clientFd) override;
    ErrCode DestroySocketChannel(sptr<IRemoteObject> sensorClient) override;
    ErrCode EnableActiveInfoCB() override;
    ErrCode DisableActiveInfoCB() override;
    ErrCode ResetSensors() override;

private:
    DISALLOW_COPY_AND_MOVE(SensorService);
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    bool CheckParameter(int32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs);

    class PermStateChangeCb : public Security::AccessToken::PermStateChangeCallbackCustomize {
    public:
        PermStateChangeCb(const Security::AccessToken::PermStateChangeScope &scope,
            sptr<SensorService> server) : PermStateChangeCallbackCustomize(scope), server_(server) {}
        void PermStateChangeCallback(PermStateChangeInfo &result) override;

    private:
        sptr<SensorService> server_ = nullptr;
    };

    void RegisterClientDeathRecipient(sptr<IRemoteObject> sensorClient, int32_t pid);
    void UnregisterClientDeathRecipient(sptr<IRemoteObject> sensorClient);
    bool InitSensorPolicy();
    void ReportOnChangeData(int32_t sensorId);
    void ReportSensorSysEvent(int32_t sensorId, bool enable, int32_t pid);
    ErrCode DisableSensor(int32_t sensorId, int32_t pid);
    bool RegisterPermCallback(int32_t sensorId);
    void UnregisterPermCallback();
    void ReportActiveInfo(int32_t sensorId, int32_t pid);
    bool CheckSensorId(int32_t sensorId);
    SensorServiceState state_;
    std::mutex serviceLock_;
    std::mutex sensorsMutex_;
    std::mutex sensorMapMutex_;
    std::vector<Sensor> sensors_;
    std::unordered_map<int32_t, Sensor> sensorMap_;
#ifdef HDF_DRIVERS_INTERFACE_SENSOR
    bool InitInterface();
    bool InitDataCallback();
    bool InitSensorList();
    SensorHdiConnection &sensorHdiConnection_ = SensorHdiConnection::GetInstance();
    sptr<SensorDataProcesser> sensorDataProcesser_ = nullptr;
    sptr<ReportDataCallback> reportDataCallback_ = nullptr;
#endif // HDF_DRIVERS_INTERFACE_SENSOR
    ClientInfo &clientInfo_ = ClientInfo::GetInstance();
    SensorManager &sensorManager_ = SensorManager::GetInstance();
    std::mutex uidLock_;
    // death recipient of sensor client
    std::mutex clientDeathObserverMutex_;
    sptr<IRemoteObject::DeathRecipient> clientDeathObserver_ = nullptr;
    std::shared_ptr<PermStateChangeCb> permStateChangeCb_ = nullptr;
    ErrCode SaveSubscriber(int32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs);
    std::atomic_bool isReportActiveInfo_ = false;
};

#define POWER_POLICY SensorPowerPolicy::GetInstance()
}  // namespace Sensors
}  // namespace OHOS
#endif  // SENSOR_SERVICE_H
