/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "devmgr_service_start.h"
#include "devmgr_service.h"
#include "devsvc_manager.h"
#include "devsvc_manager_clnt.h"
#include "hdf_base.h"
#include "hdf_io_service.h"
#include "hdf_log.h"
#include "hdf_sbuf.h"

#define DEV_MGR_NODE_PERM 0660

static int g_isQuickLoad = DEV_MGR_SLOW_LOAD;

int32_t HdfGetServiceNameByDeviceClass(DeviceClass deviceClass, struct HdfSBuf *reply)
{
    if (reply == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    HdfSbufFlush(reply);
    DevSvcManagerListService(reply, deviceClass);
    HdfSbufWriteString(reply, NULL);
    return HDF_SUCCESS;
}

int32_t HdfLoadDriverByServiceName(const char *svcName)
{
    if (svcName == NULL) {
        HDF_LOGE("%s: load svc name is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    static struct SubscriberCallback callback = {
        .deviceObject = NULL,
        .OnServiceConnected = NULL,
    };
    return DevSvcManagerClntSubscribeService(svcName, callback);
}

int DeviceManagerUnloadService(const char *svcName)
{
    if (svcName == NULL) {
        HDF_LOGE("%s: unload svc name is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return DevSvcManagerClntUnsubscribeService(svcName);
}

void DeviceManagerSetQuickLoad(int loadFlag)
{
    g_isQuickLoad = loadFlag;
}

int DeviceManagerIsQuickLoad(void)
{
    return g_isQuickLoad;
}

int DeviceManagerStart(void)
{
    struct IDevmgrService *instance = DevmgrServiceGetInstance();
    if (instance == NULL || instance->StartService == NULL) {
        HDF_LOGE("Device manager start failed, service instance is null!");
        return HDF_FAILURE;
    }

    int ret = instance->StartService(instance);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("failed to start hdf devmgr");
        return ret;
    }

    return HDF_SUCCESS;
}

int DeviceManagerStartStep2(void)
{
    if (DeviceManagerIsQuickLoad() == DEV_MGR_SLOW_LOAD) {
        HDF_LOGW("%s device manager is not set quick load!", __func__);
        return HDF_SUCCESS;
    }
    struct DevmgrService *devMgrSvc = (struct DevmgrService *)DevmgrServiceGetInstance();
    return DevmgrServiceLoadLeftDriver(devMgrSvc);
}
