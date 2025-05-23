/*
 * Copyright (c) 2020-2023 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "mmc_dispatch.h"
#include "hdf_log.h"
#include "mmc_corex.h"
#include "mmc_emmc.h"
#include "mmc_sdio.h"

static int32_t MmcCmdDevPresent(struct MmcCntlr *cntlr, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)data;
    if (reply == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    if (!HdfSbufWriteUint8(reply, MmcCntlrDevPresent(cntlr))) {
        HDF_LOGE("MmcCmdDevPresent: write reply sbuf fail!");
        return HDF_ERR_IO;
    }
    return HDF_SUCCESS;
}

static int32_t MmcDispatch(struct MmcCntlr *cntlr, int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    if (cmd == MMC_CMD_DEV_PRESENT) {
        return MmcCmdDevPresent(cntlr, data, reply);
    } else {
        HDF_LOGE("MmcDispatch: cmd: %d is not support!", cmd);
        return HDF_ERR_NOT_SUPPORT;
    }
}

static int32_t EmmcCmdGetCid(struct MmcCntlr *cntlr, struct HdfSBuf *reply)
{
    int32_t ret;
    uint8_t cid[EMMC_CID_LEN] = {0};

    if (reply == NULL || cntlr == NULL) {
        HDF_LOGE("EmmcCmdGetCid: reply or cntlr is null!");
        return HDF_ERR_INVALID_PARAM;
    }

    ret = EmmcDeviceGetCid((struct EmmcDevice *)cntlr->curDev, cid, EMMC_CID_LEN);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("EmmcCmdGetCid: get cid fail, ret: %d!", ret);
        return ret;
    }

    if (HdfSbufWriteBuffer(reply, cid, EMMC_CID_LEN) == false) {
        HDF_LOGE("EmmcCmdGetCid: write back cid fail!");
        return HDF_ERR_IO;
    }

    return HDF_SUCCESS;
}

static int32_t EmmcDispatch(struct MmcCntlr *cntlr, int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)data;

    if (cmd == EMMC_CMD_GET_CID) {
        return EmmcCmdGetCid(cntlr, reply);
    } else {
        HDF_LOGE("EmmcDispatch: cmd: %d is not support!", cmd);
        return HDF_ERR_NOT_SUPPORT;
    }
}

static int32_t SdioDispatch(struct MmcCntlr *cntlr, int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)cntlr;
    (void)cmd;
    (void)data;
    (void)reply;
    return HDF_ERR_NOT_SUPPORT;
}

int32_t MmcIoDispatch(struct HdfDeviceIoClient *client, int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ret;
    struct MmcCntlr *cntlr = NULL;

    if (client == NULL || client->device == NULL) {
        HDF_LOGE("MmcIoDispatch: client or hdf dev obj is null!");
        return HDF_ERR_INVALID_OBJECT;
    }

    cntlr = (struct MmcCntlr *)client->device->service;
    if (cntlr == NULL) {
        HDF_LOGE("MmcIoDispatch: service is null!");
        return HDF_ERR_INVALID_OBJECT;
    }

    if (cmd < MMC_CMD_MAX) {
        ret = MmcDispatch(cntlr, cmd, data, reply);
    }  else if (cmd < EMMC_CMD_MAX) {
        ret = EmmcDispatch(cntlr, cmd, data, reply);
    }  else if (cmd < SDIO_CMD_MAX) {
        ret = SdioDispatch(cntlr, cmd, data, reply);
    } else {
        ret = HDF_ERR_NOT_SUPPORT;
    }

    return ret;
}
