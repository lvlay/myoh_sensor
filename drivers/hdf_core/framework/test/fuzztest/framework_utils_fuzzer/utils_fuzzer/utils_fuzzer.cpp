/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */
#include "utils_fuzzer.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "parcel.h"
#include <string>
#include "osal_mem.h"
#include "hdi_support.h"
#include <string_ex.h>
#include "hdf_map.h"
#include "hdf_sbuf.h"
#include "hdf_sbuf_impl.h"
#include "hdf_cstring.h"
#include "hdf_thread_ex.h"
#include "osal_message.h"

namespace OHOS {
static void HdfUtilsFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        HDF_LOGE("%{public}s: data is nullptr!", __func__);
    }
    Map testMap;
    Map testMap1;
    MapInit(&testMap);
    MapInit(&testMap1);
    MapDelete(&testMap);
    int num = 0;
    int num1 = 1;
    const char *name = "i2c";
    const char *name1 = "rtc";
    MapSet(&testMap1, name, &num, sizeof(int *));
    MapSet(&testMap1, name1, &num1, sizeof(int *));
    MapGet(&testMap1, "");
    MapGet(&testMap1, name);
    MapErase(&testMap1, "");
    MapErase(&testMap1, name);
    MapDelete(&testMap1);

    struct HdfMessage *message = HdfMessageObtain(size);
    HdfMessageDelete(reinterpret_cast<HdfSListNode *>(message));

    const char *str = "123";
    struct HdfCString *cstr = HdfCStringObtain(str);
    HdfCStringRecycle(cstr);
    struct HdfThread *thread = HdfThreadNewInstance();
    HdfThreadFreeInstance(thread);

    struct HdfSBuf *sbuf = HdfSbufObtainDefaultSize();
    struct HdfSBuf *copy = HdfSbufCopy(sbuf);
    HdfSbufSetDataSize(copy, size);
    HdfSbufGetDataSize(copy);
    HdfSbufGetData(copy);
    struct HdfSBufImpl *impl = HdfSbufGetImpl(copy);
    if (impl == nullptr) {
        impl = (struct HdfSBufImpl *)OsalMemCalloc(sizeof(struct HdfSBufImpl));
    }
    struct HdfSBuf *newCopy = HdfSbufMove(copy);
    HdfSbufTransDataOwnership(copy);
    HdfSbufTypedObtainInplace(static_cast<uint32_t>(size), impl);
    impl->recycle(impl);
    HdfSbufFlush(copy);
    HdfSbufGetCapacity(copy);
    HdfSbufRecycle(sbuf);
    HdfSbufRecycle(copy);
    HdfSbufRecycle(newCopy);
    sbuf = nullptr;
    copy = nullptr;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::HdfUtilsFuzzTest(data, size);
    return HDF_SUCCESS;
}