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
 
#ifndef OHOS_HDI_MANIU_V1_0_MANIUINTERFACESERVICE_H
#define OHOS_HDI_MANIU_V1_0_MANIUINTERFACESERVICE_H

#include "v1_0/imaniu_interface.h"

namespace OHOS {
namespace HDI {
namespace Maniu {
namespace V1_0 {
class ManiuInterfaceService : public OHOS::HDI::Maniu::V1_0::IManiuInterface {
public:
    ManiuInterfaceService() = default;
    virtual ~ManiuInterfaceService() = default;

    int32_t Helloworld(const std::string &sendMsg, std::string &recvMsg) override;

    int32_t ManiuInterfaceService::Func1(int32_t id) override;

    int32_t ManiuInterfaceService::Func2(int32_t data) override;

    int32_t ManiuInterfaceService::FuncRegister(int32_t id, const sptr<OHOS::HDI::Maniu::V1_0::IManiuCallback>& callbackObj) override;


};
} // V1_0
} // Maniu
} // HDI
} // OHOS

#endif // OHOS_HDI_MANIU_V1_0_MANIUINTERFACESERVICE_H