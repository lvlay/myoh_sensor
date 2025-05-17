/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "usb_ddk_api.h"
#include "usb_ddk_types.h"
#include "hid_ddk_api.h"
#include "hid_ddk_types.h"
#include "ddk_api.h"
#include <cstdlib>
#include <js_native_api_types.h>
#include <vector>

using namespace std;

static napi_value UsbInit(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_Usb_Init();
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbGetDeviceDescriptor(napi_env env, napi_callback_info info)
{
    struct UsbDeviceDescriptor devDesc;
    int32_t returnValue = OH_Usb_GetDeviceDescriptor(0, &devDesc);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbGetConfigDescriptor(napi_env env, napi_callback_info info)
{
    struct UsbDdkConfigDescriptor *config = nullptr;
    int32_t returnValue = OH_Usb_GetConfigDescriptor(0, 1, &config);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbClaimInterface(napi_env env, napi_callback_info info)
{
    uint64_t g_interfaceHandle = 0;
    int32_t returnValue = OH_Usb_ClaimInterface(0, 0, &g_interfaceHandle);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbReleaseInterface(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_Usb_ReleaseInterface(0);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbSelectInterfaceSetting(napi_env env, napi_callback_info info)
{
    int32_t returnValue = OH_Usb_SelectInterfaceSetting(0, 0);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbGetCurrentInterfaceSetting(napi_env env, napi_callback_info info)
{
    uint8_t settingIndex = 0;
    int32_t returnValue = OH_Usb_GetCurrentInterfaceSetting(0, &settingIndex);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbSendControlReadRequest(napi_env env, napi_callback_info info)
{
    struct UsbControlRequestSetup setup;
    uint8_t strDesc[2] = {0};
    uint32_t len = 100;
    int32_t returnValue = OH_Usb_SendControlReadRequest(0, &setup, 0, strDesc, &len);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbSendControlWriteRequest(napi_env env, napi_callback_info info)
{
    struct UsbControlRequestSetup strDescSetup;
    uint8_t data[2] = {0x02, 0x02};
    int32_t returnValue = OH_Usb_SendControlWriteRequest(0, &strDescSetup, 0, data, 2);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbSendPipeRequest(napi_env env, napi_callback_info info)
{
    struct UsbRequestPipe pipe;
    uint8_t address = 0;
    struct UsbDeviceMemMap devMmap = {.address = &address};
    int32_t returnValue = OH_Usb_SendPipeRequest(&pipe, &devMmap);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbCreateDeviceMemMap(napi_env env, napi_callback_info info)
{
    UsbDeviceMemMap *devMmap = nullptr;
    int32_t returnValue = OH_Usb_CreateDeviceMemMap(0, 100, &devMmap);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value UsbSendPipeRequestWithAshmem(napi_env env, napi_callback_info info)
{
    DDK_Ashmem *ashmem = nullptr;
    struct UsbRequestPipe pipe;
    int32_t returnValue = OH_Usb_SendPipeRequestWithAshmem(&pipe, ashmem);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static int32_t CreateTestDevice()
{
    std::vector<Hid_DeviceProp> deviceProp = { HID_PROP_DIRECT };
    Hid_Device hidDevice = {
        .deviceName = "VSoC keyboard",
        .vendorId = 0x6006,
        .productId = 0x6006,
        .version = 1,
        .bustype = 3,
        .properties = deviceProp.data(),
        .propLength = (uint16_t)deviceProp.size()
    };

    std::vector<Hid_EventType> eventType = { HID_EV_ABS, HID_EV_KEY, HID_EV_SYN, HID_EV_MSC };
    Hid_EventTypeArray eventTypeArray = {
        .hidEventType = eventType.data(),
        .length = (uint16_t)eventType.size()
    };
    std::vector<Hid_KeyCode> keyCode = { HID_BTN_TOOL_PEN, HID_BTN_TOOL_RUBBER, HID_BTN_TOUCH, HID_BTN_STYLUS,
        HID_BTN_RIGHT };
    Hid_KeyCodeArray keyCodeArray = {
        .hidKeyCode = keyCode.data(),
        .length = (uint16_t)keyCode.size()
    };
    std::vector<Hid_MscEvent> mscEvent = { HID_MSC_SCAN };
    Hid_MscEventArray mscEventArray = {
        .hidMscEvent = mscEvent.data(),
        .length = (uint16_t)mscEvent.size()
    };
    std::vector<Hid_AbsAxes> absAxes = { HID_ABS_X, HID_ABS_Y, HID_ABS_PRESSURE };
    Hid_AbsAxesArray absAxesArray = {
        .hidAbsAxes = absAxes.data(),
        .length = (uint16_t)absAxes.size()
    };
    Hid_EventProperties hidEventProp = {
        .hidEventTypes = eventTypeArray,
        .hidKeys = keyCodeArray,
        .hidAbs = absAxesArray,
        .hidMiscellaneous = mscEventArray
    };
    return OH_Hid_CreateDevice(&hidDevice, &hidEventProp);
}

static napi_value HidCreateDevice(napi_env env, napi_callback_info info)
{
    int32_t deviceId = CreateTestDevice();
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, deviceId, &result));
    return result;
}

static napi_value HidEmitEvent(napi_env env, napi_callback_info info)
{
    Hid_EmitItem event = {
        .type = HID_EV_MSC,
        .code = HID_MSC_SCAN,
        .value = 0x000d0042
    };
    std::vector<Hid_EmitItem> items;
    items.push_back(event);
    int32_t returnValue = OH_Hid_EmitEvent(0, items.data(), (uint16_t)items.size());
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static napi_value HidDestroyDevice(napi_env env, napi_callback_info info)
{
    int32_t deviceId = 0;
    int32_t returnValue = OH_Hid_DestroyDevice(deviceId);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("usbInit", UsbInit),
        DECLARE_NAPI_FUNCTION("usbGetDeviceDescriptor", UsbGetDeviceDescriptor),
        DECLARE_NAPI_FUNCTION("usbGetConfigDescriptor", UsbGetConfigDescriptor),
        DECLARE_NAPI_FUNCTION("usbClaimInterface", UsbClaimInterface),
        DECLARE_NAPI_FUNCTION("usbReleaseInterface", UsbReleaseInterface),
        DECLARE_NAPI_FUNCTION("usbSelectInterfaceSetting", UsbSelectInterfaceSetting),
        DECLARE_NAPI_FUNCTION("usbGetCurrentInterfaceSetting", UsbGetCurrentInterfaceSetting),
        DECLARE_NAPI_FUNCTION("usbSendControlReadRequest", UsbSendControlReadRequest),
        DECLARE_NAPI_FUNCTION("usbSendControlWriteRequest", UsbSendControlWriteRequest),
        DECLARE_NAPI_FUNCTION("usbSendPipeRequest", UsbSendPipeRequest),
        DECLARE_NAPI_FUNCTION("usbCreateDeviceMemMap", UsbCreateDeviceMemMap),
        DECLARE_NAPI_FUNCTION("usbSendPipeRequestWithAshmem", UsbSendPipeRequestWithAshmem),
        DECLARE_NAPI_FUNCTION("hidCreateDevice", HidCreateDevice),
        DECLARE_NAPI_FUNCTION("hidEmitEvent", HidEmitEvent),
        DECLARE_NAPI_FUNCTION("hidDestroyDevice", HidDestroyDevice),
    };

    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "libddk_permission_test",
    .nm_priv = ((void *)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&demoModule);
}
