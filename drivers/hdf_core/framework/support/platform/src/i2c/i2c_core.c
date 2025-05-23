/*
 * Copyright (c) 2020-2023 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */
#include "i2c_core.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "i2c_msg.h"
#include "i2c_service.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "platform_core.h"

#define HDF_LOG_TAG         i2c_core
#define LOCK_WAIT_SECONDS_M 1
#define I2C_HANDLE_SHIFT    ((uintptr_t)(-1) << 16)

struct I2cManager {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    struct I2cCntlr *cntlrs[I2C_BUS_MAX];
    struct OsalMutex lock;
};

static struct I2cManager *g_i2cManager = NULL;

static int32_t I2cCntlrLockDefault(struct I2cCntlr *cntlr)
{
    if (cntlr == NULL) {
        HDF_LOGE("I2cCntlrLockDefault: cntlr is null!");
        return HDF_ERR_INVALID_OBJECT;
    }
    return OsalMutexLock(&cntlr->lock);
}

static void I2cCntlrUnlockDefault(struct I2cCntlr *cntlr)
{
    if (cntlr == NULL) {
        HDF_LOGE("I2cCntlrUnlockDefault: cntlr is null!");
        return;
    }
    (void)OsalMutexUnlock(&cntlr->lock);
}

static const struct I2cLockMethod g_i2cLockOpsDefault = {
    .lock = I2cCntlrLockDefault,
    .unlock = I2cCntlrUnlockDefault,
};

static int32_t I2cManagerAddCntlr(struct I2cCntlr *cntlr)
{
    int32_t ret;
    struct I2cManager *manager = g_i2cManager;

    if (cntlr->busId >= I2C_BUS_MAX) {
        HDF_LOGE("I2cManagerAddCntlr: busId:%hd exceed!", cntlr->busId);
        return HDF_ERR_INVALID_PARAM;
    }

    if (manager == NULL) {
        HDF_LOGE("I2cManagerAddCntlr: get i2c manager fail!");
        return HDF_ERR_NOT_SUPPORT;
    }
    if (OsalMutexLock(&manager->lock) != HDF_SUCCESS) {
        HDF_LOGE("I2cManagerAddCntlr: lock i2c manager fail!");
        return HDF_ERR_DEVICE_BUSY;
    }

    if (manager->cntlrs[cntlr->busId] != NULL) {
        HDF_LOGE("I2cManagerAddCntlr: cntlr of bus:%hd already exits!", cntlr->busId);
        ret = HDF_FAILURE;
    } else {
        manager->cntlrs[cntlr->busId] = cntlr;
        ret = HDF_SUCCESS;
    }

    (void)OsalMutexUnlock(&manager->lock);
    return ret;
}

static void I2cManagerRemoveCntlr(struct I2cCntlr *cntlr)
{
    struct I2cManager *manager = g_i2cManager;

    if (cntlr->busId < 0 || cntlr->busId >= I2C_BUS_MAX) {
        HDF_LOGE("I2cManagerRemoveCntlr: invalid busId:%hd!", cntlr->busId);
        return;
    }

    if (manager == NULL) {
        HDF_LOGE("I2cManagerRemoveCntlr: get i2c manager fail!");
        return;
    }
    if (OsalMutexLock(&manager->lock) != HDF_SUCCESS) {
        HDF_LOGE("I2cManagerRemoveCntlr: lock i2c manager fail!");
        return;
    }

    if (manager->cntlrs[cntlr->busId] != cntlr) {
        HDF_LOGE("I2cManagerRemoveCntlr: cntlr(%hd) not in manager!", cntlr->busId);
    } else {
        manager->cntlrs[cntlr->busId] = NULL;
    }

    (void)OsalMutexUnlock(&manager->lock);
}

/*
 * Find an i2c controller by bus number, without ref count
 */
static struct I2cCntlr *I2cManagerFindCntlr(int16_t number)
{
    struct I2cCntlr *cntlr = NULL;
    struct I2cManager *manager = g_i2cManager;

    if (number < 0 || number >= I2C_BUS_MAX) {
        HDF_LOGE("I2cManagerFindCntlr: invalid busId:%hd!", number);
        return NULL;
    }

    if (manager == NULL) {
        HDF_LOGE("I2cManagerFindCntlr: get i2c manager fail!");
        return NULL;
    }

    if (OsalMutexLock(&manager->lock) != HDF_SUCCESS) {
        HDF_LOGE("I2cManagerFindCntlr: lock i2c manager fail!");
        return NULL;
    }
    cntlr = manager->cntlrs[number];
    (void)OsalMutexUnlock(&manager->lock);
    return cntlr;
}

/*
 * Find and return an i2c controller by number, with ref count
 */
struct I2cCntlr *I2cCntlrGet(int16_t number)
{
    return I2cManagerFindCntlr(number);
}

/*
 * Put back the i2c controller, with ref count
 */
void I2cCntlrPut(struct I2cCntlr *cntlr)
{
    (void)cntlr;
}

int32_t I2cCntlrAdd(struct I2cCntlr *cntlr)
{
    int32_t ret;

    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    if (cntlr->ops == NULL) {
        HDF_LOGE("I2cCntlrAdd: no ops supplied!");
        return HDF_ERR_INVALID_OBJECT;
    }

    if (cntlr->lockOps == NULL) {
        HDF_LOGD("I2cCntlrAdd: use default lock methods!");
        cntlr->lockOps = &g_i2cLockOpsDefault;
    }

    if (OsalMutexInit(&cntlr->lock) != HDF_SUCCESS) {
        HDF_LOGE("I2cCntlrAdd: init lock fail!");
        return HDF_FAILURE;
    }

    ret = I2cManagerAddCntlr(cntlr);
    if (ret != HDF_SUCCESS) {
        (void)OsalMutexDestroy(&cntlr->lock);
        return ret;
    }
    return HDF_SUCCESS;
}

void I2cCntlrRemove(struct I2cCntlr *cntlr)
{
    if (cntlr == NULL) {
        return;
    }
    I2cManagerRemoveCntlr(cntlr);
    (void)OsalMutexDestroy(&cntlr->lock);
}

static inline int32_t I2cCntlrLock(struct I2cCntlr *cntlr)
{
    if (cntlr->lockOps == NULL || cntlr->lockOps->lock == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return cntlr->lockOps->lock(cntlr);
}

static inline void I2cCntlrUnlock(struct I2cCntlr *cntlr)
{
    if (cntlr->lockOps != NULL && cntlr->lockOps->unlock != NULL) {
        cntlr->lockOps->unlock(cntlr);
    }
}

int32_t I2cCntlrTransfer(struct I2cCntlr *cntlr, struct I2cMsg *msgs, int16_t count)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("I2cCntlrTransfer: cntlr is null!");
        return HDF_ERR_INVALID_OBJECT;
    }

    if (cntlr->ops == NULL || cntlr->ops->transfer == NULL) {
        HDF_LOGE("I2cCntlrTransfer: ops or transfer is null!");
        return HDF_ERR_NOT_SUPPORT;
    }

    if (I2cCntlrLock(cntlr) != HDF_SUCCESS) {
        HDF_LOGE("I2cCntlrTransfer: lock controller fail!");
        return HDF_ERR_DEVICE_BUSY;
    }
    ret = cntlr->ops->transfer(cntlr, msgs, count);
    I2cCntlrUnlock(cntlr);
    return ret;
}

#if defined(LOSCFG_USER_I2C_SUPPORT) || defined(CONFIG_USER_I2C_SUPPORT)
static int32_t I2cIoDoTransfer(struct I2cCntlr *cntlr, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    struct I2cMsg *msgs = NULL;
    int16_t count = 0;
    int16_t transCnt = 0;
    int32_t ret = HDF_SUCCESS;

    if (I2cMsgsRebuildFromSbuf(data, &msgs, &count) != HDF_SUCCESS) {
        HDF_LOGE("I2cIoDoTransfer: fail to rebuild i2c msg!");
        return HDF_ERR_INVALID_PARAM;
    }

    transCnt = I2cCntlrTransfer(cntlr, msgs, count);
    if (transCnt != count) {
        I2cMsgsFree(msgs, count);
        HDF_LOGE("I2cIoDoTransfer: i2c transfer fail!");
        return HDF_ERR_IO;
    }

    if (I2cMsgsWriteToSbuf(msgs, count, reply) != HDF_SUCCESS) {
        HDF_LOGE("I2cIoDoTransfer: fail to write i2c msg!");
        ret = HDF_ERR_INVALID_PARAM;
    }
    I2cMsgsFree(msgs, count);

    return ret;
}

// user data format:handle---count---count data records of I2cUserMsg;
static int32_t I2cManagerIoTransfer(struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int16_t number;
    uint32_t handle;
    struct I2cCntlr *cntlr = NULL;

    if (data == NULL || reply == NULL) {
        HDF_LOGE("I2cManagerIoTransfer: data or reply is null!");
        return HDF_ERR_INVALID_PARAM;
    }
    if (!HdfSbufReadUint32(data, &handle)) {
        HDF_LOGE("I2cManagerIoTransfer: read handle fail!");
        return HDF_ERR_IO;
    }

    number = (int16_t)(handle - I2C_HANDLE_SHIFT);
    cntlr = I2cManagerFindCntlr(number);
    if (cntlr == NULL || cntlr->ops == NULL || cntlr->ops->transfer == NULL) {
        HDF_LOGE("I2cManagerIoTransfer: invalid cntlr num %d!", number);
        return HDF_ERR_NOT_SUPPORT;
    }

    return I2cIoDoTransfer(cntlr, data, reply);
}

static int32_t I2cManagerIoOpen(struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int16_t number;
    uint32_t handle;

    if (!HdfSbufReadUint16(data, (uint16_t *)&number)) {
        HDF_LOGE("I2cManagerIoOpen: read nuber fail!");
        return HDF_ERR_IO;
    }

    if (number < 0 || number >= I2C_BUS_MAX || reply == NULL) {
        HDF_LOGE("I2cManagerIoOpen: number is invalid or reply is null!");
        return HDF_ERR_INVALID_PARAM;
    }

    if (I2cCntlrGet(number) == NULL) {
        HDF_LOGE("I2cManagerIoOpen: i2c cnltr get fail!");
        return HDF_ERR_NOT_SUPPORT;
    }

    handle = (uint32_t)(number + I2C_HANDLE_SHIFT);
    if (!HdfSbufWriteUint32(reply, handle)) {
        HDF_LOGE("I2cManagerIoOpen: write handle fail!");
        return HDF_ERR_IO;
    }
    return HDF_SUCCESS;
}

static int32_t I2cManagerIoClose(struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int16_t number;
    uint32_t handle;

    (void)reply;
    if (!HdfSbufReadUint32(data, &handle)) {
        HDF_LOGE("I2cManagerIoClose: read handle fail!");
        return HDF_ERR_IO;
    }

    number = (int16_t)(handle - I2C_HANDLE_SHIFT);
    if (number < 0 || number >= I2C_BUS_MAX) {
        HDF_LOGE("I2cManagerIoClose: number is invalid!");
        return HDF_ERR_INVALID_PARAM;
    }
    I2cCntlrPut(I2cManagerFindCntlr(number));
    return HDF_SUCCESS;
}

static int32_t I2cManagerDispatch(
    struct HdfDeviceIoClient *client, int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ret;

    (void)client;
    switch (cmd) {
        case I2C_IO_OPEN:
            return I2cManagerIoOpen(data, reply);
        case I2C_IO_CLOSE:
            return I2cManagerIoClose(data, reply);
        case I2C_IO_TRANSFER:
            return I2cManagerIoTransfer(data, reply);
        default:
            HDF_LOGE("I2cManagerDispatch: cmd %d is not support!", cmd);
            ret = HDF_ERR_NOT_SUPPORT;
            break;
    }
    return ret;
}
#endif // USER_I2C_SUPPORT
static int32_t I2cManagerBind(struct HdfDeviceObject *device)
{
    int32_t ret;
    struct I2cManager *manager = NULL;

    HDF_LOGI("I2cManagerBind: enter!");
    if (device == NULL) {
        HDF_LOGE("I2cManagerBind: device is null!");
        return HDF_ERR_INVALID_OBJECT;
    }

    manager = (struct I2cManager *)OsalMemCalloc(sizeof(*manager));
    if (manager == NULL) {
        HDF_LOGE("I2cManagerBind: malloc manager fail!");
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = OsalMutexInit(&manager->lock);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("I2cManagerBind: mutex init fail, ret: %d!", ret);
        OsalMemFree(manager);
        return HDF_FAILURE;
    }

    manager->device = device;
#if defined(LOSCFG_USER_I2C_SUPPORT) || defined(CONFIG_USER_I2C_SUPPORT)
    device->service = &manager->service;
    device->service->Dispatch = I2cManagerDispatch;
#endif // USER_I2C_SUPPORT
    g_i2cManager = manager;
    return HDF_SUCCESS;
}

static int32_t I2cManagerInit(struct HdfDeviceObject *device)
{
    (void)device;
    return HDF_SUCCESS;
}

static void I2cManagerRelease(struct HdfDeviceObject *device)
{
    struct I2cManager *manager = NULL;

    HDF_LOGI("I2cManagerRelease: enter!");
    if (device == NULL) {
        HDF_LOGI("I2cManagerRelease: device is null!");
        return;
    }
    manager = (struct I2cManager *)device->service;
    if (manager == NULL) {
        HDF_LOGI("I2cManagerRelease: no service binded!");
        return;
    }
    g_i2cManager = NULL;
    OsalMemFree(manager);
}

struct HdfDriverEntry g_i2cManagerEntry = {
    .moduleVersion = 1,
    .Bind = I2cManagerBind,
    .Init = I2cManagerInit,
    .Release = I2cManagerRelease,
    .moduleName = "HDF_PLATFORM_I2C_MANAGER",
};
HDF_INIT(g_i2cManagerEntry);
