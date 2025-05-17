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

#include "medical_sensor_service_proxy.h"

#include <vector>

#include "dmd_report.h"
#include "message_parcel.h"
#include "medical_errors.h"
#include "medical_log_domain.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

enum class FlushCmdId {
    FLUSH = 0,
    SET_MODE,
    RESERVED,
};

namespace {
constexpr HiLogLabel LABEL = {
    LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_FRAMEWORK, "MedicalSensorServiceProxy"
};
constexpr int32_t MAX_SENSOR_COUNT = 200;
}  // namespace

MedicalSensorServiceProxy::MedicalSensorServiceProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IMedicalSensorService>(impl)
{}

ErrCode MedicalSensorServiceProxy::EnableSensor(uint32_t sensorId, int64_t samplingPeriodNs, int64_t maxReportDelayNs)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(MedicalSensorServiceProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "%{public}s write descriptor failed", __func__);
        return WRITE_MSG_ERR;
    }
    if (!data.WriteUint32(sensorId)) {
        HiLog::Error(LABEL, "%{public}s write sensorId failed", __func__);
        return WRITE_MSG_ERR;
    }
    if (!data.WriteInt64(samplingPeriodNs)) {
        HiLog::Error(LABEL, "%{public}s write samplingPeriodNs failed", __func__);
        return WRITE_MSG_ERR;
    }
    if (!data.WriteInt64(maxReportDelayNs)) {
        HiLog::Error(LABEL, "%{public}s write maxReportDelayNs failed", __func__);
        return WRITE_MSG_ERR;
    }
    int32_t ret = Remote()->SendRequest(IMedicalSensorService::ENABLE_SENSOR, data, reply, option);
    if (ret != NO_ERROR) {
        DmdReport::ReportException(SENSOR_SERVICE_IPC_EXCEPTION, "EnableSensor", ret);
        HiLog::Error(LABEL, "%{public}s failed, ret : %{public}d", __func__, ret);
    }
    return static_cast<ErrCode>(ret);
}

ErrCode MedicalSensorServiceProxy::DisableSensor(uint32_t sensorId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(MedicalSensorServiceProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "%{public}s write descriptor failed", __func__);
        return WRITE_MSG_ERR;
    }
    if (!data.WriteUint32(sensorId)) {
        HiLog::Error(LABEL, "%{public}s write sensorId failed", __func__);
        return WRITE_MSG_ERR;
    }
    int32_t ret = Remote()->SendRequest(IMedicalSensorService::DISABLE_SENSOR, data, reply, option);
    if (ret != NO_ERROR) {
        DmdReport::ReportException(SENSOR_SERVICE_IPC_EXCEPTION, "DisableSensor", ret);
        HiLog::Error(LABEL, "%{public}s failed, ret : %{public}d", __func__, ret);
    }
    return static_cast<ErrCode>(ret);
}

ErrCode MedicalSensorServiceProxy::SetOption(uint32_t sensorId, uint32_t opt)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(MedicalSensorServiceProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "%{public}s write descriptor failed", __func__);
        return WRITE_MSG_ERR;
    }
    if (!data.WriteUint32(sensorId)) {
        HiLog::Error(LABEL, "%{public}s write sensorId failed", __func__);
        return WRITE_MSG_ERR;
    }
    if (!data.WriteUint32(opt)) {
        HiLog::Error(LABEL, "%{public}s write sensorId failed", __func__);
        return WRITE_MSG_ERR;
    }

    int32_t ret = Remote()->SendRequest(IMedicalSensorService::SET_OPTION, data, reply, option);
    if (ret != NO_ERROR) {
        DmdReport::ReportException(SENSOR_SERVICE_IPC_EXCEPTION, "DisableSensor", ret);
        HiLog::Error(LABEL, "%{public}s failed, ret : %{public}d", __func__, ret);
    }
    return static_cast<ErrCode>(ret);
}

int32_t MedicalSensorServiceProxy::GetSensorState(uint32_t sensorId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(MedicalSensorServiceProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "%{public}s write descriptor failed", __func__);
        return WRITE_MSG_ERR;
    }
    if (!data.WriteUint32(sensorId)) {
        HiLog::Error(LABEL, "%{public}s write sensorId failed", __func__);
        return WRITE_MSG_ERR;
    }
    int32_t ret = Remote()->SendRequest(IMedicalSensorService::GET_SENSOR_STATE, data, reply, option);
    if (ret != NO_ERROR) {
        DmdReport::ReportException(SENSOR_SERVICE_IPC_EXCEPTION, "GetSensorState", ret);
        HiLog::Error(LABEL, "%{public}s failed, ret : %{public}d", __func__, ret);
    }
    return static_cast<ErrCode>(ret);
}

ErrCode MedicalSensorServiceProxy::RunCommand(uint32_t sensorId, uint32_t cmdType, uint32_t params)
{
    if (cmdType > static_cast<uint32_t>(FlushCmdId::RESERVED)) {
        HiLog::Error(LABEL, "%{public}s failed, cmdType : %{public}u", __func__, cmdType);
        return CMD_TYPE_ERR;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(MedicalSensorServiceProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "%{public}s write descriptor failed", __func__);
        return WRITE_MSG_ERR;
    }
    if (!data.WriteUint32(sensorId)) {
        HiLog::Error(LABEL, "%{public}s write sensorId failed", __func__);
        return WRITE_MSG_ERR;
    }
    if (!data.WriteUint32(cmdType)) {
        HiLog::Error(LABEL, "%{public}s write cmdType failed", __func__);
        return WRITE_MSG_ERR;
    }
    if (!data.WriteUint32(params)) {
        HiLog::Error(LABEL, "%{public}s write params failed", __func__);
        return WRITE_MSG_ERR;
    }
    int32_t ret = Remote()->SendRequest(IMedicalSensorService::RUN_COMMAND, data, reply, option);
    if (ret != NO_ERROR) {
        DmdReport::ReportException(SENSOR_SERVICE_IPC_EXCEPTION, "RunCommand", ret);
        HiLog::Error(LABEL, "%{public}s failed, ret : %{public}d", __func__, ret);
    }
    return static_cast<ErrCode>(ret);
}

std::vector<MedicalSensor> MedicalSensorServiceProxy::GetSensorList()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::vector<MedicalSensor> sensors;
    if (!data.WriteInterfaceToken(MedicalSensorServiceProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "%{public}s write descriptor failed", __func__);
        return sensors;
    }
    int32_t ret = Remote()->SendRequest(IMedicalSensorService::GET_SENSOR_LIST, data, reply, option);
    if (ret != NO_ERROR) {
        DmdReport::ReportException(SENSOR_SERVICE_IPC_EXCEPTION, "GetSensorList", ret);
        HiLog::Error(LABEL, "%{public}s failed, ret : %{public}d", __func__, ret);
        return sensors;
    }

    int32_t sensorCount = reply.ReadInt32();
    HiLog::Debug(LABEL, "%{public}s  sensorCount : %{public}d", __func__, sensorCount);
    if (sensorCount > MAX_SENSOR_COUNT) {
        sensorCount = MAX_SENSOR_COUNT;
    }
    MedicalSensor sensor;
    for (int32_t i = 0; i < sensorCount; i++) {
        auto tmpSensor = sensor.Unmarshalling(reply);
        if (tmpSensor == nullptr) {
            continue;
        }
        sensors.push_back(*tmpSensor);
    }
    return sensors;
}

ErrCode MedicalSensorServiceProxy::TransferDataChannel(
    const sptr<MedicalSensorBasicDataChannel> &sensorBasicDataChannel, const sptr<IRemoteObject> &afeClient)
{
    HiLog::Debug(LABEL, "%{public}s sendFd: %{public}d", __func__, sensorBasicDataChannel->GetSendDataFd());
    if (sensorBasicDataChannel == nullptr || afeClient == nullptr) {
        HiLog::Error(LABEL, "%{public}s sensorBasicDataChannel or afeClient cannot be null", __func__);
        return OBJECT_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(MedicalSensorServiceProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "%{public}s write descriptor failed", __func__);
        return WRITE_MSG_ERR;
    }
    sensorBasicDataChannel->SendToBinder(data);
    if (!data.WriteRemoteObject(afeClient)) {
        HiLog::Error(LABEL, "%{public}s write afeClient failed", __func__);
        return WRITE_MSG_ERR;
    }

    int32_t ret = Remote()->SendRequest(IMedicalSensorService::TRANSFER_DATA_CHANNEL, data, reply, option);
    if (ret != NO_ERROR) {
        DmdReport::ReportException(SENSOR_SERVICE_IPC_EXCEPTION, "TransferDataChannel", ret);
        HiLog::Error(LABEL, "%{public}s failed, ret : %{public}d", __func__, ret);
    }
    sensorBasicDataChannel->CloseSendFd();
    return static_cast<ErrCode>(ret);
}

ErrCode MedicalSensorServiceProxy::DestroySensorChannel(sptr<IRemoteObject> afeClient)
{
    if (afeClient == nullptr) {
        HiLog::Error(LABEL, "%{public}s afeClient cannot be null", __func__);
        return OBJECT_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(MedicalSensorServiceProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "%{public}s write descriptor failed", __func__);
        return WRITE_MSG_ERR;
    }
    if (!data.WriteRemoteObject(afeClient)) {
        HiLog::Error(LABEL, "%{public}s write afeClient failed", __func__);
        return WRITE_MSG_ERR;
    }
    int32_t ret = Remote()->SendRequest(IMedicalSensorService::DESTROY_SENSOR_CHANNEL, data, reply, option);
    if (ret != NO_ERROR) {
        DmdReport::ReportException(SENSOR_SERVICE_IPC_EXCEPTION, "DestroySensorChannel", ret);
        HiLog::Error(LABEL, "%{public}s failed, ret : %{public}d", __func__, ret);
    }
    return static_cast<ErrCode>(ret);
}
}  // namespace Sensors
}  // namespace OHOS
