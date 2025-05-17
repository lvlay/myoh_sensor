/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "service_fuzzer.h"

#include "hdf_base.h"
#include "hdf_log.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "devmgr_service.h"
#ifdef __cplusplus
} // extern "C"
#endif
#include "parcel.h"
#include <cstddef>
#include <cstdint>


#define HDF_LOG_TAG devicemanagerstart_fuzzer

namespace OHOS {
constexpr size_t THRESHOLD = 10;

void ServiceFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        HDF_LOGE("%{public}s: data is nullptr!", __func__);
    }

    Parcel parcel;
    parcel.WriteBuffer(data, size);
    struct HdfObject *object = DevmgrServiceCreate();
    struct IDevmgrService *service = DevmgrServiceGetInstance();
    struct DevmgrService *dmService = reinterpret_cast<struct DevmgrService *>(service);
    DevmgrServiceLoadLeftDriver(dmService);
    service->PowerStateChange(service, POWER_STATE_SUSPEND);
    DevmgrServiceRelease(object);
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::THRESHOLD) {
        return HDF_SUCCESS;
    }

    OHOS::ServiceFuzzTest(data, size);
    return HDF_SUCCESS;
}