/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "sensor_client_stub.h"

#include "message_parcel.h"
#include "sensor_log.h"

#undef LOG_TAG
#define LOG_TAG "SensorClientStub"
namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

int32_t SensorClientStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                          MessageOption &option)
{
    std::u16string descriptor = SensorClientStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        SEN_HILOGE("Client and service descriptors are inconsistent");
        return OBJECT_NULL;
    }
    SEN_HILOGD("Begin, cmd:%{public}u", code);
    return NO_ERROR;
}
}  // namespace Sensors
}  // namespace OHOS
