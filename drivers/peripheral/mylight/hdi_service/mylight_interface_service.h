 
  /*
 * Copyright (c) 2024 Shenzhen Kaihong Digital Industry Development Co., Ltd.
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
 
#ifndef OHOS_HDI_MYLIGHT_V1_0_MYLIGHTINTERFACESERVICE_H
#define OHOS_HDI_MYLIGHT_V1_0_MYLIGHTINTERFACESERVICE_H

#include "v1_0/imylight_interface.h"

namespace OHOS {
namespace HDI {
namespace Mylight {
namespace V1_0 {
class MylightInterfaceService : public OHOS::HDI::Mylight::V1_0::IMylightInterface {
public:
    MylightInterfaceService() = default;
    virtual ~MylightInterfaceService() = default;

    int32_t set_led_status(int32_t mode) override;

};
} // V1_0
} // Mylight
} // HDI
} // OHOS

#endif // OHOS_HDI_MYLIGHT_V1_0_MYLIGHTINTERFACESERVICE_H
  