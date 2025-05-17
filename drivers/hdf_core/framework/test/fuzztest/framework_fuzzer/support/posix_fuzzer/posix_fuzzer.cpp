/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_sem.h"
#include "osal_time.h"
#include "osal_spinlock.h"
#include "posix_fuzzer.h"
#include <thread>

namespace OHOS {
constexpr size_t THRESHOLD = 10;
#define HDF_LOG_TAG posix_fuzzer

enum FuncId {
    POSIX_OSALMEMALLOC,
    POSIX_OSALMEMCALLOC,
    POSIX_OSALMEMALLOCALIGN,
    POSIX_OSALSEMWAIT,
    POSIX_OSALSEMPOST,
    POSIX_OSALSPINLOCK,
    POSIX_OSALTIME,
    HCS_END
};

void FuncOsalMemAlloc(const uint8_t *data, size_t size)
{
    if (size < sizeof(size_t)) {
        return;
    }

    size_t sz = *(size_t *)data;
    uint8_t *mem = (uint8_t *)OsalMemAlloc(sz);
    if (mem != nullptr) {
        OsalMemFree(mem);
    }
    return;
}

void FuncOsalMemCalloc(const uint8_t *data, size_t size)
{
    if (size < sizeof(size_t)) {
        return;
    }

    size_t sz = *(size_t *)data;
    uint8_t *mem = (uint8_t *)OsalMemCalloc(sz);
    if (mem != nullptr) {
        OsalMemFree(mem);
    }
    return;
}

void FuncOsalMemAllocAlign(const uint8_t *data, size_t size)
{
    if (size < sizeof(size_t)) {
        return;
    }
    size_t alignment = 8;
    size_t sz = 0;

    uint8_t *mem = (uint8_t *)OsalMemAllocAlign(alignment, sz);
    if (mem != nullptr) {
        OsalMemFree(mem);
    }
    const size_t NUM = 4;
    sz = NUM;
    mem = (uint8_t *)OsalMemAllocAlign(alignment, sz);
    if (mem != nullptr) {
        OsalMemFree(mem);
    }
    return;
}

void FuncOsalSemWait(const uint8_t *data, size_t size)
{
    OsalSemWait(NULL, 0);

    struct OsalSem sem;
    OsalSemInit(&sem, 1);
    OsalSemWait(&sem, ~HDF_WAIT_FOREVER);
    OsalSemDestroy(&sem);
    return;
}

void FuncOsalSemPost(const uint8_t *data, size_t size)
{
    struct OsalSem sem;
    OsalSemInit(&sem, 1);
    OsalSemPost(&sem);
    OsalSemDestroy(&sem);
    return;
}

void FuncOsalSpinLock(const uint8_t *data, size_t size)
{
    OsalSpinInit(NULL);
    OsalSpinlock spinlock;
    OsalSpinInit(&spinlock);
    const uint32_t SLEEPTIME = 2000;
    std::thread thrd([&spinlock] {
        OsalMSleep(SLEEPTIME);
        OsalSpinUnlock(&spinlock);
    });
    OsalSpinLock(&spinlock);
    OsalSpinDestroy(&spinlock);
    thrd.join();
    return;
}

void FuncOsalTime(const uint8_t *data, size_t size)
{
    OsalGetTime(NULL);
    OsalTimespec start, end, diff;
    OsalGetTime(&start);
    OsalGetTime(&end);
    OsalDiffTime(&start, &end, &diff);
    OsalUSleep(1);
    OsalUDelay(1);
    OsalMDelay(1);
    uint64_t st = OsalGetSysTimeMs();
    (void)st;
    return;
}

void FuncSwitch(uint32_t cmd, const uint8_t *data, size_t size)
{
    switch (cmd) {
        case POSIX_OSALMEMALLOC: {
            FuncOsalMemAlloc(data, size);
            break;
        }
        case POSIX_OSALMEMCALLOC: {
            FuncOsalMemCalloc(data, size);
            break;
        }
        case POSIX_OSALMEMALLOCALIGN: {
            FuncOsalMemAllocAlign(data, size);
            break;
        }
        case POSIX_OSALSEMWAIT: {
            FuncOsalSemWait(data, size);
            break;
        }
        case POSIX_OSALSEMPOST: {
            FuncOsalSemPost(data, size);
            break;
        }
        case POSIX_OSALSPINLOCK: {
            FuncOsalSpinLock(data, size);
            break;
        }
        case POSIX_OSALTIME: {
            FuncOsalTime(data, size);
            break;
        }
        default:
            return;
    }
}

void TraverseAllFunc(const uint8_t *data, size_t size)
{
    for (uint32_t cmd = 0; cmd < HCS_END; cmd++) {
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
