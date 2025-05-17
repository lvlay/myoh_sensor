/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdfipc_fuzzer.h"
#include "hdf_remote_adapter.h"
#include "hdf_log.h"
#include "parcel.h"
#include <cstddef>
#include <cstdint>
#include "hdf_base.h"

namespace OHOS {
static void HdfIpcFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        HDF_LOGE("%{public}s: data is nullptr!", __func__);
    }

    Parcel parcel;
    parcel.WriteBuffer(data, size);
    const char *interfaceName = parcel.ReadCString();

    struct HdfRemoteService *service = HdfRemoteAdapterObtain();
    if (service == nullptr) {
        HDF_LOGE("%{public}s: serviceSet is nullptr!", __func__);
    }

    HdfRemoteAdapterAddService(interfaceName, service);
    HdfRemoteAdapterGetService(interfaceName);

    struct HdfDeathRecipient* recipient = nullptr;
    HdfRemoteAdapterAddDeathRecipient(service, recipient);
    HdfRemoteAdapterRemoveDeathRecipient(service, recipient);
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::HdfIpcFuzzTest(data, size);
    return HDF_SUCCESS;
}