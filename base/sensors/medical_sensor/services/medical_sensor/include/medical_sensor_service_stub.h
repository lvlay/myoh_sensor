/*
 * Copyright (c) 2022 Chipsea Technologies (Shenzhen) Corp., Ltd.
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

#ifndef SENSOR_SERVICE_STUB_H
#define SENSOR_SERVICE_STUB_H

#include "iremote_stub.h"
#include "message_parcel.h"
#include "nocopyable.h"

#include "i_medical_sensor_service.h"

namespace OHOS {
namespace Sensors {
class MedicalSensorServiceStub : public IRemoteStub<IMedicalSensorService> {
public:
    MedicalSensorServiceStub();
    virtual ~MedicalSensorServiceStub();
    virtual int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                    MessageOption &option) override;

private:
    DISALLOW_COPY_AND_MOVE(MedicalSensorServiceStub);
    using AfeBaseFunc = ErrCode (MedicalSensorServiceStub::*)(MessageParcel &data, MessageParcel &reply);

    ErrCode AfeEnableInner(MessageParcel &data, MessageParcel &reply);
    ErrCode AfeDisableInner(MessageParcel &data, MessageParcel &reply);
    ErrCode AfeSetOptionInner(MessageParcel &data, MessageParcel &reply);
    ErrCode GetAfeStateInner(MessageParcel &data, MessageParcel &reply);
    ErrCode RunCommandInner(MessageParcel &data, MessageParcel &reply);
    ErrCode GetAllSensorsInner(MessageParcel &data, MessageParcel &reply);
    ErrCode CreateDataChannelInner(MessageParcel &data, MessageParcel &reply);
    ErrCode DestroyDataChannelInner(MessageParcel &data, MessageParcel &reply);

    std::unordered_map<uint32_t, AfeBaseFunc> baseFuncs_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // SENSOR_SERVICE_STUB_H
