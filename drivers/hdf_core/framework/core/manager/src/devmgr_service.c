/*
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "devmgr_service.h"
#include "devhost_service_clnt.h"
#include "device_token_clnt.h"
#include "devsvc_manager.h"
#include "hdf_attribute_manager.h"
#include "hdf_base.h"
#include "hdf_driver_installer.h"
#include "hdf_host_info.h"
#include "hdf_log.h"
#include "hdf_object_manager.h"
#include "osal_time.h"

#define HDF_LOG_TAG devmgr_service
#define INVALID_PID (-1)

static bool DevmgrServiceDynamicDevInfoFound(
    const char *svcName, struct DevHostServiceClnt **targetHostClnt, struct HdfDeviceInfo **targetDeviceInfo)
{
    struct HdfSListIterator itDeviceInfo;
    struct HdfDeviceInfo *deviceInfo = NULL;
    struct DevHostServiceClnt *hostClnt = NULL;
    struct DevmgrService *devMgrSvc = (struct DevmgrService *)DevmgrServiceGetInstance();
    if (devMgrSvc == NULL) {
        return false;
    }

    DLIST_FOR_EACH_ENTRY(hostClnt, &devMgrSvc->hosts, struct DevHostServiceClnt, node) {
        HdfSListIteratorInit(&itDeviceInfo, &hostClnt->dynamicDevInfos);
        while (HdfSListIteratorHasNext(&itDeviceInfo)) {
            deviceInfo = (struct HdfDeviceInfo *)HdfSListIteratorNext(&itDeviceInfo);
            if (strcmp(deviceInfo->svcName, svcName) == 0) {
                *targetDeviceInfo = deviceInfo;
                *targetHostClnt = hostClnt;
                return true;
            }
        }
    }

    return false;
}

#define WAIT_HOST_SLEEP_TIME    1 // ms
#define WAIT_HOST_SLEEP_CNT     1000
static int DevmgrServiceStartHostProcess(struct DevHostServiceClnt *hostClnt, bool sync, bool dynamic)
{
    int waitCount = WAIT_HOST_SLEEP_CNT;
    struct IDriverInstaller *installer = DriverInstallerGetInstance();
    if (installer == NULL || installer->StartDeviceHost == NULL) {
        HDF_LOGE("invalid installer");
        return HDF_FAILURE;
    }

    hostClnt->hostPid = installer->StartDeviceHost(hostClnt->hostId, hostClnt->hostName, dynamic);
    if (hostClnt->hostPid == HDF_FAILURE) {
        HDF_LOGW("failed to start device host(%{public}s, %{public}u)", hostClnt->hostName, hostClnt->hostId);
        return HDF_FAILURE;
    }
    hostClnt->stopFlag = false;
    if (!sync) {
        return HDF_SUCCESS;
    }

    while (hostClnt->hostService == NULL && waitCount > 0) {
        OsalMSleep(WAIT_HOST_SLEEP_TIME);
        waitCount--;
    }

    if (waitCount <= 0) {
        HDF_LOGE("wait host(%{public}s, %{public}d) attach timeout", hostClnt->hostName, hostClnt->hostId);
        hostClnt->hostPid = -1;
        return HDF_ERR_TIMEOUT;
    }

    return HDF_SUCCESS;
}

static int DevmgrServiceLoadDevice(struct IDevmgrService *devMgrSvc, const char *serviceName)
{
    struct HdfDeviceInfo *deviceInfo = NULL;
    struct DevHostServiceClnt *hostClnt = NULL;
    bool dynamic = true;
    int ret;
    (void)devMgrSvc;

    if (serviceName == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (!DevmgrServiceDynamicDevInfoFound(serviceName, &hostClnt, &deviceInfo)) {
        HDF_LOGE("device %{public}s not in configed device list", serviceName);
        return HDF_DEV_ERR_NO_DEVICE;
    }

    if (deviceInfo->preload != DEVICE_PRELOAD_DISABLE) {
        HDF_LOGE("device %{public}s not an dynamic load device", serviceName);
        return HDF_DEV_ERR_NORANGE;
    }

    dynamic = HdfSListIsEmpty(&hostClnt->unloadDevInfos) && !HdfSListIsEmpty(&hostClnt->dynamicDevInfos);
    OsalMutexLock(&hostClnt->hostLock);
    if (hostClnt->hostPid < 0) {
        OsalMutexUnlock(&hostClnt->hostLock);
        if (DevmgrServiceStartHostProcess(hostClnt, true, dynamic) != HDF_SUCCESS) {
            HDF_LOGW("failed to start device host(%{public}s, %{public}u)", hostClnt->hostName, hostClnt->hostId);
            return HDF_FAILURE;
        }
        OsalMutexLock(&hostClnt->hostLock);
    }

    if (hostClnt->hostService == NULL || hostClnt->hostService->AddDevice == NULL) {
        OsalMutexUnlock(&hostClnt->hostLock);
        HDF_LOGE("%{public}s load %{public}s failed, hostService is null", __func__, serviceName);
        return HDF_FAILURE;
    }
    ret = hostClnt->hostService->AddDevice(hostClnt->hostService, deviceInfo);
    OsalMutexUnlock(&hostClnt->hostLock);
    if (ret == HDF_SUCCESS) {
        deviceInfo->status = HDF_SERVICE_USABLE;
    }
    return ret;
}

static int DevmgrServiceStopHost(struct DevHostServiceClnt *hostClnt)
{
    struct IDriverInstaller *installer = DriverInstallerGetInstance();
    if (installer == NULL || installer->StopDeviceHost == NULL) {
        HDF_LOGE("invalid installer");
        return HDF_FAILURE;
    }
    installer->StopDeviceHost(hostClnt->hostId, hostClnt->hostName);
    hostClnt->stopFlag = true;
    return HDF_SUCCESS;
}

static int DevmgrServiceUnloadDevice(struct IDevmgrService *devMgrSvc, const char *serviceName)
{
    struct HdfDeviceInfo *deviceInfo = NULL;
    struct DevHostServiceClnt *hostClnt = NULL;
    int ret;
    (void)devMgrSvc;

    if (serviceName == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (!DevmgrServiceDynamicDevInfoFound(serviceName, &hostClnt, &deviceInfo) ||
        deviceInfo->preload != DEVICE_PRELOAD_DISABLE) {
        HDF_LOGE("device %{public}s not in configed dynamic device list", serviceName);
        return HDF_DEV_ERR_NO_DEVICE;
    }
    OsalMutexLock(&hostClnt->hostLock);
    if (hostClnt->hostService == NULL || hostClnt->hostService->DelDevice == NULL) {
        OsalMutexUnlock(&hostClnt->hostLock);
        HDF_LOGE("%{public}s unload %{public}s failed, hostService is null", __func__, serviceName);
        return HDF_FAILURE;
    }
    ret = hostClnt->hostService->DelDevice(hostClnt->hostService, deviceInfo->deviceId);
    if (ret != HDF_SUCCESS) {
        OsalMutexUnlock(&hostClnt->hostLock);
        HDF_LOGI("%{public}s:unload service %{public}s delDevice failed", __func__, serviceName);
        return ret;
    }
    deviceInfo->status = HDF_SERVICE_UNUSABLE;
    if (!HdfSListIsEmpty(&hostClnt->devices)) {
        OsalMutexUnlock(&hostClnt->hostLock);
        HDF_LOGD("%{public}s host %{public}s devices is not empty", __func__, hostClnt->hostName);
        return HDF_SUCCESS;
    }
    if (!HdfSListIsEmpty(&hostClnt->unloadDevInfos)) {
        OsalMutexUnlock(&hostClnt->hostLock);
        HDF_LOGD("%{public}s the hdf_devmgr need not to stop automatically started host %{public}s", __func__,
            hostClnt->hostName);
        return HDF_SUCCESS;
    }
    hostClnt->hostPid = INVALID_PID;
    hostClnt->hostService = NULL; // old hostService will be recycled in CleanupDiedHostResources
    HdfSListFlush(&hostClnt->devices, DeviceTokenClntDelete);
    OsalMutexUnlock(&hostClnt->hostLock);
    ret = DevmgrServiceStopHost(hostClnt);

    return ret;
}

int32_t DevmgrServiceLoadLeftDriver(struct DevmgrService *devMgrSvc)
{
    int32_t ret;
    struct HdfSListIterator itDeviceInfo;
    struct HdfDeviceInfo *deviceInfo = NULL;
    struct DevHostServiceClnt *hostClnt = NULL;
    if (devMgrSvc == NULL) {
        return HDF_FAILURE;
    }

    DLIST_FOR_EACH_ENTRY(hostClnt, &devMgrSvc->hosts, struct DevHostServiceClnt, node) {
        HdfSListIteratorInit(&itDeviceInfo, &hostClnt->unloadDevInfos);
        while (HdfSListIteratorHasNext(&itDeviceInfo)) {
            deviceInfo = (struct HdfDeviceInfo *)HdfSListIteratorNext(&itDeviceInfo);
            if (deviceInfo->preload == DEVICE_PRELOAD_ENABLE_STEP2) {
                ret = hostClnt->hostService->AddDevice(hostClnt->hostService, deviceInfo);
                if (ret != HDF_SUCCESS) {
                    HDF_LOGE("%{public}s:failed to load driver %{public}s", __func__, deviceInfo->moduleName);
                    continue;
                }
                deviceInfo->status = HDF_SERVICE_USABLE;
                HdfSListIteratorRemove(&itDeviceInfo);
            }
        }
    }
    return HDF_SUCCESS;
}

static struct DevHostServiceClnt *DevmgrServiceFindDeviceHost(struct IDevmgrService *inst, uint16_t hostId)
{
    struct DevHostServiceClnt *hostClnt = NULL;
    struct DevmgrService *dmService = (struct DevmgrService *)inst;
    if (dmService == NULL) {
        HDF_LOGE("failed to find device host, dmService is null");
        return NULL;
    }

    DLIST_FOR_EACH_ENTRY(hostClnt, &dmService->hosts, struct DevHostServiceClnt, node) {
        if (hostClnt->hostId == hostId) {
            return hostClnt;
        }
    }
    HDF_LOGE("cannot find host %{public}u", hostId);
    return NULL;
}

static int DevmgrServiceAttachDevice(struct IDevmgrService *inst, struct IHdfDeviceToken *token)
{
    struct DevHostServiceClnt *hostClnt = NULL;
    struct DeviceTokenClnt *tokenClnt = NULL;

    if (token == NULL) {
        return HDF_FAILURE;
    }
    hostClnt = DevmgrServiceFindDeviceHost(inst, HOSTID(token->devid));
    if (hostClnt == NULL) {
        HDF_LOGE("failed to attach device, hostClnt is null");
        return HDF_FAILURE;
    }
    tokenClnt = DeviceTokenClntNewInstance(token);
    if (tokenClnt == NULL) {
        HDF_LOGE("failed to attach device, tokenClnt is null");
        return HDF_FAILURE;
    }

    HdfSListAdd(&hostClnt->devices, &tokenClnt->node);
    return HDF_SUCCESS;
}

static bool HdfSListHostSearchDeviceTokenComparer(struct HdfSListNode *tokenNode, uint32_t devid)
{
    struct DeviceTokenClnt *tokenClnt = CONTAINER_OF(tokenNode, struct DeviceTokenClnt, node);
    return tokenClnt->tokenIf->devid == devid;
}

static int DevmgrServiceDetachDevice(struct IDevmgrService *inst, devid_t devid)
{
    struct DevHostServiceClnt *hostClnt = NULL;
    struct DeviceTokenClnt *tokenClnt = NULL;
    struct HdfSListNode *tokenClntNode = NULL;

    hostClnt = DevmgrServiceFindDeviceHost(inst, HOSTID(devid));
    if (hostClnt == NULL) {
        HDF_LOGE("failed to attach device, hostClnt is null");
        return HDF_FAILURE;
    }
    tokenClntNode = HdfSListSearch(&hostClnt->devices, devid, HdfSListHostSearchDeviceTokenComparer);
    if (tokenClntNode == NULL) {
        HDF_LOGE("devmgr detach devic not found");
        return HDF_DEV_ERR_NO_DEVICE;
    }
    tokenClnt = CONTAINER_OF(tokenClntNode, struct DeviceTokenClnt, node);
    HdfSListRemove(&hostClnt->devices, &tokenClnt->node);
    return HDF_SUCCESS;
}

static int DevmgrServiceAttachDeviceHost(
    struct IDevmgrService *inst, uint16_t hostId, struct IDevHostService *hostService)
{
    struct DevHostServiceClnt *hostClnt = DevmgrServiceFindDeviceHost(inst, hostId);
    if (hostClnt == NULL) {
        HDF_LOGE("failed to attach device host, hostClnt is null");
        return HDF_FAILURE;
    }
    if (hostService == NULL) {
        HDF_LOGE("failed to attach device host, hostService is null");
        return HDF_FAILURE;
    }

    (void)OsalMutexLock(&hostClnt->hostLock);
    hostClnt->hostService = hostService;
    (void)OsalMutexUnlock(&hostClnt->hostLock);
    return DevHostServiceClntInstallDriver(hostClnt);
}

static int DevmgrServiceStartDeviceHost(struct DevmgrService *devmgr, struct HdfHostInfo *hostAttr)
{
    struct DevHostServiceClnt *hostClnt = DevHostServiceClntNewInstance(hostAttr->hostId, hostAttr->hostName);
    if (hostClnt == NULL) {
        HDF_LOGW("failed to create new device host client");
        return HDF_FAILURE;
    }

    if (HdfAttributeManagerGetDeviceList(hostClnt) != HDF_SUCCESS) {
        HDF_LOGW("failed to get device list for host %{public}s", hostClnt->hostName);
        return HDF_FAILURE;
    }

    DListInsertTail(&hostClnt->node, &devmgr->hosts);

    // not start the host which only have dynamic devices
    if (HdfSListIsEmpty(&hostClnt->unloadDevInfos)) {
        return HDF_SUCCESS;
    }

    if (DevmgrServiceStartHostProcess(hostClnt, false, false) != HDF_SUCCESS) {
        HDF_LOGW("failed to start device host, host id is %{public}u", hostAttr->hostId);
        DListRemove(&hostClnt->node);
        DevHostServiceClntFreeInstance(hostClnt);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int DevmgrServiceStartDeviceHosts(struct DevmgrService *inst)
{
    int ret;
    struct HdfSList hostList;
    struct HdfSListIterator it;
    struct HdfHostInfo *hostAttr = NULL;

    HdfSListInit(&hostList);
    if (!HdfAttributeManagerGetHostList(&hostList)) {
        HDF_LOGW("%{public}s: host list is null", __func__);
        return HDF_SUCCESS;
    }
    HdfSListIteratorInit(&it, &hostList);
    while (HdfSListIteratorHasNext(&it)) {
        hostAttr = (struct HdfHostInfo *)HdfSListIteratorNext(&it);
        ret = DevmgrServiceStartDeviceHost(inst, hostAttr);
        if (ret != HDF_SUCCESS) {
            HDF_LOGW("%{public}s failed to start device host, host id is %{public}u, host name is '%{public}s'",
                __func__, hostAttr->hostId, hostAttr->hostName);
        }
    }
    HdfSListFlush(&hostList, HdfHostInfoDelete);
    return HDF_SUCCESS;
}

static int32_t DevmgrServiceListAllDevice(struct IDevmgrService *inst, struct HdfSBuf *reply)
{
    struct DevmgrService *devMgrSvc = (struct DevmgrService *)inst;
    struct DevHostServiceClnt *hostClnt = NULL;
    struct HdfSListIterator iterator;
    struct HdfSListNode *node = NULL;
    const char *name = NULL;

    if (devMgrSvc == NULL || reply == NULL) {
        HDF_LOGE("%{public}s failed, parameter is null", __func__);
        return HDF_FAILURE;
    }

    DLIST_FOR_EACH_ENTRY(hostClnt, &devMgrSvc->hosts, struct DevHostServiceClnt, node) {
        HdfSbufWriteString(reply, hostClnt->hostName);
        HdfSbufWriteUint32(reply, hostClnt->hostId);
        HdfSbufWriteUint32(reply, HdfSListCount(&hostClnt->devices));

        HdfSListIteratorInit(&iterator, &hostClnt->devices);
        while (HdfSListIteratorHasNext(&iterator)) {
            node = HdfSListIteratorNext(&iterator);
            struct DeviceTokenClnt *tokenClnt = (struct DeviceTokenClnt *)node;
            if (tokenClnt != NULL && tokenClnt->tokenIf != NULL) {
                name = (tokenClnt->tokenIf->deviceName == NULL) ? "" : tokenClnt->tokenIf->deviceName;
                HdfSbufWriteString(reply, name);
                HdfSbufWriteUint32(reply, tokenClnt->tokenIf->devid);
                name = (tokenClnt->tokenIf->servName == NULL) ? "" : tokenClnt->tokenIf->servName;
                HdfSbufWriteString(reply, name);
            } else {
                HDF_LOGI("%{public}s host:%{public}s token null", __func__, hostClnt->hostName);
            }
        }
    }
    return HDF_SUCCESS;
}

int DevmgrServiceStartService(struct IDevmgrService *inst)
{
    int ret;
    struct DevmgrService *dmService = (struct DevmgrService *)inst;
    if (dmService == NULL) {
        HDF_LOGE("failed to start device manager service, dmService is null");
        return HDF_FAILURE;
    }

    ret = DevmgrServiceStartDeviceHosts(dmService);
    int startServiceRet = DevSvcManagerStartService();
    HDF_LOGI("start svcmgr result %{public}d. Init DeviceHosts info result: %{public}d", startServiceRet, ret);
    return ret;
}

int DevmgrServicePowerStateChange(struct IDevmgrService *devmgrService, enum HdfPowerState powerState)
{
    struct DevHostServiceClnt *hostClient = NULL;
    struct DevmgrService *devmgr = NULL;
    int result = HDF_SUCCESS;

    if (devmgrService == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    if (!IsValidPowerState(powerState)) {
        HDF_LOGE("%{public}s:invalid power event %{public}u", __func__, powerState);
        return HDF_ERR_INVALID_PARAM;
    }
    devmgr = CONTAINER_OF(devmgrService, struct DevmgrService, super);

    if (IsPowerWakeState(powerState)) {
        HDF_LOGI("%{public}s:wake state %{public}u", __func__, powerState);
        DLIST_FOR_EACH_ENTRY(hostClient, &devmgr->hosts, struct DevHostServiceClnt, node) {
            if (hostClient->hostService != NULL) {
                if (hostClient->hostService->PmNotify(hostClient->hostService, powerState) != HDF_SUCCESS) {
                    result = HDF_FAILURE;
                }
            }
        }
    } else {
        HDF_LOGI("%{public}s:suspend state %{public}u", __func__, powerState);
        DLIST_FOR_EACH_ENTRY_REVERSE(hostClient, &devmgr->hosts, struct DevHostServiceClnt, node) {
            if (hostClient->hostService != NULL) {
                if (hostClient->hostService->PmNotify(hostClient->hostService, powerState) != HDF_SUCCESS) {
                    result = HDF_FAILURE;
                }
            }
        }
    }

    return result;
}

bool DevmgrServiceConstruct(struct DevmgrService *inst)
{
    if (inst == NULL) {
        HDF_LOGE("%{public}s:inst is null ", __func__);
        return false;
    }
    struct IDevmgrService *devMgrSvcIf = NULL;
    if (OsalMutexInit(&inst->devMgrMutex) != HDF_SUCCESS) {
        HDF_LOGE("%{public}s:failed to mutex init ", __func__);
        return false;
    }
    devMgrSvcIf = (struct IDevmgrService *)inst;
    if (devMgrSvcIf != NULL) {
        devMgrSvcIf->AttachDevice = DevmgrServiceAttachDevice;
        devMgrSvcIf->DetachDevice = DevmgrServiceDetachDevice;
        devMgrSvcIf->LoadDevice = DevmgrServiceLoadDevice;
        devMgrSvcIf->UnloadDevice = DevmgrServiceUnloadDevice;
        devMgrSvcIf->AttachDeviceHost = DevmgrServiceAttachDeviceHost;
        devMgrSvcIf->StartService = DevmgrServiceStartService;
        devMgrSvcIf->PowerStateChange = DevmgrServicePowerStateChange;
        devMgrSvcIf->ListAllDevice = DevmgrServiceListAllDevice;
        DListHeadInit(&inst->hosts);
        return true;
    } else {
        return false;
    }
}

struct HdfObject *DevmgrServiceCreate(void)
{
    static bool isDevMgrServiceInit = false;
    static struct DevmgrService devmgrServiceInstance;
    if (!isDevMgrServiceInit) {
        if (!DevmgrServiceConstruct(&devmgrServiceInstance)) {
            return NULL;
        }
        isDevMgrServiceInit = true;
    }
    return (struct HdfObject *)&devmgrServiceInstance;
}

struct IDevmgrService *DevmgrServiceGetInstance(void)
{
    static struct IDevmgrService *instance = NULL;
    if (instance == NULL) {
        instance = (struct IDevmgrService *)HdfObjectManagerGetObject(HDF_OBJECT_ID_DEVMGR_SERVICE);
    }
    return instance;
}

void DevmgrServiceRelease(struct HdfObject *object)
{
    struct DevmgrService *devmgrService = (struct DevmgrService *)object;
    struct DevHostServiceClnt *hostClnt = NULL;
    struct DevHostServiceClnt *hostClntTmp = NULL;
    if (devmgrService == NULL) {
        return;
    }
    DLIST_FOR_EACH_ENTRY_SAFE(hostClnt, hostClntTmp, &devmgrService->hosts, struct DevHostServiceClnt, node) {
        DListRemove(&hostClnt->node);
        DevHostServiceClntDelete(hostClnt);
    }

    OsalMutexDestroy(&devmgrService->devMgrMutex);
}
