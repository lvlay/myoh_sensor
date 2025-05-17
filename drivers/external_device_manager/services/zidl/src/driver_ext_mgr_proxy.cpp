/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "driver_ext_mgr_proxy.h"
#include <cinttypes>

#include "hilog_wrapper.h"
#include "message_parcel.h"
#include "securec.h"

namespace OHOS {
namespace ExternalDeviceManager {
UsbErrCode DriverExtMgrProxy::QueryDevice(uint32_t busType, std::vector<std::shared_ptr<DeviceData>> &devices)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        EDM_LOGE(MODULE_FRAMEWORK, "remote is nullptr");
        return UsbErrCode::EDM_ERR_INVALID_OBJECT;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write interface token");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    if (!data.WriteUint32(busType)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write busType");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(DriverExtMgrInterfaceCode::QUERY_DEVICE), data, reply, option);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_FRAMEWORK, "SendRequest is failed, ret: %{public}d", ret);
        return static_cast<UsbErrCode>(ret);
    }

    uint64_t deviceInfoSize = 0;
    if (!reply.ReadUint64(deviceInfoSize)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to read size of DeviceData");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    if (deviceInfoSize > devices.max_size()) {
        EDM_LOGE(MODULE_FRAMEWORK, "invalid size of DeviceData");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    for (uint64_t i = 0; i < deviceInfoSize; i++) {
        std::shared_ptr<DeviceData> device = DeviceData::UnMarshalling(reply);
        if (device == nullptr) {
            EDM_LOGE(MODULE_FRAMEWORK, "failed to read %{public}016" PRIX64 " device", i);
            return UsbErrCode::EDM_ERR_INVALID_PARAM;
        }
        devices.push_back(std::move(device));
    }

    return UsbErrCode::EDM_OK;
}

UsbErrCode DriverExtMgrProxy::BindDevice(uint64_t deviceId, const sptr<IDriverExtMgrCallback> &connectCallback)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        EDM_LOGE(MODULE_FRAMEWORK, "remote is nullptr");
        return UsbErrCode::EDM_ERR_INVALID_OBJECT;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write interface token");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    if (!data.WriteUint64(deviceId)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write deviceId");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    if (connectCallback == nullptr || !data.WriteRemoteObject(connectCallback->AsObject())) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write connectCallback object");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(DriverExtMgrInterfaceCode::BIND_DEVICE), data, reply, option);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_FRAMEWORK, "SendRequest is failed, ret: %{public}d", ret);
        return static_cast<UsbErrCode>(ret);
    }

    return UsbErrCode::EDM_OK;
}

UsbErrCode DriverExtMgrProxy::UnBindDevice(uint64_t deviceId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        EDM_LOGE(MODULE_FRAMEWORK, "remote is nullptr");
        return UsbErrCode::EDM_ERR_INVALID_OBJECT;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write interface token");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    if (!data.WriteUint64(deviceId)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write deviceId");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(DriverExtMgrInterfaceCode::UNBIND_DEVICE), data, reply, option);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_FRAMEWORK, "SendRequest is failed, ret: %{public}d", ret);
        return static_cast<UsbErrCode>(ret);
    }

    return UsbErrCode::EDM_OK;
}

UsbErrCode DriverExtMgrProxy::QueryDeviceInfo(std::vector<std::shared_ptr<DeviceInfoData>> &deviceInfos,
    bool isByDeviceId, const uint64_t deviceId)
{
    EDM_LOGD(MODULE_FRAMEWORK, "proxy QueryDeviceInfo start");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        EDM_LOGE(MODULE_FRAMEWORK, "remote is nullptr");
        return UsbErrCode::EDM_ERR_INVALID_OBJECT;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write interface token");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    if (!data.WriteBool(isByDeviceId)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write isByDeviceId");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    if (isByDeviceId && !data.WriteUint64(deviceId)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write deviceId");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(DriverExtMgrInterfaceCode::QUERY_DEVICE_INFO), data, reply, option);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_FRAMEWORK, "SendRequest is failed, ret: %{public}d", ret);
        return static_cast<UsbErrCode>(ret);
    }

    if (!DeviceInfoData::DeviceInfosUnMarshalling(reply, deviceInfos)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to read deviceInfos");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    EDM_LOGD(MODULE_FRAMEWORK, "proxy QueryDeviceInfo end");

    return UsbErrCode::EDM_OK;
}

UsbErrCode DriverExtMgrProxy::QueryDriverInfo(std::vector<std::shared_ptr<DriverInfoData>> &driverInfos,
    bool isByDriverUid, const std::string &driverUid)
{
    EDM_LOGD(MODULE_FRAMEWORK, "proxy QueryDriverInfo start");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        EDM_LOGE(MODULE_FRAMEWORK, "remote is nullptr");
        return UsbErrCode::EDM_ERR_INVALID_OBJECT;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write interface token");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    if (!data.WriteBool(isByDriverUid)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write isByDriverUid");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    if (isByDriverUid && !data.WriteString(driverUid)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write driverUid");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(DriverExtMgrInterfaceCode::QUERY_DRIVER_INFO), data, reply, option);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_FRAMEWORK, "SendRequest is failed, ret: %{public}d", ret);
        return static_cast<UsbErrCode>(ret);
    }

    if (!DriverInfoData::DriverInfosUnMarshalling(reply, driverInfos)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write driverInfos");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    EDM_LOGD(MODULE_FRAMEWORK, "proxy QueryDriverInfo end");

    return UsbErrCode::EDM_OK;
}
} // namespace ExternalDeviceManager
} // namespace OHOS
