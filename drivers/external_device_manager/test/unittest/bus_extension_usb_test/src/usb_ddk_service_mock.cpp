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
#include "usb_ddk_service_mock.h"

namespace OHOS {
namespace USB {
using namespace OHOS;
using namespace OHOS::HDI;
using namespace OHOS::HDI::Usb::Ddk::V1_0;
std::vector<uint8_t> g_configDescBuf {
    9, 2, 59, 0, 2, 1, 0, 160, 250, 9, 4, 0, 0, 1, 3, 1, 1, 0, 9, 33, 17, 1, 0, 1, 34, 67, 0, 7, 5, 129, 3, 8, 0, 1, 9,
    4, 1, 0, 1, 3, 0, 0, 0, 9, 33, 17, 1, 0, 1, 34, 102, 0, 7, 5, 130, 3, 16, 0, 1
};

int32_t UsbDdkServiceMock::GetConfigDescriptor(uint64_t deviceId, uint8_t configIndex, std::vector<uint8_t> &configDesc)
{
    configDesc = g_configDescBuf;
    return HDF_SUCCESS;
}
}
}
