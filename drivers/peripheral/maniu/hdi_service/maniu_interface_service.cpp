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

#include "v1_0/maniu_interface_service.h"
#include "v1_0/imaniu_callback.h"
#include <hdf_base.h>
#include "maniu_log.h"
#include "devhost_dump_reg.h"
#include "maniu_dump.h"

#define HDF_LOG_TAG    maniu_interface_service

namespace OHOS {
namespace HDI {
namespace Maniu {
namespace V1_0 {
extern "C" IManiuInterface *ManiuInterfaceImplGetInstance(void)
{
    // עᨩdumper
    DevHostRegisterDumpHost(GetManiuDump);
    // [hdf-gen] Todo
    HDF_LOGI("%{public}s: IManiuInterface init", __func__);
    return new (std::nothrow) ManiuInterfaceService();
}

int32_t ManiuInterfaceService::Func1(int32_t id)
{
    HDF_LOGI("%{public}s: Enter ", __func__);
    return HDF_SUCCESS;
}

int32_t ManiuInterfaceService::Func2(int32_t data)
{
    HDF_LOGI("%{public}s: Enter ", __func__);
    return HDF_SUCCESS;
}

int32_t ManiuInterfaceService::FuncRegister(int32_t id, const sptr<OHOS::HDI::Maniu::V1_0::IManiuCallback>& callbackObj)
{
    HDF_LOGI("%{public}s: Enter ", __func__);
    return HDF_SUCCESS;
}

} // V1_0
} // Maniu
} // HDI
} // OHOS
