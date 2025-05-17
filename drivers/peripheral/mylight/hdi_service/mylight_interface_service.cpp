 
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
 
#include "v1_0/mylight_interface_service.h"
#include <hdf_base.h>
// #include "mylight_log.h"
#include <hdf_log.h>
#include "devhost_dump_reg.h"
#include "mylight_dump.h"
#include "hdf_base.h"
#include "hdf_io_service_if.h"
#define HDF_LOG_TAG    mylight_interface_service


#define LED_SERVICE_NAME                                "maniu_led_service"
#define LED_WRITE                                       1

namespace OHOS {
namespace HDI {
namespace Mylight {
namespace V1_0 {
extern "C" IMylightInterface *MylightInterfaceImplGetInstance(void)
{
    // ע��hidumper
    DevHostRegisterDumpHost(GetMylightDump);
    // [hdf-gen] Todo
    HDF_LOGI("%{public}s: IMylightInterface init", __func__);
    return new (std::nothrow) MylightInterfaceService();
}

int32_t MylightInterfaceService::set_led_status(int32_t mode)
{
    // [hdf-gen] TODO: Invoke the business implementation
    HDF_LOGI("%{public}s:   mylight_interface_service set_led_status called", __func__);

    int ret = HDF_SUCCESS;
    HDF_LOGI("NAPITEST_LOGI set_led_status entend 1  %s\r\n", LED_SERVICE_NAME);  

    struct HdfIoService *serv = HdfIoServiceBind(LED_SERVICE_NAME);
    if (serv == NULL) {
        HDF_LOGI("NAPITEST_LOGI set_led_status  failed  %s\r\n", LED_SERVICE_NAME);  
        // HILOG_ERROR(LOG_APP, "get service %s failed", LED_SERVICE_NAME);
        return -1;
    }
    struct HdfSBuf *data = HdfSbufObtainDefaultSize();
    if (data == NULL) {
        HDF_LOGI("NAPITEST_LOGI obtain data   failed  %s\r\n", LED_SERVICE_NAME);  
        return -1;
    }

    if (!HdfSbufWriteInt32(data, mode)) {
        HILOG_ERROR(LOG_APP, "write data failed");
        return -1;
    }
    HDF_LOGI("NAPITEST_LOGI set_led_status entend 2  %s\r\n", LED_SERVICE_NAME);  
    ret = serv->dispatcher->Dispatch(&serv->object, LED_WRITE, data, NULL);
    HDF_LOGI("NAPITEST_LOGI set_led_status entend 3  %s\r\n", LED_SERVICE_NAME);      
    HdfSbufRecycle(data);
    HdfIoServiceRecycle(serv);
    HILOG_INFO(LOG_APP, "[%s] main exit.", LOG_TAG);

    return ret;

    // return HDF_SUCCESS;
}

} // V1_0
} // Mylight
} // HDI
} // OHOS

  