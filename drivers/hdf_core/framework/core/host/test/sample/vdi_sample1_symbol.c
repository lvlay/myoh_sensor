/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_base.h"
#include "hdf_load_vdi.h"
#include "hdf_log.h"

#include "vdi_sample1_symbol.h"

#define HDF_LOG_TAG vdi_sample1_symbol

static int ServiceA(void)
{
    HDF_LOGI("%{public}s", __func__);
    return HDF_SUCCESS;
}

static int ServiceB(struct ModuleA1 *modA)
{
    HDF_LOGI("%{public}s %{public}d", __func__, modA->priData);
    return HDF_SUCCESS;
}

static int SampleAOpen(struct HdfVdiBase *vdiBase)
{
    (void)vdiBase;
    HDF_LOGI("%{public}s", __func__);
    return HDF_SUCCESS;
}

static int SampleAClose(struct HdfVdiBase *vdiBase)
{
    (void)vdiBase;
    HDF_LOGI("%{public}s", __func__);
    return HDF_SUCCESS;
}

static struct ModuleA1 g_modA1 = {
    .ServiceA = ServiceA,
    .ServiceB = ServiceB,
    .priData = 1,
};

struct VdiWrapperA1 g_vdiA = {
    .base = {
        .moduleVersion = 1,
        .moduleName = "SampleServiceA",
        .CreateVdiInstance = SampleAOpen,
        .DestoryVdiInstance = SampleAClose,
    },
    .module = &g_modA1,
};
