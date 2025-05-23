/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "adc_if.h"
#include "adc_core.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "securec.h"
#define HDF_LOG_TAG adc_if_c
#define ADC_SERVICE_NAME "HDF_PLATFORM_ADC_MANAGER"

DevHandle AdcOpen(uint32_t number)
{
    int32_t ret;
    struct AdcDevice *device = NULL;

    device = AdcDeviceGet(number);
    if (device == NULL) {
        HDF_LOGE("AdcOpen: device is null!");
        return NULL;
    }

    ret = AdcDeviceStart(device);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("AdcOpen: adc device start fail!");
        return NULL;
    }

    return (DevHandle)device;
}

void AdcClose(DevHandle handle)
{
    struct AdcDevice *device = (struct AdcDevice *)handle;

    if (device == NULL) {
        HDF_LOGE("AdcClose: device is null!");
        return;
    }

    (void)AdcDeviceStop(device);
    AdcDevicePut(device);
}

int32_t AdcRead(DevHandle handle, uint32_t channel, uint32_t *val)
{
    if (handle == NULL) {
        HDF_LOGE("AdcRead: invalid handle!");
        return HDF_ERR_INVALID_PARAM;
    }
    return AdcDeviceRead((struct AdcDevice *)handle, channel, val);
}
