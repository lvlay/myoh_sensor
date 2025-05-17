/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_device.h"
#include "hdf_base.h"
#include "hdf_device_node.h"
#include "hdf_device_token.h"
#include "hdf_log.h"
#include "hdf_object_manager.h"
#include "osal_mem.h"

#define HDF_LOG_TAG hdf_device

// device node id less than 129 is configured in hcs, dynamic allocation ID starts from 129
#define DYNAMIC_ALLOC_ID 129

static void UpdateDeivceNodeIdIndex(struct HdfDevice *device, devid_t nodeDevid)
{
    if (device->devidIndex < DEVNODEID(nodeDevid)) {
        device->devidIndex = DEVNODEID(nodeDevid);
    }
}

static devid_t FindUsableDevNodeId(struct HdfDevice *device)
{
    uint16_t nodeId = DYNAMIC_ALLOC_ID;
    bool find = false;
    struct HdfDeviceNode *devNode = NULL;
    for (; nodeId <= device->devidIndex; nodeId++) {
        find = false;
        DLIST_FOR_EACH_ENTRY(devNode, &device->devNodes, struct HdfDeviceNode, entry) {
            if (DEVNODEID(devNode->devId) == nodeId) {
                find = true;
                break;
            }
        }
        if (!find) {
            return nodeId;
        }
    }
    return nodeId;
}

static int AcquireNodeDeivceId(struct HdfDevice *device, devid_t *devid)
{
    devid_t nodeId;
    devid_t usableId;
    if (device->devidIndex >= DEVNODEID_MASK) {
        return HDF_FAILURE;
    }

    if (device->devidIndex < DYNAMIC_ALLOC_ID) {
        device->devidIndex = DYNAMIC_ALLOC_ID;
        nodeId = device->devidIndex;
    } else {
        usableId = FindUsableDevNodeId(device);
        if (usableId <= device->devidIndex) {
            nodeId = usableId;
        } else {
            device->devidIndex++;
            nodeId = device->devidIndex;
        }
    }

    if (devid == NULL) {
        HDF_LOGE("params invalid *devid");
        return HDF_ERR_INVALID_PARAM;
    }
    *devid = MK_DEVID(HOSTID(device->deviceId), DEVICEID(device->deviceId), nodeId);

    return HDF_SUCCESS;
}

static int HdfDeviceAttach(struct IHdfDevice *devInst, struct HdfDeviceNode *devNode)
{
    int ret;
    struct HdfDevice *device = (struct HdfDevice *)devInst;
    struct IDeviceNode *nodeIf = (struct IDeviceNode *)devNode;

    if (device == NULL || nodeIf == NULL || nodeIf->LaunchNode == NULL) {
        HDF_LOGE("failed to attach device, input params invalid");
        return HDF_ERR_INVALID_PARAM;
    }

    // for dynamic added device node, assign device id here
    if (devNode->devId == 0 && AcquireNodeDeivceId(device, &devNode->devId) != HDF_SUCCESS) {
        HDF_LOGE("failed to attach device, invalid device id");
        return HDF_ERR_INVALID_PARAM;
    }
    devNode->token->devid = devNode->devId;
    ret = nodeIf->LaunchNode(devNode);
    if (ret == HDF_SUCCESS) {
        DListInsertTail(&devNode->entry, &device->devNodes);
        UpdateDeivceNodeIdIndex(device, devNode->devId);
    }

    return ret;
}

int HdfDeviceDetach(struct IHdfDevice *devInst, struct HdfDeviceNode *devNode)
{
    struct HdfDevice *device = NULL;
    if (devInst == NULL || devNode == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    device = CONTAINER_OF(devInst, struct HdfDevice, super);
    if (DEVICEID(device->deviceId) != DEVICEID(devNode->devId)) {
        HDF_LOGE("%{public}s: device detach unknown devnode ",
            __func__);
        return HDF_DEV_ERR_NO_DEVICE;
    }

    if (devNode->entry.next != NULL) {
        DListRemove(&devNode->entry);
    }
    if (devNode->super.UnlaunchNode != NULL) {
        devNode->super.UnlaunchNode(devNode);
    }

    return HDF_SUCCESS;
}

static struct HdfDeviceNode *HdfDeviceGetDeviceNode(struct IHdfDevice *device, devid_t devid)
{
    struct HdfDeviceNode *devNode = NULL;
    struct HdfDevice *dev = CONTAINER_OF(device, struct HdfDevice, super);
    DLIST_FOR_EACH_ENTRY(devNode, &dev->devNodes, struct HdfDeviceNode, entry) {
        if (devNode->devId == devid) {
            return devNode;
        };
    }
    return NULL;
}

static int HdfDeviceDetachWithDevid(struct IHdfDevice *device, devid_t devid)
{
    struct HdfDevice *dev = CONTAINER_OF(device, struct HdfDevice, super);
    (void)dev;
    struct HdfDeviceNode *devNode = HdfDeviceGetDeviceNode(device, devid);
    if (devNode == NULL) {
        HDF_LOGE("devNode is NULL");
        return HDF_DEV_ERR_NO_DEVICE;
    }

    return HdfDeviceDetach(device, devNode);
}

void HdfDeviceConstruct(struct HdfDevice *device)
{
    device->super.Attach = HdfDeviceAttach;
    device->super.Detach = HdfDeviceDetach;
    device->super.DetachWithDevid = HdfDeviceDetachWithDevid;
    device->super.GetDeviceNode = HdfDeviceGetDeviceNode;

    DListHeadInit(&device->devNodes);
}

void HdfDeviceDestruct(struct HdfDevice *device)
{
    struct HdfDeviceNode *devNode = NULL;
    struct HdfDeviceNode *tmp = NULL;
    DLIST_FOR_EACH_ENTRY_SAFE(devNode, tmp, &device->devNodes, struct HdfDeviceNode, entry) {
        HdfDeviceNodeFreeInstance(devNode);
    }
    DListHeadInit(&device->devNodes);
}

struct HdfObject *HdfDeviceCreate(void)
{
    struct HdfDevice *device = (struct HdfDevice *)OsalMemCalloc(sizeof(struct HdfDevice));
    if (device != NULL) {
        HdfDeviceConstruct(device);
    }
    return (struct HdfObject *)device;
}

void HdfDeviceRelease(struct HdfObject *object)
{
    struct HdfDevice *device = (struct HdfDevice *)object;
    if (device != NULL) {
        HdfDeviceDestruct(device);
        OsalMemFree(device);
    }
}

struct HdfDevice *HdfDeviceNewInstance(void)
{
    return (struct HdfDevice *)HdfObjectManagerGetObject(HDF_OBJECT_ID_DEVICE);
}

void HdfDeviceFreeInstance(struct HdfDevice *device)
{
    if (device != NULL) {
        HdfObjectManagerFreeObject(&device->super.object);
    }
}
