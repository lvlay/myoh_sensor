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

#ifndef DRIVER_EXTENSION_MANAGER_TYPES_H
#define DRIVER_EXTENSION_MANAGER_TYPES_H

#include <memory>
#include <string>

#include "ext_object.h"
#include "message_parcel.h"

namespace OHOS {
namespace ExternalDeviceManager {
struct ErrMsg {
    ErrMsg(UsbErrCode code = UsbErrCode::EDM_NOK, const std::string &message = "") : errCode(code), msg(message) {}

    inline bool IsOk() const
    {
        return errCode == UsbErrCode::EDM_OK;
    }

    bool Marshalling(MessageParcel &parcel) const;
    static bool UnMarshalling(MessageParcel &parcel, ErrMsg &data);

    UsbErrCode errCode;
    std::string msg;
};

class DeviceData {
public:
    virtual ~DeviceData() = default;

    virtual bool Marshalling(MessageParcel &parcel) const;
    static std::shared_ptr<DeviceData> UnMarshalling(MessageParcel &parcel);
    virtual std::string Dump();

    BusType busType;
    uint64_t deviceId;
    std::string descripton;
};

class USBDevice : public DeviceData {
public:
    virtual ~USBDevice() = default;

    bool Marshalling(MessageParcel &parcel) const override;
    static std::shared_ptr<DeviceData> UnMarshalling(MessageParcel &parcel);
    std::string Dump() override;

    uint16_t productId;
    uint16_t vendorId;
};

class DeviceInfoData {
public:
    virtual ~DeviceInfoData() = default;
    virtual bool Marshalling(MessageParcel &parcel) const;
    static std::shared_ptr<DeviceInfoData> UnMarshalling(MessageParcel &parcel);
    static bool DeviceInfosUnMarshalling(MessageParcel &parcel,
        std::vector<std::shared_ptr<DeviceInfoData>> &deviceInfos);
    static BusType GetBusTypeByDeviceId(uint64_t deviceId);

    uint64_t deviceId;
    bool isDriverMatched = false;
    std::string driverUid = "";
};

class USBInterfaceDesc {
public:
    virtual ~USBInterfaceDesc() = default;

    bool Marshalling(MessageParcel &parcel) const;
    static std::shared_ptr<USBInterfaceDesc> UnMarshalling(MessageParcel &parcel);

    uint8_t bInterfaceNumber;
    uint8_t bClass;
    uint8_t bSubClass;
    uint8_t bProtocol;
};

class USBDeviceInfoData : public DeviceInfoData {
public:
    virtual ~USBDeviceInfoData() = default;

    bool Marshalling(MessageParcel &parcel) const override;
    static std::shared_ptr<USBDeviceInfoData> UnMarshalling(MessageParcel &parcel);
  
    uint16_t productId;
    uint16_t vendorId;
    std::vector<std::shared_ptr<USBInterfaceDesc>> interfaceDescList;
};

class DriverInfoData {
public:
    virtual ~DriverInfoData() = default;
    virtual bool Marshalling(MessageParcel &parcel) const;
    static std::shared_ptr<DriverInfoData> UnMarshalling(MessageParcel &parcel);
    static bool DriverInfosUnMarshalling(MessageParcel &parcel,
        std::vector<std::shared_ptr<DriverInfoData>> &driverInfos);
    
    BusType busType;
    std::string driverUid;
    std::string driverName;
    std::string bundleSize;
    std::string version;
    std::string description;
};

class USBDriverInfoData : public DriverInfoData {
public:
    virtual ~USBDriverInfoData() = default;

    bool Marshalling(MessageParcel &parcel) const override;
    static std::shared_ptr<USBDriverInfoData> UnMarshalling(MessageParcel &parcel);
    std::vector<uint16_t> pids;
    std::vector<uint16_t> vids;
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // DRIVER_EXTENSION_MANAGER_TYPES_H
