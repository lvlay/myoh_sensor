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

#ifndef VIBRATOR_CLIENT_STUB_H
#define VIBRATOR_CLIENT_STUB_H

#include "message_parcel.h"
#include "iremote_stub.h"
#include "i_vibrator_client.h"

namespace OHOS {
namespace Sensors {
class VibratorClientStub : public IRemoteStub<IVibratorClient> {
public:
    VibratorClientStub() = default;
    virtual ~VibratorClientStub() = default;
    virtual int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override;
};
}  // namespace Sensors
}  // namespace OHOS
#endif // VIBRATOR_CLIENT_STUB_H
