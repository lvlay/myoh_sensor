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

#include <cstdint>
#include <cstring>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include "hid_ddk_api.h"
#include "hid_ddk_types.h"
#include "v1_0/ihid_ddk.h"
#include "hilog_wrapper.h"
#include <iproxy_broker.h>

using namespace OHOS;
using namespace OHOS::ExternalDeviceManager;
namespace {
static OHOS::sptr<OHOS::HDI::Input::Ddk::V1_0::IHidDdk> g_ddk = nullptr;
static OHOS::sptr<IRemoteObject::DeathRecipient> recipient_ = nullptr;
std::mutex g_mutex;
constexpr uint32_t MAX_EMIT_ITEM_NUM = 20;
constexpr uint32_t MAX_HID_DEVICE_PROP_LEN = 7;
constexpr uint32_t MAX_HID_EVENT_TYPES_LEN = 5;
constexpr uint32_t MAX_HID_KEYS_LEN = 100;
constexpr uint32_t MAX_HID_ABS_LEN = 26;
constexpr uint32_t MAX_HID_REL_BITS_LEN = 13;
constexpr uint32_t MAX_HID_MISC_EVENT_LEN = 6;
constexpr uint32_t MAX_NAME_LENGTH = 80;
}
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
static std::unordered_map<int32_t, std::shared_ptr<struct TempDevice>> g_deviceMap;

struct TempDevice {
    OHOS::HDI::Input::Ddk::V1_0::Hid_Device tempDevice;
    OHOS::HDI::Input::Ddk::V1_0::Hid_EventProperties tempProperties;
    uint32_t realId;
};

class HidDeathRecipient : public IRemoteObject::DeathRecipient {
public:
        void OnRemoteDied(const wptr<IRemoteObject> &object) override;
};

void HidDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    EDM_LOGI(MODULE_HID_DDK, "hid_ddk remote died");
    if (g_ddk != nullptr) {
        sptr<IRemoteObject> remote = OHOS::HDI::hdi_objcast<OHOS::HDI::Input::Ddk::V1_0::IHidDdk>(g_ddk);
        remote->RemoveDeathRecipient(recipient_);
        recipient_.clear();
        g_ddk = nullptr;
        EDM_LOGI(MODULE_HID_DDK, "remove death recipient success");
    }
}

static uint32_t GetRealDeviceId(int32_t deviceId)
{
    if (g_deviceMap.find(deviceId) != g_deviceMap.end()) {
        if (g_deviceMap[deviceId] != nullptr) {
            return g_deviceMap[deviceId]->realId;
        }
    }
    return static_cast<uint32_t>(deviceId);
}

static int32_t Connect()
{
    if (g_ddk == nullptr) {
        g_ddk = OHOS::HDI::Input::Ddk::V1_0::IHidDdk::Get();
        if (g_ddk == nullptr) {
            EDM_LOGE(MODULE_HID_DDK, "get hid ddk faild");
            return HID_DDK_FAILURE;
        }
        if (g_deviceMap.size() > 0) {
            for (const auto &[_, value] : g_deviceMap) {
                (void)g_ddk->CreateDevice(value->tempDevice, value->tempProperties, value->realId);
            }
        }
        recipient_ = new HidDeathRecipient();
        sptr<IRemoteObject> remote = OHOS::HDI::hdi_objcast<OHOS::HDI::Input::Ddk::V1_0::IHidDdk>(g_ddk);
        if (!remote->AddDeathRecipient(recipient_)) {
            EDM_LOGE(MODULE_HID_DDK, "add DeathRecipient failed");
            return HID_DDK_FAILURE;
        }
    }
    return HID_DDK_SUCCESS;
}

static OHOS::HDI::Input::Ddk::V1_0::Hid_Device ParseHidDevice(Hid_Device *hidDevice)
{
    OHOS::HDI::Input::Ddk::V1_0::Hid_Device tempDevice = {
        .deviceName = hidDevice->deviceName,
        .vendorId = hidDevice->vendorId,
        .productId = hidDevice->productId,
        .version = hidDevice->version,
        .bustype = hidDevice->bustype
    };

    if (hidDevice->properties != nullptr) {
        std::transform(hidDevice->properties, hidDevice->properties + hidDevice->propLength,
            std::back_inserter(tempDevice.properties), [](uint32_t n) {
                return static_cast<OHOS::HDI::Input::Ddk::V1_0::Hid_DeviceProp>(n);
            });
    }

    return tempDevice;
}

static OHOS::HDI::Input::Ddk::V1_0::Hid_EventProperties ParseHidEventProperties(Hid_EventProperties *hidEventProperties)
{
    const uint16_t absLength = 64;
    OHOS::HDI::Input::Ddk::V1_0::Hid_EventProperties tempProperties = {
        .hidAbsMax = std::vector<int32_t>(hidEventProperties->hidAbsMax, hidEventProperties->hidAbsMax + absLength),
        .hidAbsMin = std::vector<int32_t>(hidEventProperties->hidAbsMin, hidEventProperties->hidAbsMin + absLength),
        .hidAbsFuzz = std::vector<int32_t>(hidEventProperties->hidAbsFuzz, hidEventProperties->hidAbsFuzz + absLength),
        .hidAbsFlat = std::vector<int32_t>(hidEventProperties->hidAbsFlat, hidEventProperties->hidAbsFlat + absLength)
    };

    if (hidEventProperties->hidEventTypes.hidEventType != nullptr) {
        std::transform(hidEventProperties->hidEventTypes.hidEventType,
            hidEventProperties->hidEventTypes.hidEventType + hidEventProperties->hidEventTypes.length,
            std::back_inserter(tempProperties.hidEventTypes), [](uint32_t n) {
                return static_cast<OHOS::HDI::Input::Ddk::V1_0::Hid_EventType>(n);
            });
    }

    if (hidEventProperties->hidKeys.hidKeyCode != nullptr) {
        std::transform(hidEventProperties->hidKeys.hidKeyCode,
            hidEventProperties->hidKeys.hidKeyCode + hidEventProperties->hidKeys.length,
            std::back_inserter(tempProperties.hidKeys), [](uint32_t n) {
                return static_cast<OHOS::HDI::Input::Ddk::V1_0::Hid_KeyCode>(n);
            });
    }
    
    if (hidEventProperties->hidAbs.hidAbsAxes != nullptr) {
        std::transform(hidEventProperties->hidAbs.hidAbsAxes,
            hidEventProperties->hidAbs.hidAbsAxes + hidEventProperties->hidAbs.length,
            std::back_inserter(tempProperties.hidAbs), [](uint32_t n) {
                return static_cast<OHOS::HDI::Input::Ddk::V1_0::Hid_AbsAxes>(n);
            });
    }

    if (hidEventProperties->hidRelBits.hidRelAxes != nullptr) {
        std::transform(hidEventProperties->hidRelBits.hidRelAxes,
            hidEventProperties->hidRelBits.hidRelAxes + hidEventProperties->hidRelBits.length,
            std::back_inserter(tempProperties.hidRelBits), [](uint32_t n) {
                return static_cast<OHOS::HDI::Input::Ddk::V1_0::Hid_RelAxes>(n);
            });
    }

    if (hidEventProperties->hidMiscellaneous.hidMscEvent != nullptr) {
        std::transform(hidEventProperties->hidMiscellaneous.hidMscEvent,
            hidEventProperties->hidMiscellaneous.hidMscEvent + hidEventProperties->hidMiscellaneous.length,
            std::back_inserter(tempProperties.hidMiscellaneous), [](uint32_t n) {
                return static_cast<OHOS::HDI::Input::Ddk::V1_0::Hid_MscEvent>(n);
            });
    }

    return tempProperties;
}

static int32_t CacheDeviceInfor(OHOS::HDI::Input::Ddk::V1_0::Hid_Device tempDevice,
    OHOS::HDI::Input::Ddk::V1_0::Hid_EventProperties tempProperties, uint32_t deviceId)
{
    EDM_LOGD(MODULE_HID_DDK, "enter CacheDeviceInfor");
    int32_t id = static_cast<int32_t>(deviceId);
    std::shared_ptr<struct TempDevice> device = std::make_shared<struct TempDevice>();
    device->tempDevice = tempDevice;
    device->tempProperties = tempProperties;
    device->realId = deviceId;

    g_deviceMap[id] = device;
    return id;
}

static bool CheckHidDevice(Hid_Device *hidDevice)
{
    if (hidDevice == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "hidDevice is null");
        return false;
    }
    
    if (hidDevice->propLength > MAX_HID_DEVICE_PROP_LEN) {
        EDM_LOGE(MODULE_HID_DDK, "properties length is out of range");
        return false;
    }
    
    if (hidDevice->deviceName == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "hidDevice->deviceName is nullpointer");
        return false;
    }
    
    if (strlen(hidDevice->deviceName) == 0 || strlen(hidDevice->deviceName) > MAX_NAME_LENGTH - 1) {
        EDM_LOGE(MODULE_HID_DDK, "length of hidDevice->deviceName is out of range");
        return false;
    }
    return true;
}

int32_t OH_Hid_CreateDevice(Hid_Device *hidDevice, Hid_EventProperties *hidEventProperties)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (Connect() != HID_DDK_SUCCESS) {
        return HID_DDK_INVALID_OPERATION;
    }

    if (!CheckHidDevice(hidDevice)) {
        return HID_DDK_INVALID_PARAMETER;
    }

    if (hidEventProperties == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "hidEventProperties is null");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (hidEventProperties->hidEventTypes.length > MAX_HID_EVENT_TYPES_LEN) {
        EDM_LOGE(MODULE_HID_DDK, "hidEventTypes length is out of range");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (hidEventProperties->hidKeys.length > MAX_HID_KEYS_LEN) {
        EDM_LOGE(MODULE_HID_DDK, "hidKeys length is out of range");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (hidEventProperties->hidAbs.length > MAX_HID_ABS_LEN) {
        EDM_LOGE(MODULE_HID_DDK, "hidAbs length is out of range");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (hidEventProperties->hidRelBits.length > MAX_HID_REL_BITS_LEN) {
        EDM_LOGE(MODULE_HID_DDK, "hidRelBits length is out of range");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (hidEventProperties->hidMiscellaneous.length > MAX_HID_MISC_EVENT_LEN) {
        EDM_LOGE(MODULE_HID_DDK, "hidMiscellaneous length is out of range");
        return HID_DDK_INVALID_PARAMETER;
    }

    auto tempDevice = ParseHidDevice(hidDevice);
    auto tempEventProperties = ParseHidEventProperties(hidEventProperties);

    uint32_t deviceId = 0;
    auto ret = g_ddk->CreateDevice(tempDevice, tempEventProperties, deviceId);
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_HID_DDK, "create device failed:%{public}d", ret);
        return ret;
    }
    return CacheDeviceInfor(tempDevice, tempEventProperties, deviceId);
}

int32_t OH_Hid_EmitEvent(int32_t deviceId, const Hid_EmitItem items[], uint16_t length)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (Connect() != HID_DDK_SUCCESS) {
        return HID_DDK_INVALID_OPERATION;
    }

    if (deviceId < 0) {
        EDM_LOGE(MODULE_HID_DDK, "device id is invaild");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (length > MAX_EMIT_ITEM_NUM) {
        EDM_LOGE(MODULE_HID_DDK, "items length is out of range");
        return HID_DDK_INVALID_PARAMETER;
    }

    if (items == nullptr) {
        EDM_LOGE(MODULE_HID_DDK, "items is null");
        return HID_DDK_INVALID_PARAMETER;
    }

    std::vector<OHOS::HDI::Input::Ddk::V1_0::Hid_EmitItem> itemsTemp;
    std::transform(items, items + length, std::back_inserter(itemsTemp), [](Hid_EmitItem item) {
        return *reinterpret_cast<OHOS::HDI::Input::Ddk::V1_0::Hid_EmitItem *>(&item);
    });

    auto ret = g_ddk->EmitEvent(GetRealDeviceId(deviceId), itemsTemp);
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_HID_DDK, "emit event failed:%{public}d", ret);
        return ret;
    }
    return HID_DDK_SUCCESS;
}

int32_t OH_Hid_DestroyDevice(int32_t deviceId)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (Connect() != HID_DDK_SUCCESS) {
        return HID_DDK_INVALID_OPERATION;
    }

    auto ret = g_ddk->DestroyDevice(GetRealDeviceId(deviceId));
    if (ret != HID_DDK_SUCCESS) {
        EDM_LOGE(MODULE_HID_DDK, "destroy device failed:%{public}d", ret);
        return ret;
    }

    g_deviceMap.erase(deviceId);
    return HID_DDK_SUCCESS;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
