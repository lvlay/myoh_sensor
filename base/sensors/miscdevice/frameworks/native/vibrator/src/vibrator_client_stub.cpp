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

#include "vibrator_client_stub.h"

#include "message_parcel.h"

#include "miscdevice_log.h"

#undef LOG_TAG
#define LOG_TAG "VibratorClientStub"
namespace OHOS {
namespace Sensors {

int32_t VibratorClientStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    std::u16string descriptor = VibratorClientStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        MISC_HILOGE("Client and service descriptors are inconsistent");
        return OBJECT_NULL;
    }
    MISC_HILOGD("Begin, cmd:%{public}u", code);
    return NO_ERROR;
}
}  // namespace Sensors
}  // namespace OHOS