/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "i_sensor_service.h"

namespace OHOS {
namespace Sensors {
class SensorServiceStub : public IRemoteStub<ISensorService> {
public:
    SensorServiceStub();
    virtual ~SensorServiceStub();
    virtual int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                    MessageOption &option) override;

private:
    DISALLOW_COPY_AND_MOVE(SensorServiceStub);
    ErrCode SensorEnableInner(MessageParcel &data, MessageParcel &reply);
    ErrCode SensorDisableInner(MessageParcel &data, MessageParcel &reply);
    ErrCode GetAllSensorsInner(MessageParcel &data, MessageParcel &reply);
    ErrCode CreateDataChannelInner(MessageParcel &data, MessageParcel &reply);
    ErrCode DestroyDataChannelInner(MessageParcel &data, MessageParcel &reply);
    ErrCode SuspendSensorsInner(MessageParcel &data, MessageParcel &reply);
    ErrCode ResumeSensorsInner(MessageParcel &data, MessageParcel &reply);
    ErrCode GetActiveInfoListInner(MessageParcel &data, MessageParcel &reply);
    ErrCode CreateSocketChannelInner(MessageParcel &data, MessageParcel &reply);
    ErrCode DestroySocketChannelInner(MessageParcel &data, MessageParcel &reply);
    ErrCode EnableActiveInfoCBInner(MessageParcel &data, MessageParcel &reply);
    ErrCode DisableActiveInfoCBInner(MessageParcel &data, MessageParcel &reply);
    ErrCode ResetSensorsInner(MessageParcel &data, MessageParcel &reply);
    bool IsSystemServiceCalling();
    int32_t ProcessRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    bool IsSystemCalling();
};
} // namespace Sensors
} // namespace OHOS
#endif // SENSOR_SERVICE_STUB_H
