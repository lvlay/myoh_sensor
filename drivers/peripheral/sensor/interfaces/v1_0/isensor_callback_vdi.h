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

#ifndef OHOS_HDI_SENSOR_V1_1_ISENSORCALLBACK_VDI_H
#define OHOS_HDI_SENSOR_V1_1_ISENSORCALLBACK_VDI_H

#include <stdint.h>
#include <vector>
#include <hdf_base.h>
#include <hdi_base.h>
#include "iremote_object.h"
#include "v2_0/sensor_types.h"

namespace OHOS {
namespace HDI {
namespace Sensor {
namespace V1_1 {

struct HdfSensorEventsVdi {
    int32_t sensorId;
    int32_t version;
    int64_t timestamp;
    uint32_t option;
    int32_t mode;
    std::vector<uint8_t> data;
    uint32_t dataLen;
};

class ISensorCallbackVdi : public HdiBase {
public:
    virtual ~ISensorCallbackVdi() = default;
    virtual int32_t OnDataEventVdi(const HdfSensorEventsVdi& eventVdi) = 0;
    virtual int32_t OnDataEvent(const V2_0::HdfSensorEvents& event) = 0;
    virtual sptr<IRemoteObject> HandleCallbackDeath() = 0;
};

} // V1_1
} // Sensor
} // HDI
} // OHOS

#endif // OHOS_HDI_SENSOR_V1_1_ISENSORCALLBACK_VDI_H
