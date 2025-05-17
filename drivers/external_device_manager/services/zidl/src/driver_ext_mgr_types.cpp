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
#include "driver_ext_mgr_types.h"
#include <sstream>

#include "hilog_wrapper.h"

namespace OHOS {
namespace ExternalDeviceManager {
bool ErrMsg::Marshalling(MessageParcel &parcel) const
{
    if (!parcel.WriteInt32(errCode)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write errCode");
        return false;
    }

    if (!parcel.WriteString(msg)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write msg");
        return false;
    }
    return true;
}

bool ErrMsg::UnMarshalling(MessageParcel &parcel, ErrMsg &data)
{
    int32_t err = 0;
    if (!parcel.ReadInt32(err)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read errCode");
        return false;
    }
    data.errCode = static_cast<UsbErrCode>(err);

    if (!parcel.ReadString(data.msg)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read msg");
        return false;
    }
    return true;
}

bool DeviceData::Marshalling(MessageParcel &parcel) const
{
    if (!parcel.WriteUint32(static_cast<uint32_t>(busType))) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write busType");
        return false;
    }

    if (!parcel.WriteUint64(deviceId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write deviceId");
        return false;
    }

    if (!parcel.WriteString(descripton)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write descripton");
        return false;
    }

    return true;
}

bool USBDevice::Marshalling(MessageParcel &parcel) const
{
    if (!DeviceData::Marshalling(parcel)) {
        return false;
    }

    if (!parcel.WriteUint16(productId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write productId");
        return false;
    }

    if (!parcel.WriteUint16(vendorId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write vendorId");
        return false;
    }

    return true;
}

std::shared_ptr<DeviceData> DeviceData::UnMarshalling(MessageParcel &parcel)
{
    uint32_t busTypeData = 0;
    if (!parcel.ReadUint32(busTypeData)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read busType");
        return nullptr;
    }

    BusType busType = static_cast<BusType>(busTypeData);
    if (busType == BusType::BUS_TYPE_INVALID) {
        EDM_LOGE(MODULE_DEV_MGR, "invalid busType:%{public}u", busTypeData);
        return nullptr;
    }

    // if you need to extend the DeviceData type, please add code to read it here
    std::shared_ptr<DeviceData> device;
    switch (busType) {
        case BusType::BUS_TYPE_USB: {
            device = USBDevice::UnMarshalling(parcel);
            break;
        }
        default:
            break;
    }

    if (device != nullptr) {
        device->busType = busType;
    }
    return device;
}

std::string DeviceData::Dump()
{
    std::stringstream os;
    os << "{busType:" << busType << ", ";
    os << "deviceId:" << deviceId << ", ";
    os << "descripton:" << descripton << "}";
    return os.str();
}

std::shared_ptr<DeviceData> USBDevice::UnMarshalling(MessageParcel &parcel)
{
    // the busType has been read
    std::shared_ptr<USBDevice> device = std::make_shared<USBDevice>();
    if (!parcel.ReadUint64(device->deviceId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read deviceId");
        return nullptr;
    }

    if (!parcel.ReadString(device->descripton)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read descripton");
        return nullptr;
    }

    if (!parcel.ReadUint16(device->productId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read productId");
        return nullptr;
    }

    if (!parcel.ReadUint16(device->vendorId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read vendorId");
        return nullptr;
    }

    return device;
}

std::string USBDevice::Dump()
{
    std::stringstream os;
    os << "{busType:" << busType << ", ";
    os << "deviceId:" << deviceId << ", ";
    os << "descripton:" << descripton << ", ";
    os << "productId:" << productId << ", ";
    os << "vendorId:" << vendorId << "}";
    return os.str();
}

bool DeviceInfoData::Marshalling(MessageParcel &parcel) const
{
    EDM_LOGD(MODULE_DEV_MGR, "DeviceInfoData Marshalling enter");
    if (!parcel.WriteUint64(deviceId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write deviceId");
        return false;
    }

    if (!parcel.WriteBool(isDriverMatched)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write isDriverMatched");
        return false;
    }

    if (!parcel.WriteString(driverUid)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write driverUid");
        return false;
    }
    return true;
}

bool USBDeviceInfoData::Marshalling(MessageParcel &parcel) const
{
    EDM_LOGD(MODULE_DEV_MGR, "USBDeviceInfoData Marshalling enter");
    if (!DeviceInfoData::Marshalling(parcel)) {
        return false;
    }

    if (!parcel.WriteUint16(productId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write productId");
        return false;
    }

    if (!parcel.WriteUint16(vendorId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write vendorId");
        return false;
    }

    EDM_LOGD(MODULE_DEV_MGR, "interfaceDescList size: %{public}zu", interfaceDescList.size());
    if (!parcel.WriteUint64(static_cast<uint64_t>(interfaceDescList.size()))) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write interfaceDescList size");
        return false;
    }
    
    for (auto &desc : interfaceDescList) {
        if (desc == nullptr) {
            EDM_LOGE(MODULE_DEV_MGR, "invalid usb interface desc");
            return false;
        }

        if (!desc->Marshalling(parcel)) {
            return false;
        }
    }

    return true;
}

bool USBInterfaceDesc::Marshalling(MessageParcel &parcel) const
{
    EDM_LOGD(MODULE_DEV_MGR, "USBInterfaceDesc Marshalling enter");
    if (!parcel.WriteUint8(bInterfaceNumber)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write bInterfaceNumber");
        return false;
    }

    if (!parcel.WriteUint8(bClass)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write bClass");
        return false;
    }

    if (!parcel.WriteUint8(bSubClass)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write bSubClass");
        return false;
    }

    if (!parcel.WriteUint8(bProtocol)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write bProtocol");
        return false;
    }
    return true;
}

std::shared_ptr<DeviceInfoData> DeviceInfoData::UnMarshalling(MessageParcel &parcel)
{
    uint64_t deviceId = 0;
    if (!parcel.ReadUint64(deviceId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read deviceId");
        return nullptr;
    }

    BusType busType = DeviceInfoData::GetBusTypeByDeviceId(deviceId);
    if (busType <= BusType::BUS_TYPE_INVALID || busType >= BusType::BUS_TYPE_MAX) {
        EDM_LOGE(MODULE_DEV_MGR, "invalid busType:%{public}u", busType);
        return nullptr;
    }

    bool isDriverMatched = false;
    if (!parcel.ReadBool(isDriverMatched)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read isDriverMatched");
        return nullptr;
    }

    std::string driverUid = "";
    if (!parcel.ReadString(driverUid)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read driverUid");
        return nullptr;
    }
    std::shared_ptr<DeviceInfoData> deviceInfo = nullptr;
    switch (busType) {
        case BusType::BUS_TYPE_USB: {
            deviceInfo = USBDeviceInfoData::UnMarshalling(parcel);
            break;
        }
        default:
            break;
    }
    if (deviceInfo != nullptr) {
        deviceInfo->deviceId = deviceId;
        deviceInfo->isDriverMatched = isDriverMatched;
        deviceInfo->driverUid = driverUid;
    }
    return deviceInfo;
}

bool DeviceInfoData::DeviceInfosUnMarshalling(MessageParcel &parcel,
    std::vector<std::shared_ptr<DeviceInfoData>> &deviceInfos)
{
    uint64_t deviceInfoSize = 0;
    if (!parcel.ReadUint64(deviceInfoSize)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to read size of DeviceInfoData");
        return false;
    }

    if (deviceInfoSize > deviceInfos.max_size()) {
        EDM_LOGE(MODULE_FRAMEWORK, "invalid size of DeviceInfoData");
        return false;
    }

    for (uint64_t i = 0; i < deviceInfoSize; i++) {
        std::shared_ptr<DeviceInfoData> deviceInfo = DeviceInfoData::UnMarshalling(parcel);
        if (deviceInfo == nullptr) {
            EDM_LOGE(MODULE_FRAMEWORK, "failed to read deviceInfo");
            return false;
        }
        deviceInfos.push_back(deviceInfo);
    }
    return true;
}

BusType DeviceInfoData::GetBusTypeByDeviceId(uint64_t deviceId)
{
    return static_cast<BusType>(deviceId & 0x00000000FFFFFFF);
}

std::shared_ptr<USBDeviceInfoData> USBDeviceInfoData::UnMarshalling(MessageParcel &parcel)
{
    std::shared_ptr<USBDeviceInfoData> deviceInfo = std::make_shared<USBDeviceInfoData>();
    if (!parcel.ReadUint16(deviceInfo->productId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read productId");
        return nullptr;
    }
    if (!parcel.ReadUint16(deviceInfo->vendorId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read vendorId");
        return nullptr;
    }

    uint64_t interfaceDescSize = 0;
    if (!parcel.ReadUint64(interfaceDescSize)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read interfaceDescList size");
        return nullptr;
    }
    for (uint64_t i = 0; i < interfaceDescSize; i++) {
        std::shared_ptr<USBInterfaceDesc> desc = USBInterfaceDesc::UnMarshalling(parcel);
        if (desc == nullptr) {
            return nullptr;
        }
        deviceInfo->interfaceDescList.push_back(desc);
    }
        
    return deviceInfo;
}

std::shared_ptr<USBInterfaceDesc> USBInterfaceDesc::UnMarshalling(MessageParcel &parcel)
{
    std::shared_ptr<USBInterfaceDesc> desc = std::make_shared<USBInterfaceDesc>();

    if (!parcel.ReadUint8(desc->bInterfaceNumber)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read bInterfaceNumber");
        return nullptr;
    }
    if (!parcel.ReadUint8(desc->bClass)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read bClass");
        return nullptr;
    }
    if (!parcel.ReadUint8(desc->bSubClass)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read bSubClass");
        return nullptr;
    }
    if (!parcel.ReadUint8(desc->bProtocol)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read bProtocol");
        return nullptr;
    }
    return desc;
}

bool DriverInfoData::Marshalling(MessageParcel &parcel) const
{
    if (!parcel.WriteUint32(static_cast<uint32_t>(busType))) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write busType");
        return false;
    }

    if (!parcel.WriteString(driverUid)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write driverUid");
        return false;
    }

    if (!parcel.WriteString(driverName)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write driverName");
        return false;
    }

    if (!parcel.WriteString(bundleSize)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write bundleSize");
        return false;
    }

    if (!parcel.WriteString(version)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write version");
        return false;
    }

    if (!parcel.WriteString(description)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write description");
        return false;
    }

    return true;
}

bool USBDriverInfoData::Marshalling(MessageParcel &parcel) const
{
    if (!DriverInfoData::Marshalling(parcel)) {
        return false;
    }

    if (!parcel.WriteUInt16Vector(pids)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write pids");
        return false;
    }

    if (!parcel.WriteUInt16Vector(vids)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write vids");
        return false;
    }

    return true;
}

static std::shared_ptr<DriverInfoData> ReadDriverInfoData(MessageParcel &parcel)
{
    std::shared_ptr<DriverInfoData> driverInfo = std::make_shared<DriverInfoData>();

    if (!parcel.ReadUint32(reinterpret_cast<uint32_t &>(driverInfo->busType))) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read busType");
        return nullptr;
    }

    if (driverInfo->busType <= BusType::BUS_TYPE_INVALID || driverInfo->busType >= BusType::BUS_TYPE_MAX) {
        EDM_LOGE(MODULE_DEV_MGR, "invalid busType:%{public}u", driverInfo->busType);
        return nullptr;
    }

    if (!parcel.ReadString(driverInfo->driverUid)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read driverUid");
        return nullptr;
    }

    if (!parcel.ReadString(driverInfo->driverName)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read driverName");
        return nullptr;
    }

    if (!parcel.ReadString(driverInfo->bundleSize)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read bundleSize");
        return nullptr;
    }

    if (!parcel.ReadString(driverInfo->version)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read version");
        return nullptr;
    }

    if (!parcel.ReadString(driverInfo->description)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read description");
        return nullptr;
    }
    return driverInfo;
}

std::shared_ptr<DriverInfoData> DriverInfoData::UnMarshalling(MessageParcel &parcel)
{
    std::shared_ptr<DriverInfoData> baseDriverInfo = ReadDriverInfoData(parcel);
    if (baseDriverInfo == nullptr) {
        return nullptr;
    }
    std::shared_ptr<DriverInfoData> driverInfo = nullptr;
    switch (baseDriverInfo->busType) {
        case BusType::BUS_TYPE_USB: {
            driverInfo = USBDriverInfoData::UnMarshalling(parcel);
            break;
        }
        default:
            break;
    }
    if (driverInfo != nullptr) {
        driverInfo->busType = baseDriverInfo->busType;
        driverInfo->driverUid = baseDriverInfo->driverUid;
        driverInfo->driverName = baseDriverInfo->driverName;
        driverInfo->bundleSize = baseDriverInfo->bundleSize;
        driverInfo->version = baseDriverInfo->version;
        driverInfo->description = baseDriverInfo->description;
    }
    return driverInfo;
}

bool DriverInfoData::DriverInfosUnMarshalling(MessageParcel &parcel,
    std::vector<std::shared_ptr<DriverInfoData>> &driverInfos)
{
    uint64_t driverInfoSize = 0;
    if (!parcel.ReadUint64(driverInfoSize)) {
        EDM_LOGE(MODULE_FRAMEWORK, "failed to read size of DriverInfoData");
        return false;
    }

    if (driverInfoSize > driverInfos.max_size()) {
        EDM_LOGE(MODULE_FRAMEWORK, "invalid size of DriverInfoData");
        return false;
    }

    for (uint64_t i = 0; i < driverInfoSize; i++) {
        std::shared_ptr<DriverInfoData> driverInfo = DriverInfoData::UnMarshalling(parcel);
        if (driverInfo == nullptr) {
            EDM_LOGE(MODULE_FRAMEWORK, "failed to read driverInfo");
            return false;
        }
        driverInfos.push_back(driverInfo);
    }
    return true;
}

std::shared_ptr<USBDriverInfoData> USBDriverInfoData::UnMarshalling(MessageParcel &parcel)
{
    std::shared_ptr<USBDriverInfoData> driverInfo = std::make_shared<USBDriverInfoData>();

    if (!parcel.ReadUInt16Vector(&driverInfo->pids)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read pids");
        return nullptr;
    }

    if (!parcel.ReadUInt16Vector(&driverInfo->vids)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read vids");
        return nullptr;
    }

    return driverInfo;
}
} // namespace ExternalDeviceManager
} // namespace OHOS