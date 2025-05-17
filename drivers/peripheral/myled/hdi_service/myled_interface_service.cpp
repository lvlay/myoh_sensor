 
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
 
#include "v1_0/myled_interface_service.h"
#include <hdf_base.h>
// #include "myled_log.h"
#include <hdf_log.h>
#include "devhost_dump_reg.h"
#include "myled_dump.h"

#define HDF_LOG_TAG    myled_interface_service

namespace OHOS {
namespace HDI {
namespace Myled {
namespace V1_0 {
extern "C" IMyledInterface *MyledInterfaceImplGetInstance(void)
{
    // ע��hidumper
    DevHostRegisterDumpHost(GetMyledDump);
    // [hdf-gen] Todo
    HDF_LOGI("%{public}s: IMyledInterface init", __func__);
    return new (std::nothrow) MyledInterfaceService();
}

int32_t MyledInterfaceService::set_led_status(int32_t mode)
{
    // [hdf-gen] TODO: Invoke the business implementation
    HDF_LOGI("%{public}s: IMyledInterface set_led_status", __func__);
    return HDF_SUCCESS;
}


} // V1_0
} // Myled
} // HDI
} // OHOS

  