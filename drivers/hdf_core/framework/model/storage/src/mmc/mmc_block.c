/*
 * Copyright (c) 2020-2023 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "securec.h"

#include "hdf_base.h"
#include "hdf_log.h"
#include "mmc_block.h"

#define HDF_LOG_TAG mmc_block_c

int32_t MmcBlockInit(struct MmcDevice *mmcDevice)
{
    int32_t ret;
    size_t nameSize;
    struct MmcBlock *mb = NULL;

    if (mmcDevice == NULL) {
        HDF_LOGE("MmcBlockInit: mmcDevice is null!");
        return HDF_ERR_INVALID_OBJECT;
    }

    if (MmcDeviceGet(mmcDevice) == NULL) {
        HDF_LOGE("MmcBlockInit: mmcDevice get fail!");
        return HDF_PLT_ERR_DEV_GET;
    }

    mb = (struct MmcBlock *)OsalMemCalloc(sizeof(*mb));
    if (mb == NULL) {
        HDF_LOGE("MmcBlockInit: memcalloc fail!");
        PlatformDevicePut(&mmcDevice->device);
        return HDF_ERR_MALLOC_FAIL;
    }

    mmcDevice->mb = mb;
    mb->mmc = mmcDevice;
    mb->secSize = mmcDevice->secSize;
    mb->capacity = mmcDevice->capacity;
    mb->removable = (mmcDevice->state.bits.removable == 0) ? false : true;
    nameSize = sizeof(mb->name);
    ret = snprintf_s(mb->name, nameSize, nameSize - 1, "/dev/mmcblk%0d", mmcDevice->cntlr->index);
    if (ret <= 0) {
        OsalMemFree(mb);
        PlatformDevicePut(&mmcDevice->device);
        HDF_LOGE("MmcBlockInit: format block dev name fail, ret = %d!", ret);
        return HDF_PLT_ERR_OS_API;
    }

    ret = MmcBlockOsInit(mmcDevice);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("MmcBlockInit: mmc block os init fail, ret = %d!", ret);
        OsalMemFree(mb);
        PlatformDevicePut(&mmcDevice->device);
        return ret;
    }
    return HDF_SUCCESS;
}

void MmcBlockUninit(struct MmcDevice *mmcDevice)
{
    if (mmcDevice != NULL) {
        MmcBlockOsUninit(mmcDevice);
        OsalMemFree(mmcDevice->mb);
        mmcDevice->mb = NULL;
        PlatformDevicePut(&mmcDevice->device);
    }
}
