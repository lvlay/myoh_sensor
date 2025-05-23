/*
 * Copyright (c) 2020-2023 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "spi_if.h"
#include "devsvc_manager_clnt.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "securec.h"
#include "spi_core.h"

#define HDF_LOG_TAG spi_if
#define HOST_NAME_LEN 32

struct SpiClient {
    struct SpiCntlr *cntlr;
    uint32_t csNum;
};

static struct SpiCntlr *SpiGetCntlrByBusNum(uint32_t num)
{
    int ret;
    char name[HOST_NAME_LEN + 1] = {0};
    struct SpiCntlr *cntlr = NULL;

    ret = snprintf_s(name, HOST_NAME_LEN + 1, HOST_NAME_LEN, "HDF_PLATFORM_SPI_%u", num);
    if (ret < 0) {
        HDF_LOGE("SpiGetCntlrByBusNum: snprintf_s fail!");
        return NULL;
    }
    cntlr = (struct SpiCntlr *)DevSvcManagerClntGetService(name);
    return cntlr;
}

int32_t SpiTransfer(DevHandle handle, struct SpiMsg *msgs, uint32_t count)
{
    struct SpiClient *client = NULL;

    if (handle == NULL) {
        HDF_LOGE("SpiTransfer: handle is null!");
        return HDF_ERR_INVALID_PARAM;
    }
    client = (struct SpiClient *)handle;
    return SpiCntlrTransfer(client->cntlr, client->csNum, msgs, count);
}

int32_t SpiRead(DevHandle handle, uint8_t *buf, uint32_t len)
{
    struct SpiMsg msg = {0};

    msg.wbuf = NULL;
    msg.rbuf = buf;
    msg.len = len;
    return SpiTransfer(handle, &msg, 1);
}

int32_t SpiWrite(DevHandle handle, uint8_t *buf, uint32_t len)
{
    struct SpiMsg msg = {0};

    msg.wbuf = buf;
    msg.rbuf = NULL;
    msg.len = len;
    return SpiTransfer(handle, &msg, 1);
}

int32_t SpiSetCfg(DevHandle handle, struct SpiCfg *cfg)
{
    struct SpiClient *client = NULL;

    if (handle == NULL) {
        HDF_LOGE("SpiSetCfg: handle is null!");
        return HDF_ERR_INVALID_OBJECT;
    }
    client = (struct SpiClient *)handle;
    return SpiCntlrSetCfg(client->cntlr, client->csNum, cfg);
}

int32_t SpiGetCfg(DevHandle handle, struct SpiCfg *cfg)
{
    struct SpiClient *client = NULL;

    if (handle == NULL) {
        HDF_LOGE("SpiGetCfg: handle is null!");
        return HDF_ERR_INVALID_OBJECT;
    }
    client = (struct SpiClient *)handle;
    return SpiCntlrGetCfg(client->cntlr, client->csNum, cfg);
}

void SpiClose(DevHandle handle)
{
    int32_t ret;
    struct SpiClient *client = NULL;

    if (handle == NULL) {
        HDF_LOGE("SpiClose: handle is null!");
        return;
    }

    client = (struct SpiClient *)handle;
    ret = SpiCntlrClose(client->cntlr, client->csNum);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("SpiClose: error, ret is %d!", ret);
    }
    OsalMemFree(handle);
}

DevHandle SpiOpen(const struct SpiDevInfo *info)
{
    int32_t ret;
    struct SpiClient *object = NULL;
    struct SpiCntlr *cntlr = NULL;

    if (info == NULL) {
        HDF_LOGE("SpiOpen: info is null!");
        return NULL;
    }
    cntlr = SpiGetCntlrByBusNum(info->busNum);
    if (cntlr == NULL) {
        HDF_LOGE("SpiOpen: cntlr is null!");
        return NULL;
    }

    object = (struct SpiClient *)OsalMemCalloc(sizeof(*object));
    if (object == NULL) {
        HDF_LOGE("SpiOpen: object malloc error!");
        return NULL;
    }

    ret = SpiCntlrOpen(cntlr, info->csNum);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("SpiOpen: spi cntlr open error, ret is %d!", ret);
        OsalMemFree(object);
        return NULL;
    }

    object->cntlr = cntlr;
    object->csNum = info->csNum;
    return (DevHandle)object;
}
