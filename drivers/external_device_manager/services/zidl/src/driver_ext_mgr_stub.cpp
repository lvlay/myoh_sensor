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

#include "driver_ext_mgr_stub.h"
#include <cinttypes>

#include "hilog_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace ExternalDeviceManager {
int DriverExtMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    EDM_LOGD(MODULE_FRAMEWORK, "cmd:%u, flags:%d", code, option.GetFlags());
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        EDM_LOGE(MODULE_FRAMEWORK, "remote descriptor is not matched");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    switch (code) {
        case static_cast<uint32_t>(DriverExtMgrInterfaceCode::QUERY_DEVICE):
            return OnQueryDevice(data, reply, option);
        case static_cast<uint32_t>(DriverExtMgrInterfaceCode::BIND_DEVICE):
            return OnBindDevice(data, reply, option);
        case static_cast<uint32_t>(DriverExtMgrInterfaceCode::UNBIND_DEVICE):
            return OnUnBindDevice(data, reply, option);
        case static_cast<uint32_t>(DriverExtMgrInterfaceCode::QUERY_DEVICE_INFO):
            return OnQueryDeviceInfo(data, reply, option);
        case static_cast<uint32_t>(DriverExtMgrInterfaceCode::QUERY_DRIVER_INFO):
            return OnQueryDriverInfo(data, reply, option);
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t DriverExtMgrStub::OnQueryDevice(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    uint32_t busType = 0;
    if (!data.ReadUint32(busType)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to read busType");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    std::vector<std::shared_ptr<DeviceData>> devices;
    UsbErrCode ret = QueryDevice(busType, devices);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to call QueryDevice function:%{public}d", static_cast<int32_t>(ret));
        return ret;
    }

    if (!reply.WriteUint64(static_cast<uint64_t>(devices.size()))) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write size of devices");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    for (uint64_t i = 0; i < devices.size(); i++) {
        if (devices[i] == nullptr) {
            EDM_LOGE(MODULE_FRAMEWORK, "invalid %{public}016" PRIX64 " device", i);
            return UsbErrCode::EDM_ERR_INVALID_PARAM;
        }

        if (!devices[i]->Marshalling(reply)) {
            EDM_LOGE(MODULE_FRAMEWORK, "failed to write %{public}016" PRIX64 " device", i);
            return UsbErrCode::EDM_ERR_INVALID_PARAM;
        }
    }

    return UsbErrCode::EDM_OK;
}

int32_t DriverExtMgrStub::OnBindDevice(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    uint64_t deviceId = 0;
    if (!data.ReadUint64(deviceId)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to read deviceId");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to read remote object of connectCallback");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    sptr<IDriverExtMgrCallback> connectCallback = iface_cast<IDriverExtMgrCallback>(remote);
    if (connectCallback == nullptr) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to create connectCallback object");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    UsbErrCode ret = BindDevice(deviceId, connectCallback);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to call BindDevice function:%{public}d", static_cast<int32_t>(ret));
        return ret;
    }

    return UsbErrCode::EDM_OK;
}

int32_t DriverExtMgrStub::OnUnBindDevice(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    uint64_t deviceId = 0;
    if (!data.ReadUint64(deviceId)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to read deviceId");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    UsbErrCode ret = UnBindDevice(deviceId);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to call UnBindDevice function:%{public}d", static_cast<int32_t>(ret));
        return ret;
    }

    return UsbErrCode::EDM_OK;
}

int32_t DriverExtMgrStub::OnQueryDeviceInfo(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    EDM_LOGD(MODULE_FRAMEWORK, "OnQueryDeviceInfo start");
    bool isByDeviceId = false;
    if (!data.ReadBool(isByDeviceId)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to read isByDeviceId");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    uint64_t deviceId = 0;
    if (isByDeviceId && !data.ReadUint64(deviceId)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to read deviceId");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    std::vector<std::shared_ptr<DeviceInfoData>> deviceInfos;
    int32_t ret = QueryDeviceInfo(deviceInfos, isByDeviceId, deviceId);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to call QueryDeviceInfo");
        return ret;
    }

    uint64_t deviceInfoSize = static_cast<uint64_t>(deviceInfos.size());
    if (!reply.WriteUint64(deviceInfoSize)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write size of deviceInfos");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    for (uint64_t i = 0; i < deviceInfoSize; i++) {
        if (deviceInfos[i] == nullptr) {
            EDM_LOGE(MODULE_FRAMEWORK, "invalid deviceInfo, index: %{public}" PRIu64, i);
            return UsbErrCode::EDM_ERR_INVALID_PARAM;
        }

        if (!deviceInfos[i]->Marshalling(reply)) {
            EDM_LOGE(MODULE_FRAMEWORK, "failed to write deviceInfo, index: %{public}" PRIu64, i);
            return UsbErrCode::EDM_ERR_INVALID_PARAM;
        }
    }

    EDM_LOGD(MODULE_FRAMEWORK, "OnQueryDeviceInfo end");

    return UsbErrCode::EDM_OK;
}

int32_t DriverExtMgrStub::OnQueryDriverInfo(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    EDM_LOGD(MODULE_FRAMEWORK, "OnQueryDriverInfo start");

    bool isByDriverUid = false;
    if (!data.ReadBool(isByDriverUid)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to read isByDriverUid");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    std::string driverUid = "";
    if (isByDriverUid && !data.ReadString(driverUid)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to read driverUid");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    std::vector<std::shared_ptr<DriverInfoData>> driverInfos;
    int32_t ret = QueryDriverInfo(driverInfos, isByDriverUid, driverUid);
    if (ret != UsbErrCode::EDM_OK) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to call QueryDriverInfo");
        return ret;
    }

    uint64_t driverInfoSize = static_cast<uint64_t>(driverInfos.size());
    if (!reply.WriteUint64(driverInfoSize)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to write size of driverInfos");
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    for (uint64_t i = 0; i < driverInfoSize; i++) {
        if (driverInfos[i] == nullptr) {
            EDM_LOGE(MODULE_FRAMEWORK, "invalid driverInfo, index: %{public}" PRIu64, i);
            return UsbErrCode::EDM_ERR_INVALID_PARAM;
        }

        if (!driverInfos[i]->Marshalling(reply)) {
            EDM_LOGE(MODULE_FRAMEWORK, "failed to write driverInfo, index: %{public}" PRIu64, i);
            return UsbErrCode::EDM_ERR_INVALID_PARAM;
        }
    }

    EDM_LOGD(MODULE_FRAMEWORK, "OnQueryDriverInfo end");

    return UsbErrCode::EDM_OK;
}
} // namespace ExternalDeviceManager
} // namespace OHOS
