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

#include "sensor_service_stub.h"

#include <string>
#include <sys/socket.h>
#include <tokenid_kit.h>
#include <unistd.h>
#include <vector>

#include "accesstoken_kit.h"
#include "hisysevent.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "permission_util.h"
#include "sensor_client_proxy.h"
#include "sensor_errors.h"
#include "sensor_parcel.h"

#undef LOG_TAG
#define LOG_TAG "SensorServiceStub"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

SensorServiceStub::SensorServiceStub() {}

SensorServiceStub::~SensorServiceStub() {}

int32_t SensorServiceStub::ProcessRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    switch (code) {
        case static_cast<int32_t>(SensorInterfaceCode::ENABLE_SENSOR): {
            return SensorEnableInner(data, reply);
        }
        case static_cast<int32_t>(SensorInterfaceCode::DISABLE_SENSOR): {
            return SensorDisableInner(data, reply);
        }
        case static_cast<int32_t>(SensorInterfaceCode::GET_SENSOR_LIST): {
            return GetAllSensorsInner(data, reply);
        }
        case static_cast<int32_t>(SensorInterfaceCode::TRANSFER_DATA_CHANNEL): {
            return CreateDataChannelInner(data, reply);
        }
        case static_cast<int32_t>(SensorInterfaceCode::DESTROY_SENSOR_CHANNEL): {
            return DestroyDataChannelInner(data, reply);
        }
        case static_cast<int32_t>(SensorInterfaceCode::SUSPEND_SENSORS): {
            return SuspendSensorsInner(data, reply);
        }
        case static_cast<int32_t>(SensorInterfaceCode::RESUME_SENSORS): {
            return ResumeSensorsInner(data, reply);
        }
        case static_cast<int32_t>(SensorInterfaceCode::GET_ACTIVE_INFO_LIST): {
            return GetActiveInfoListInner(data, reply);
        }
        case static_cast<int32_t>(SensorInterfaceCode::CREATE_SOCKET_CHANNEL): {
            return CreateSocketChannelInner(data, reply);
        }
        case static_cast<int32_t>(SensorInterfaceCode::DESTROY_SOCKET_CHANNEL): {
            return DestroySocketChannelInner(data, reply);
        }
        case static_cast<int32_t>(SensorInterfaceCode::ENABLE_ACTIVE_INFO_CB): {
            return EnableActiveInfoCBInner(data, reply);
        }
        case static_cast<int32_t>(SensorInterfaceCode::DISABLE_ACTIVE_INFO_CB): {
            return DisableActiveInfoCBInner(data, reply);
        }
        case static_cast<int32_t>(SensorInterfaceCode::RESET_SENSORS): {
            return ResetSensorsInner(data, reply);
        }
        default: {
            SEN_HILOGD("No member func supporting, applying default process");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

int32_t SensorServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                           MessageOption &option)
{
    SEN_HILOGD("Begin, cmd:%{public}u", code);
    std::u16string descriptor = SensorServiceStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        SEN_HILOGE("Client and service descriptors are inconsistent");
        return OBJECT_NULL;
    }
    return ProcessRemoteRequest(code, data, reply, option);
}

bool SensorServiceStub::IsSystemServiceCalling()
{
    const auto tokenId = IPCSkeleton::GetCallingTokenID();
    const auto flag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (flag == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE ||
        flag == Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL) {
        SEN_HILOGD("system service calling, tokenId: %{public}u, flag: %{public}u", tokenId, flag);
        return true;
    }
    return false;
}

bool SensorServiceStub::IsSystemCalling()
{
    if (IsSystemServiceCalling()) {
        return true;
    }
    return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(IPCSkeleton::GetCallingFullTokenID());
}

ErrCode SensorServiceStub::SensorEnableInner(MessageParcel &data, MessageParcel &reply)
{
    (void)reply;
    int32_t sensorId;
    READINT32(data, sensorId, READ_PARCEL_ERR);
    if ((sensorId == SENSOR_TYPE_ID_COLOR || sensorId == SENSOR_TYPE_ID_SAR) && !IsSystemCalling()) {
        SEN_HILOGE("Permission check failed. A non-system application uses the system API");
        return NON_SYSTEM_API;
    }
    PermissionUtil &permissionUtil = PermissionUtil::GetInstance();
    int32_t ret = permissionUtil.CheckSensorPermission(GetCallingTokenID(), sensorId);
    if (ret != PERMISSION_GRANTED) {
        HiSysEventWrite(HiSysEvent::Domain::SENSOR, "VERIFY_ACCESS_TOKEN_FAIL",
            HiSysEvent::EventType::SECURITY, "PKG_NAME", "SensorEnableInner", "ERROR_CODE", ret);
        SEN_HILOGE("sensorId:%{public}d grant failed, result:%{public}d", sensorId, ret);
        return PERMISSION_DENIED;
    }
    int64_t samplingPeriodNs;
    int64_t maxReportDelayNs;
    READINT64(data, samplingPeriodNs, READ_PARCEL_ERR);
    READINT64(data, maxReportDelayNs, READ_PARCEL_ERR);
    return EnableSensor(sensorId, samplingPeriodNs, maxReportDelayNs);
}

ErrCode SensorServiceStub::SensorDisableInner(MessageParcel &data, MessageParcel &reply)
{
    (void)reply;
    int32_t sensorId;
    READINT32(data, sensorId, READ_PARCEL_ERR);
    if ((sensorId == SENSOR_TYPE_ID_COLOR || sensorId == SENSOR_TYPE_ID_SAR) && !IsSystemCalling()) {
        SEN_HILOGE("Permission check failed. A non-system application uses the system API");
        return NON_SYSTEM_API;
    }
    PermissionUtil &permissionUtil = PermissionUtil::GetInstance();
    int32_t ret = permissionUtil.CheckSensorPermission(GetCallingTokenID(), sensorId);
    if (ret != PERMISSION_GRANTED) {
        HiSysEventWrite(HiSysEvent::Domain::SENSOR, "VERIFY_ACCESS_TOKEN_FAIL",
            HiSysEvent::EventType::SECURITY, "PKG_NAME", "SensorDisableInner", "ERROR_CODE", ret);
        SEN_HILOGE("sensorId:%{public}d grant failed, result:%{public}d", sensorId, ret);
        return PERMISSION_DENIED;
    }
    return DisableSensor(sensorId);
}

ErrCode SensorServiceStub::GetAllSensorsInner(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    std::vector<Sensor> sensors = GetSensorList();
    uint32_t sensorCount = static_cast<uint32_t>(sensors.size());
    if (sensorCount > MAX_SENSOR_COUNT) {
        SEN_HILOGD("SensorCount:%{public}u", sensorCount);
        sensorCount = MAX_SENSOR_COUNT;
    }
    WRITEUINT32(reply, sensorCount, WRITE_PARCEL_ERR);
    for (uint32_t i = 0; i < sensorCount; ++i) {
        if (!sensors[i].Marshalling(reply)) {
            SEN_HILOGE("Sensor %{public}u marshalling failed", i);
            return GET_SENSOR_LIST_ERR;
        }
    }
    return NO_ERROR;
}

ErrCode SensorServiceStub::CreateDataChannelInner(MessageParcel &data, MessageParcel &reply)
{
    (void)reply;
    sptr<SensorBasicDataChannel> sensorChannel = new (std::nothrow) SensorBasicDataChannel();
    CHKPR(sensorChannel, OBJECT_NULL);
    auto ret = sensorChannel->CreateSensorBasicChannel(data);
    if (ret != ERR_OK) {
        SEN_HILOGE("CreateSensorBasicChannel ret:%{public}d", ret);
        return OBJECT_NULL;
    }
    sptr<IRemoteObject> sensorClient = data.ReadRemoteObject();
    CHKPR(sensorClient, OBJECT_NULL);
    return TransferDataChannel(sensorChannel, sensorClient);
}

ErrCode SensorServiceStub::DestroyDataChannelInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> sensorClient = data.ReadRemoteObject();
    CHKPR(sensorClient, OBJECT_NULL);
    return DestroySensorChannel(sensorClient);
}

ErrCode SensorServiceStub::SuspendSensorsInner(MessageParcel &data, MessageParcel &reply)
{
    PermissionUtil &permissionUtil = PermissionUtil::GetInstance();
    if (!permissionUtil.IsNativeToken(GetCallingTokenID())) {
        SEN_HILOGE("TokenType is not TOKEN_NATIVE");
        return PERMISSION_DENIED;
    }
    int32_t ret = permissionUtil.CheckManageSensorPermission(GetCallingTokenID());
    if (ret != PERMISSION_GRANTED) {
        SEN_HILOGE("Check manage sensor permission failed, ret:%{public}d", ret);
        return PERMISSION_DENIED;
    }
    (void)reply;
    int32_t pid;
    READINT32(data, pid, READ_PARCEL_ERR);
    return SuspendSensors(pid);
}

ErrCode SensorServiceStub::ResumeSensorsInner(MessageParcel &data, MessageParcel &reply)
{
    PermissionUtil &permissionUtil = PermissionUtil::GetInstance();
    if (!permissionUtil.IsNativeToken(GetCallingTokenID())) {
        SEN_HILOGE("TokenType is not TOKEN_NATIVE");
        return PERMISSION_DENIED;
    }
    int32_t ret = permissionUtil.CheckManageSensorPermission(GetCallingTokenID());
    if (ret != PERMISSION_GRANTED) {
        SEN_HILOGE("Check manage sensor permission failed, ret:%{public}d", ret);
        return PERMISSION_DENIED;
    }
    (void)reply;
    int32_t pid;
    READINT32(data, pid, READ_PARCEL_ERR);
    return ResumeSensors(pid);
}

ErrCode SensorServiceStub::GetActiveInfoListInner(MessageParcel &data, MessageParcel &reply)
{
    PermissionUtil &permissionUtil = PermissionUtil::GetInstance();
    if (!permissionUtil.IsNativeToken(GetCallingTokenID())) {
        SEN_HILOGE("TokenType is not TOKEN_NATIVE");
        return PERMISSION_DENIED;
    }
    int32_t pid;
    READINT32(data, pid, READ_PARCEL_ERR);
    std::vector<ActiveInfo> activeInfoList;
    int32_t ret = GetActiveInfoList(pid, activeInfoList);
    if (ret != ERR_OK) {
        SEN_HILOGE("Get activeInfo list failed");
        return ret;
    }
    uint32_t activeInfoCount = static_cast<uint32_t>(activeInfoList.size());
    if (activeInfoCount > MAX_SENSOR_COUNT) {
        SEN_HILOGD("ActiveInfoCount:%{public}u", activeInfoCount);
        activeInfoCount = MAX_SENSOR_COUNT;
    }
    WRITEUINT32(reply, activeInfoCount, WRITE_PARCEL_ERR);
    for (uint32_t i = 0; i < activeInfoCount; ++i) {
        if (!activeInfoList[i].Marshalling(reply)) {
            SEN_HILOGE("ActiveInfo %{public}u marshalling failed", i);
            return WRITE_PARCEL_ERR;
        }
    }
    return ERR_OK;
}

ErrCode SensorServiceStub::CreateSocketChannelInner(MessageParcel &data, MessageParcel &reply)
{
    PermissionUtil &permissionUtil = PermissionUtil::GetInstance();
    if (!permissionUtil.IsNativeToken(GetCallingTokenID())) {
        SEN_HILOGE("TokenType is not TOKEN_NATIVE");
        return PERMISSION_DENIED;
    }
    sptr<IRemoteObject> sensorClient = data.ReadRemoteObject();
    CHKPR(sensorClient, INVALID_POINTER);
    int32_t clientFd = -1;
    int32_t ret = CreateSocketChannel(sensorClient, clientFd);
    if (ret != ERR_OK) {
        SEN_HILOGE("Create socket channel failed");
        return ret;
    }
    if (!reply.WriteFileDescriptor(clientFd)) {
        SEN_HILOGE("Parcel write file descriptor failed");
        close(clientFd);
        return WRITE_PARCEL_ERR;
    }
    close(clientFd);
    return ERR_OK;
}

ErrCode SensorServiceStub::DestroySocketChannelInner(MessageParcel &data, MessageParcel &reply)
{
    PermissionUtil &permissionUtil = PermissionUtil::GetInstance();
    if (!permissionUtil.IsNativeToken(GetCallingTokenID())) {
        SEN_HILOGE("TokenType is not TOKEN_NATIVE");
        return PERMISSION_DENIED;
    }
    sptr<IRemoteObject> sensorClient = data.ReadRemoteObject();
    CHKPR(sensorClient, INVALID_POINTER);
    int32_t ret = DestroySocketChannel(sensorClient);
    if (ret != ERR_OK) {
        SEN_HILOGE("Destroy socket channel failed");
        return ret;
    }
    return ERR_OK;
}

ErrCode SensorServiceStub::EnableActiveInfoCBInner(MessageParcel &data, MessageParcel &reply)
{
    PermissionUtil &permissionUtil = PermissionUtil::GetInstance();
    if (!permissionUtil.IsNativeToken(GetCallingTokenID())) {
        SEN_HILOGE("TokenType is not TOKEN_NATIVE");
        return PERMISSION_DENIED;
    }
    return EnableActiveInfoCB();
}

ErrCode SensorServiceStub::DisableActiveInfoCBInner(MessageParcel &data, MessageParcel &reply)
{
    PermissionUtil &permissionUtil = PermissionUtil::GetInstance();
    if (!permissionUtil.IsNativeToken(GetCallingTokenID())) {
        SEN_HILOGE("TokenType is not TOKEN_NATIVE");
        return PERMISSION_DENIED;
    }
    return DisableActiveInfoCB();
}

ErrCode SensorServiceStub::ResetSensorsInner(MessageParcel &data, MessageParcel &reply)
{
    PermissionUtil &permissionUtil = PermissionUtil::GetInstance();
    if (!permissionUtil.IsNativeToken(GetCallingTokenID())) {
        SEN_HILOGE("TokenType is not TOKEN_NATIVE");
        return PERMISSION_DENIED;
    }
    int32_t ret = permissionUtil.CheckManageSensorPermission(GetCallingTokenID());
    if (ret != PERMISSION_GRANTED) {
        SEN_HILOGE("Check manage sensor permission failed, ret:%{public}d", ret);
        return PERMISSION_DENIED;
    }
    return ResetSensors();
}
} // namespace Sensors
} // namespace OHOS
