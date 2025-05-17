/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <fcntl.h>
#include <hdf_base.h>
#include <hdf_device_desc.h>
#include <hdf_device_object.h>
#include <hdf_log.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <osal_mem.h>
#include <stub_collector.h>
#include "v1_1/iwpa_interface.h"
#include "wpa_impl.h"

struct HdfWpaInterfaceHost {
    struct IDeviceIoService ioService;
    struct IWpaInterface *service;
    struct HdfRemoteService **stubObject;
};

static int32_t WpaInterfaceDriverDispatch(
    struct HdfDeviceIoClient *client, int cmdId, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    HDF_LOGI("WpaInterfaceDriverDispatch enter.");
    struct HdfWpaInterfaceHost *wpainterfaceHost = CONTAINER_OF(
        client->device->service, struct HdfWpaInterfaceHost, ioService);
    if (wpainterfaceHost->service == NULL || wpainterfaceHost->stubObject == NULL) {
        HDF_LOGE("%{public}s: invalid service obj", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    struct HdfRemoteService *stubObj = *wpainterfaceHost->stubObject;
    if (stubObj == NULL || stubObj->dispatcher == NULL || stubObj->dispatcher->Dispatch == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    return stubObj->dispatcher->Dispatch((struct HdfRemoteService *)stubObj->target, cmdId, data, reply);
}

static int HdfWpaInterfaceDriverInit(struct HdfDeviceObject *deviceObject)
{
    int32_t ret;
    HDF_LOGI("HdfWpaInterfaceDriverInit enter.");
    struct HdfWpaStubData *stubData = HdfWpaStubDriver();
    DListHeadInit(&stubData->remoteListHead);
    ret = OsalMutexInit(&stubData->mutex);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: Mutex init failed, error code: %{public}d", __func__, ret);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int HdfWpaInterfaceDriverBind(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGI("HdfWpaInterfaceDriverBind enter.");

    int32_t ret = HdfDeviceObjectSetInterfaceDesc(deviceObject, IWPAINTERFACE_INTERFACE_DESC);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("failed to set interface descriptor of device object");
        return ret;
    }

    struct HdfWpaInterfaceHost *wpainterfaceHost =
        (struct HdfWpaInterfaceHost *)OsalMemAlloc(sizeof(struct HdfWpaInterfaceHost));
    if (wpainterfaceHost == NULL) {
        HDF_LOGE("HdfWpaInterfaceDriverBind OsalMemAlloc HdfWpaInterfaceHost failed!");
        return HDF_FAILURE;
    }

    struct IWpaInterface *serviceImpl = IWpaInterfaceGet(true);
    struct HdfRemoteService **stubObj = StubCollectorGetOrNewObject(IWPAINTERFACE_INTERFACE_DESC, serviceImpl);
    if (stubObj == NULL) {
        OsalMemFree(wpainterfaceHost);
        wpainterfaceHost = NULL;
        IWpaInterfaceRelease(serviceImpl, true);
        return HDF_FAILURE;
    }

    wpainterfaceHost->ioService.Dispatch = WpaInterfaceDriverDispatch;
    wpainterfaceHost->ioService.Open = NULL;
    wpainterfaceHost->ioService.Release = NULL;
    wpainterfaceHost->service = serviceImpl;
    wpainterfaceHost->stubObject = stubObj;
    deviceObject->service = &wpainterfaceHost->ioService;
    return HDF_SUCCESS;
}

static void HdfWpaInterfaceDriverRelease(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGI("HdfWpaInterfaceDriverRelease enter.");
    struct HdfWpaRemoteNode *pos = NULL;
    struct HdfWpaRemoteNode *tmp = NULL;
    struct HdfWpaStubData *stubData = HdfWpaStubDriver();
    if (stubData == NULL) {
        HDF_LOGE("%{public}s: stubData is NUll!", __func__);
        return;
    }

    DLIST_FOR_EACH_ENTRY_SAFE(pos, tmp, &stubData->remoteListHead, struct HdfWpaRemoteNode, node) {
        DListRemove(&(pos->node));
        OsalMemFree(pos);
        pos = NULL;
    }
    OsalMutexDestroy(&stubData->mutex);
    struct HdfWpaInterfaceHost *wpainterfaceHost = CONTAINER_OF(
        deviceObject->service, struct HdfWpaInterfaceHost, ioService);
    StubCollectorRemoveObject(IWPAINTERFACE_INTERFACE_DESC, wpainterfaceHost->service);
    IWpaInterfaceRelease(wpainterfaceHost->service, true);
    OsalMemFree(wpainterfaceHost);
    wpainterfaceHost = NULL;
}

struct HdfDriverEntry g_wpainterfaceDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "wpa_service",
    .Bind = HdfWpaInterfaceDriverBind,
    .Init = HdfWpaInterfaceDriverInit,
    .Release = HdfWpaInterfaceDriverRelease,
};

HDF_INIT(g_wpainterfaceDriverEntry);
