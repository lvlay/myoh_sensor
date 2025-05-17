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

#include "usb_ddk_api.h"
#include <cerrno>
#include <memory.h>
#include <securec.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "usb_config_desc_parser.h"
#include "usb_ddk_types.h"
#include "v1_0/usb_ddk_service.h"

using namespace OHOS::ExternalDeviceManager;
namespace {
OHOS::sptr<OHOS::HDI::Usb::Ddk::V1_0::IUsbDdk> g_ddk = nullptr;
} // namespace

int32_t OH_Usb_Init()
{
    g_ddk = OHOS::HDI::Usb::Ddk::V1_0::IUsbDdk::Get();
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "get ddk failed");
        return USB_DDK_FAILED;
    }

    return g_ddk->Init();
}

void OH_Usb_Release()
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "ddk is null");
        return;
    }
    g_ddk->Release();
    g_ddk.clear();
}

int32_t OH_Usb_GetDeviceDescriptor(uint64_t deviceId, UsbDeviceDescriptor *desc)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "invalid obj");
        return USB_DDK_INVALID_OPERATION;
    }
    if (desc == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "param is null");
        return USB_DDK_INVALID_PARAMETER;
    }

    auto tmpDesc = reinterpret_cast<OHOS::HDI::Usb::Ddk::V1_0::UsbDeviceDescriptor *>(desc);
    int32_t ret = g_ddk->GetDeviceDescriptor(deviceId, *tmpDesc);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_USB_DDK, "get device desc failed: %{public}d", ret);
        return ret;
    }
    return EDM_OK;
}

int32_t OH_Usb_GetConfigDescriptor(
    uint64_t deviceId, uint8_t configIndex, struct UsbDdkConfigDescriptor ** const config)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "invalid obj");
        return USB_DDK_INVALID_OPERATION;
    }
    if (config == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "param is null");
        return USB_DDK_INVALID_PARAMETER;
    }
    std::vector<uint8_t> configDescriptor;
    int32_t ret = g_ddk->GetConfigDescriptor(deviceId, configIndex, configDescriptor);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_USB_DDK, "get config desc failed");
        return ret;
    }

    return ParseUsbConfigDescriptor(configDescriptor, config);
}

void OH_Usb_FreeConfigDescriptor(UsbDdkConfigDescriptor * const config)
{
    return FreeUsbConfigDescriptor(config);
}

int32_t OH_Usb_ClaimInterface(uint64_t deviceId, uint8_t interfaceIndex, uint64_t *interfaceHandle)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "invalid obj");
        return USB_DDK_INVALID_OPERATION;
    }
    if (interfaceHandle == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "param is null");
        return USB_DDK_INVALID_PARAMETER;
    }

    return g_ddk->ClaimInterface(deviceId, interfaceIndex, *interfaceHandle);
}

int32_t OH_Usb_ReleaseInterface(uint64_t interfaceHandle)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "invalid obj");
        return USB_DDK_INVALID_OPERATION;
    }

    return g_ddk->ReleaseInterface(interfaceHandle);
}

int32_t OH_Usb_SelectInterfaceSetting(uint64_t interfaceHandle, uint8_t settingIndex)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "invalid obj");
        return USB_DDK_INVALID_OPERATION;
    }

    return g_ddk->SelectInterfaceSetting(interfaceHandle, settingIndex);
}

int32_t OH_Usb_GetCurrentInterfaceSetting(uint64_t interfaceHandle, uint8_t *settingIndex)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "invalid obj");
        return USB_DDK_INVALID_OPERATION;
    }

    if (settingIndex == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "param is null");
        return USB_DDK_INVALID_PARAMETER;
    }

    return g_ddk->GetCurrentInterfaceSetting(interfaceHandle, *settingIndex);
}

int32_t OH_Usb_SendControlReadRequest(
    uint64_t interfaceHandle, const UsbControlRequestSetup *setup, uint32_t timeout, uint8_t *data, uint32_t *dataLen)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "invalid obj");
        return USB_DDK_INVALID_OPERATION;
    }

    if (setup == nullptr || data == nullptr || dataLen == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "param is null");
        return USB_DDK_INVALID_PARAMETER;
    }

    auto tmpSetUp = reinterpret_cast<const OHOS::HDI::Usb::Ddk::V1_0::UsbControlRequestSetup *>(setup);
    std::vector<uint8_t> dataTmp;
    int32_t ret = g_ddk->SendControlReadRequest(interfaceHandle, *tmpSetUp, timeout, dataTmp);
    if (ret != 0) {
        EDM_LOGE(MODULE_USB_DDK, "send control req failed");
        return ret;
    }

    if (*dataLen < dataTmp.size()) {
        EDM_LOGE(MODULE_USB_DDK, "The data is too small");
        return USB_DDK_INVALID_PARAMETER;
    }

    if (memcpy_s(data, *dataLen, dataTmp.data(), dataTmp.size()) != 0) {
        EDM_LOGE(MODULE_USB_DDK, "copy data failed");
        return USB_DDK_MEMORY_ERROR;
    }
    *dataLen = dataTmp.size();
    return EDM_OK;
}

int32_t OH_Usb_SendControlWriteRequest(uint64_t interfaceHandle, const UsbControlRequestSetup *setup, uint32_t timeout,
    const uint8_t *data, uint32_t dataLen)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "invalid obj");
        return USB_DDK_INVALID_OPERATION;
    }

    if (setup == nullptr || data == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "param is null");
        return USB_DDK_INVALID_PARAMETER;
    }

    auto tmpSetUp = reinterpret_cast<const OHOS::HDI::Usb::Ddk::V1_0::UsbControlRequestSetup *>(setup);
    std::vector<uint8_t> dataTmp(data, data + dataLen);
    return g_ddk->SendControlWriteRequest(interfaceHandle, *tmpSetUp, timeout, dataTmp);
}

int32_t OH_Usb_SendPipeRequest(const UsbRequestPipe *pipe, UsbDeviceMemMap *devMmap)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "invalid obj");
        return USB_DDK_INVALID_OPERATION;
    }

    if (pipe == nullptr || devMmap == nullptr || devMmap->address == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "param is null");
        return USB_DDK_INVALID_PARAMETER;
    }

    auto tmpSetUp = reinterpret_cast<const OHOS::HDI::Usb::Ddk::V1_0::UsbRequestPipe *>(pipe);
    return g_ddk->SendPipeRequest(
        *tmpSetUp, devMmap->size, devMmap->offset, devMmap->bufferLength, devMmap->transferedLength);
}

int32_t OH_Usb_SendPipeRequestWithAshmem(const UsbRequestPipe *pipe, DDK_Ashmem *ashmem)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "invalid obj");
        return USB_DDK_INVALID_OPERATION;
    }

    if (pipe == nullptr || ashmem == nullptr || ashmem->address == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "param is null");
        return USB_DDK_INVALID_PARAMETER;
    }

    auto tmpSetUp = reinterpret_cast<const OHOS::HDI::Usb::Ddk::V1_0::UsbRequestPipe *>(pipe);
    std::vector<uint8_t> address = std::vector<uint8_t>(ashmem->address, ashmem->address + ashmem->size);
    OHOS::HDI::Usb::Ddk::V1_0::UsbAshmem usbAshmem = {ashmem->ashmemFd, address, ashmem->size, 0, ashmem->size, 0};
    return g_ddk->SendPipeRequestWithAshmem(*tmpSetUp, usbAshmem, ashmem->transferredLength);
}

int32_t OH_Usb_CreateDeviceMemMap(uint64_t deviceId, size_t size, UsbDeviceMemMap **devMmap)
{
    if (devMmap == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "invalid param");
        return USB_DDK_INVALID_PARAMETER;
    }

    int32_t fd = -1;
    int32_t ret = g_ddk->GetDeviceMemMapFd(deviceId, fd);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_USB_DDK, "get fd failed, errno=%{public}d", errno);
        return ret;
    }
    ftruncate(fd, size);

    auto buffer = static_cast<uint8_t *>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (buffer == MAP_FAILED) {
        EDM_LOGE(MODULE_USB_DDK, "mmap failed, errno=%{public}d", errno);
        return USB_DDK_MEMORY_ERROR;
    }

    UsbDeviceMemMap *memMap = new UsbDeviceMemMap({buffer, size, 0, size, 0});
    if (memMap == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "alloc dev mem failed, errno=%{public}d", errno);
        return USB_DDK_MEMORY_ERROR;
    }

    *devMmap = memMap;
    return EDM_OK;
}

void OH_Usb_DestroyDeviceMemMap(UsbDeviceMemMap *devMmap)
{
    if (devMmap == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "devMmap is nullptr");
        return;
    }

    if (munmap(devMmap->address, devMmap->size) != 0) {
        EDM_LOGE(MODULE_USB_DDK, "munmap failed, errno=%{public}d", errno);
        return;
    }
    delete devMmap;
}