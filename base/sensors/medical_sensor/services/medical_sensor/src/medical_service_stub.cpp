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

#include "medical_sensor_service_stub.h"

#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "permission_util.h"
#include "medical_errors.h"
#include "medical_log_domain.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_SERVICE, "MedicalSensorServiceStub" };
}  // namespace

MedicalSensorServiceStub::MedicalSensorServiceStub()
{
    baseFuncs_[ENABLE_SENSOR] = &MedicalSensorServiceStub::AfeEnableInner;
    baseFuncs_[DISABLE_SENSOR] = &MedicalSensorServiceStub::AfeDisableInner;
    baseFuncs_[GET_SENSOR_STATE] = &MedicalSensorServiceStub::GetAfeStateInner;
    baseFuncs_[RUN_COMMAND] = &MedicalSensorServiceStub::RunCommandInner;
    baseFuncs_[GET_SENSOR_LIST] = &MedicalSensorServiceStub::GetAllSensorsInner;
    baseFuncs_[TRANSFER_DATA_CHANNEL] = &MedicalSensorServiceStub::CreateDataChannelInner;
    baseFuncs_[DESTROY_SENSOR_CHANNEL] = &MedicalSensorServiceStub::DestroyDataChannelInner;
    baseFuncs_[SET_OPTION] = &MedicalSensorServiceStub::AfeSetOptionInner;
}

MedicalSensorServiceStub::~MedicalSensorServiceStub()
{
    baseFuncs_.clear();
}

int32_t MedicalSensorServiceStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HiLog::Debug(LABEL, "%{public}s begin, cmd : %{public}u", __func__, code);
    std::u16string descriptor = MedicalSensorServiceStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HiLog::Error(LABEL, "%{public}s client and service descriptors are inconsistent", __func__);
        return OBJECT_NULL;
    }
    auto itFunc = baseFuncs_.find(code);
    if (itFunc != baseFuncs_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    HiLog::Debug(LABEL, "%{public}s no member func supporting, applying default process", __func__);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

ErrCode MedicalSensorServiceStub::AfeEnableInner(MessageParcel &data, MessageParcel &reply)
{
    (void)reply;
    uint32_t sensorId = data.ReadUint32();
    PermissionUtil &permissionUtil = PermissionUtil::GetInstance();
    int32_t ret = permissionUtil.CheckSensorPermission(GetCallingTokenID(), sensorId);
    if (ret != PERMISSION_GRANTED) {
        HiLog::Error(LABEL, "%{public}s: sensorId:%{public}u permission failed, result:%{public}d",
            __func__, sensorId, ret);
        return ERR_PERMISSION_DENIED;
    }
    return EnableSensor(sensorId, data.ReadInt64(), data.ReadInt64());
}

ErrCode MedicalSensorServiceStub::AfeDisableInner(MessageParcel &data, MessageParcel &reply)
{
    (void)reply;
    uint32_t sensorId = data.ReadUint32();
    PermissionUtil &permissionUtil = PermissionUtil::GetInstance();
    int32_t ret = permissionUtil.CheckSensorPermission(GetCallingTokenID(), sensorId);
    if (ret != PERMISSION_GRANTED) {
        HiLog::Error(LABEL, "%{public}s: sensorId:%{public}u permission failed, result:%{public}d",
            __func__, sensorId, ret);
        return ERR_PERMISSION_DENIED;
    }
    return DisableSensor(sensorId);
}

ErrCode MedicalSensorServiceStub::AfeSetOptionInner(MessageParcel &data, MessageParcel &reply)
{
    (void)reply;
    uint32_t sensorId = data.ReadUint32();
    uint32_t opt = data.ReadUint32();
    PermissionUtil &permissionUtil = PermissionUtil::GetInstance();
    int32_t ret = permissionUtil.CheckSensorPermission(GetCallingTokenID(), sensorId);
    if (ret != PERMISSION_GRANTED) {
        HiLog::Error(LABEL, "%{public}s: sensorId:%{public}u permission failed, result:%{public}d",
            __func__, sensorId, ret);
        return ERR_PERMISSION_DENIED;
    }
    return SetOption(sensorId, opt);
}

ErrCode MedicalSensorServiceStub::GetAfeStateInner(MessageParcel &data, MessageParcel &reply)
{
    (void)reply;
    uint32_t sensorId = data.ReadUint32();
    PermissionUtil &permissionUtil = PermissionUtil::GetInstance();
    int32_t ret = permissionUtil.CheckSensorPermission(GetCallingTokenID(), sensorId);
    if (ret != PERMISSION_GRANTED) {
        HiLog::Error(LABEL, "%{public}s: sensorId:%{public}u permission failed, result:%{public}d",
            __func__, sensorId, ret);
        return ERR_PERMISSION_DENIED;
    }
    return GetSensorState(sensorId);
}

ErrCode MedicalSensorServiceStub::RunCommandInner(MessageParcel &data, MessageParcel &reply)
{
    (void)reply;
    uint32_t sensorId = data.ReadUint32();
    PermissionUtil &permissionUtil = PermissionUtil::GetInstance();
    int32_t ret = permissionUtil.CheckSensorPermission(GetCallingTokenID(), sensorId);
    if (ret != PERMISSION_GRANTED) {
        HiLog::Error(LABEL, "%{public}s: sensorId:%{public}u permission failed, result:%{public}d",
            __func__, sensorId, ret);
        return ERR_PERMISSION_DENIED;
    }
    return RunCommand(sensorId, data.ReadUint32(), data.ReadUint32());
}

ErrCode MedicalSensorServiceStub::GetAllSensorsInner(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    std::vector<MedicalSensor> sensors(GetSensorList());
    int32_t sensorCount = int32_t { sensors.size() };
    HiLog::Debug(LABEL, "%{public}s sensorCount : %{public}d", __func__, sensorCount);
    reply.WriteInt32(sensorCount);
    for (int32_t i = 0; i < sensorCount; i++) {
        bool flag = sensors[i].Marshalling(reply);
        if (!flag) {
            HiLog::Error(LABEL, "Marshalling sensor %{public}d failed", i);
            return GET_SENSOR_LIST_ERR;
        }
    }
    return NO_ERROR;
}

ErrCode MedicalSensorServiceStub::CreateDataChannelInner(MessageParcel &data, MessageParcel &reply)
{
    HiLog::Debug(LABEL, "%{public}s begin", __func__);

    (void)reply;
    sptr<MedicalSensorBasicDataChannel> afeChannel = new (std::nothrow) MedicalSensorBasicDataChannel();
    if (afeChannel == nullptr) {
        HiLog::Error(LABEL, "%{public}s afe.hannel cannot be null", __func__);
        return OBJECT_NULL;
    }
    auto ret = afeChannel->CreateSensorBasicChannel(data);
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s CreateSensorBasicChannel ret : %{public}d", __func__, ret);
        return OBJECT_NULL;
    }
    sptr<IRemoteObject> afeClient = data.ReadRemoteObject();
    if (afeClient == nullptr) {
        HiLog::Error(LABEL, "%{public}s afeClient cannot be null", __func__);
        return OBJECT_NULL;
    }

    return TransferDataChannel(afeChannel, afeClient);
}

ErrCode MedicalSensorServiceStub::DestroyDataChannelInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> afeClient = data.ReadRemoteObject();
    if (afeClient == nullptr) {
        HiLog::Error(LABEL, "%{public}s afeClient cannot be null", __func__);
        return OBJECT_NULL;
    }
    return DestroySensorChannel(afeClient);
}
}  // namespace Sensors
}  // namespace OHOS
