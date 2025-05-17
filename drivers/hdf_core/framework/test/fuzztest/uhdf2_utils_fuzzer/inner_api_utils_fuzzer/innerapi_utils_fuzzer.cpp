/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "innerapi_utils_fuzzer.h"

#include "hdf_dlist.h"
#include "hdf_base.h"
#include "hdf_log.h"

namespace OHOS {
constexpr size_t THRESHOLD = 10;
#define HDF_LOG_TAG innerapi_utils_fuzzer

enum FuncId {
    INNERAPI_UTILS_DLISTINSERTHEAD,
    INNERAPI_UTILS_END
};

void FuncdListInsertHead(const uint8_t *data, size_t size)
{
    (void)data;
    constexpr uint16_t cnt = 2;
    if (size < sizeof(struct DListHead) * cnt) {
        return;
    }

    struct DListHead entry;
    struct DListHead head;
    DListHeadInit(&head);
    DListInsertHead(&entry, &head);
    return;
}


void FuncSwitch(uint32_t cmd, const uint8_t *data, size_t size)
{
    switch (cmd) {
        case INNERAPI_UTILS_DLISTINSERTHEAD: {
            FuncdListInsertHead(data, size);
            break;
        }

        default:
            return;
    }
}

void TraverseAllFunc(const uint8_t *data, size_t size)
{
    for (uint32_t cmd = 0; cmd < INNERAPI_UTILS_END; cmd++) {
        FuncSwitch(cmd, data, size);
    }
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::THRESHOLD) {
        return HDF_SUCCESS;
    }

    OHOS::TraverseAllFunc(data, size);
    return HDF_SUCCESS;
}
