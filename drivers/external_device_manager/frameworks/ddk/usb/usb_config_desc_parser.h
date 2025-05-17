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

#ifndef USB_CONFIG_DESC_PARSER_H
#include <vector>

#include "usb_ddk_types.h"
namespace OHOS {
namespace ExternalDeviceManager {
int32_t ParseUsbConfigDescriptor(const std::vector<uint8_t> &configBuffer, UsbDdkConfigDescriptor ** const config);
void FreeUsbConfigDescriptor(UsbDdkConfigDescriptor * const config);
} // namespace ExternalDeviceManager
} // namespace OHOS
#define USB_CONFIG_DESC_PARSER_H
#endif // USB_CONFIG_DESC_PARSER_H