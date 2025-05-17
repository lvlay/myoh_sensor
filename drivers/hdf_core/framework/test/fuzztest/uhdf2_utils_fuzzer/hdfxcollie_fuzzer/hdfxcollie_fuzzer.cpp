/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdfxcollie_fuzzer.h"

#include "hdf_xcollie.h"
#include "hdf_base.h"
#include "hdf_log.h"
#define HDF_LOG_TAG hdfxcollie_fuzzer

static int HdfXCollieFuzzTest()
{
    ::OHOS::HdfXCollie& inst = ::OHOS::HdfXCollie::GetInstance();
    (void)inst;
    return HDF_SUCCESS;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    int ret = HDF_FAILURE;
    ret = HdfXCollieFuzzTest();
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("HdfXCollieFuzzTest failed, ret is %{public}d", ret);
        return ret;
    }
    return HDF_SUCCESS;
}
