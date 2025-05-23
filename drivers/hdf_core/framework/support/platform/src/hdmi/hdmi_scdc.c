/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_log.h"
#include "hdmi_core.h"

#define HDF_LOG_TAG hdmi_scdc_c

#define HDMI_SCDC_READ_SOURCE_VERSION_TIMES 2

static int32_t HdmiScdcRead(const struct HdmiScdc *scdc, enum HdmiScdcsOffset offset, uint8_t *buffer, uint32_t len)
{
    struct HdmiDdcCfg cfg = {0};
    struct HdmiCntlr *cntlr = NULL;

    if (scdc == NULL || scdc->priv == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    cntlr = (struct HdmiCntlr *)scdc->priv;
    cfg.type = HDMI_DDC_DEV_SCDC;
    cfg.readFlag = true;
    cfg.devAddr = HDMI_DDC_SCDC_DEV_ADDRESS;
    cfg.data = buffer;
    cfg.dataLen = len;
    cfg.offset = offset;
    cfg.mode = HDMI_DDC_MODE_READ_MUTIL_NO_ACK;
    return HdmiDdcTransfer(&(cntlr->ddc), &cfg);
}

static int32_t HdmiScdcWrite(const struct HdmiScdc *scdc, enum HdmiScdcsOffset offset, uint8_t *buffer, uint32_t len)
{
    struct HdmiDdcCfg cfg = {0};
    struct HdmiCntlr *cntlr = NULL;

    if (scdc == NULL || scdc->priv == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    cntlr = (struct HdmiCntlr *)scdc->priv;
    cfg.type = HDMI_DDC_DEV_SCDC;
    cfg.readFlag = false;
    cfg.devAddr = HDMI_DDC_SCDC_DEV_ADDRESS;
    cfg.data = buffer;
    cfg.dataLen = len;
    cfg.offset = offset;
    cfg.mode = HDMI_DDC_MODE_WRITE_MUTIL_ACK;
    return HdmiDdcTransfer(&(cntlr->ddc), &cfg);
}

static int32_t HdmiScdcReadByte(const struct HdmiScdc *scdc, enum HdmiScdcsOffset offset, uint8_t *byte)
{
    return HdmiScdcRead(scdc, offset, byte, sizeof(*byte));
}

static int32_t HdmiScdcWriteByte(const struct HdmiScdc *scdc, enum HdmiScdcsOffset offset, uint8_t *byte)
{
    return HdmiScdcWrite(scdc, offset, byte, sizeof(*byte));
}

/* read opt */
static int32_t HdmiScdcReadSinkVersion(const struct HdmiScdc *scdc, uint8_t *sinkVer)
{
    return HdmiScdcReadByte(scdc, HDMI_SCDCS_OFFSET_SINK_VERSION, sinkVer);
}

static int32_t HdmiScdcReadSourceVersion(const struct HdmiScdc *scdc, uint8_t *srcVer)
{
    return HdmiScdcReadByte(scdc, HDMI_SCDCS_OFFSET_SOURCE_VERSION, srcVer);
}

static int32_t HdmiScdcReadUpdate0(const struct HdmiScdc *scdc, uint8_t *flag)
{
    return HdmiScdcReadByte(scdc, HDMI_SCDCS_OFFSET_UPDATE_0, flag);
}

static int32_t HdmiScdcReadScramblerStatus(const struct HdmiScdc *scdc, uint8_t *status)
{
    return HdmiScdcReadByte(scdc, HDMI_SCDCS_OFFSET_SCRAMBLER_STATUS, status);
}

static int32_t HdmiScdcReadTmdsConfig(const struct HdmiScdc *scdc, uint8_t *tmdsCfg)
{
    return HdmiScdcReadByte(scdc, HDMI_SCDCS_OFFSET_TMDS_CONFIG, tmdsCfg);
}

int32_t HdmiScdcReadConfig0(const struct HdmiScdc *scdc, uint8_t *cfg)
{
    return HdmiScdcReadByte(scdc, HDMI_SCDCS_OFFSET_CONFIG_0, cfg);
}

static int32_t HdmiScdcReadConfig1(const struct HdmiScdc *scdc, uint8_t *cfg)
{
    return HdmiScdcReadByte(scdc, HDMI_SCDCS_OFFSET_CONFIG_1, cfg);
}

int32_t HdmiScdcReadTestConfig0(const struct HdmiScdc *scdc, uint8_t *testCfg)
{
    return HdmiScdcReadByte(scdc, HDMI_SCDCS_OFFSET_TEST_CONFIG_0, testCfg);
}

static int32_t HdmiScdcReadTestConfig1(const struct HdmiScdc *scdc, uint8_t *testCfg)
{
    return HdmiScdcReadByte(scdc, HDMI_SCDCS_OFFSET_TEST_CONFIG_1, testCfg);
}

static int32_t HdmiScdcReadStatusFlag0(const struct HdmiScdc *scdc, uint8_t *flag)
{
    return HdmiScdcReadByte(scdc, HDMI_SCDCS_OFFSET_STASTUS_FLAG_0, flag);
}

int32_t HdmiScdcReadStatusFlag1(const struct HdmiScdc *scdc, uint8_t *flag)
{
    return HdmiScdcReadByte(scdc, HDMI_SCDCS_OFFSET_STASTUS_FLAG_1, flag);
}

int32_t HdmiScdcReadStatusFlag2(const struct HdmiScdc *scdc, uint8_t *flag)
{
    return HdmiScdcReadByte(scdc, HDMI_SCDCS_OFFSET_STASTUS_FLAG_2, flag);
}

/* write opt */
static int32_t HdmiScdcWriteSourceVersion(const struct HdmiScdc *scdc, uint8_t *srcVer)
{
    return HdmiScdcWriteByte(scdc, HDMI_SCDCS_OFFSET_SOURCE_VERSION, srcVer);
}

static int32_t HdmiScdcWriteUpdate0(const struct HdmiScdc *scdc, uint8_t *flag)
{
    return HdmiScdcWriteByte(scdc, HDMI_SCDCS_OFFSET_UPDATE_0, flag);
}

static int32_t HdmiScdcWriteTmdsConfig(const struct HdmiScdc *scdc, uint8_t *tmdsCfg)
{
    return HdmiScdcWriteByte(scdc, HDMI_SCDCS_OFFSET_TMDS_CONFIG, tmdsCfg);
}

static int32_t HdmiScdcWriteConfig0(const struct HdmiScdc *scdc, uint8_t *cfg)
{
    return HdmiScdcWriteByte(scdc, HDMI_SCDCS_OFFSET_CONFIG_0, cfg);
}

static int32_t HdmiScdcWriteConfig1(const struct HdmiScdc *scdc, uint8_t *cfg)
{
    return HdmiScdcWriteByte(scdc, HDMI_SCDCS_OFFSET_CONFIG_1, cfg);
}

static int32_t HdmiScdcWriteTestConfig0(const struct HdmiScdc *scdc, uint8_t *testCfg)
{
    return HdmiScdcWriteByte(scdc, HDMI_SCDCS_OFFSET_TEST_CONFIG_0, testCfg);
}

/* cntlr ops */
static bool HdmiCntlrScdcSourceScrambleGet(struct HdmiCntlr *cntlr)
{
    bool ret = false;

    if (cntlr == NULL || cntlr->ops == NULL || cntlr->ops->scdcSourceScrambleGet == NULL) {
        return ret;
    }
    HdmiCntlrLock(cntlr);
    ret = cntlr->ops->scdcSourceScrambleGet(cntlr);
    HdmiCntlrUnlock(cntlr);
    return ret;
}

static int32_t HdmiCntlrScdcSourceScrambleSet(struct HdmiCntlr *cntlr, bool enable)
{
    int32_t ret;

    if (cntlr == NULL || cntlr->ops == NULL || cntlr->ops->scdcSourceScrambleSet == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    HdmiCntlrLock(cntlr);
    ret = cntlr->ops->scdcSourceScrambleSet(cntlr, enable);
    HdmiCntlrUnlock(cntlr);
    return ret;
}

/* scdc opt */
int32_t HdmiScdcRrDisable(struct HdmiScdc *scdc)
{
    union HdmiScdcsConfig0 cfg = {0};
    union HdmiScdcsTestConfig0 testCfg = {0};
    int32_t ret;

    if (scdc == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    ret = HdmiScdcWriteConfig0(scdc, &(cfg.data));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("scdc write config0 fail");
        return ret;
    }
    scdc->status.cfg0 = cfg;

    ret = HdmiScdcWriteTestConfig0(scdc, &(testCfg.data));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("scdc write test config0 fail");
        return ret;
    }
    scdc->status.testCfg0 = testCfg;
    return HDF_SUCCESS;
}

int32_t HdmiScdcScrambleSet(struct HdmiScdc *scdc, struct HdmiScdcScrambleCap *scramble)
{
    union HdmiScdcsTmdsConfig cfg = {0};
    int32_t ret;

    if (scdc == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    ret = HdmiCntlrScdcSourceScrambleSet((struct HdmiCntlr *)scdc->priv, scramble->sourceScramble);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("scdc source scrambler set fail");
        return ret;
    }

    if (scramble->sinkScramble == true) {
        cfg.bits.scramblingEnable = 1;
    }
    if (scramble->tmdsBitClockRatio40 == true) {
        cfg.bits.tmdsBitClockRatio = 1;
    }
    ret = HdmiScdcWriteTmdsConfig(scdc, &(cfg.data));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("scdc write tmds config fail");
        return ret;
    }
    return HDF_SUCCESS;
}

int32_t HdmiScdcScrambleGet(struct HdmiScdc *scdc, struct HdmiScdcScrambleCap *scramble)
{
    union HdmiScdcsScramblerStatus status = {0};
    union HdmiScdcsTmdsConfig cfg = {0};
    int32_t ret;

    if (scdc == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    ret = HdmiScdcReadScramblerStatus(scdc, &(status.data));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("scdc read scrambler status fail");
        return ret;
    }
    scramble->sinkScramble = (status.bits.scramblingStatus ? true : false);

    ret = HdmiScdcReadTmdsConfig(scdc, &(cfg.data));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("scdc read tmds config fail");
        return ret;
    }
    scramble->tmdsBitClockRatio40 = (cfg.bits.tmdsBitClockRatio ? true : false);
    scramble->sourceScramble = HdmiCntlrScdcSourceScrambleGet(scdc->priv);

    scdc->status.tmdsCfg = cfg;
    scdc->status.scramblerStatus = status;
    return HDF_SUCCESS;
}

bool HdmiScdcSinkSupport(struct HdmiScdc *scdc)
{
    uint8_t srcVer;
    uint8_t i;
    int32_t ret;

    if (scdc == NULL) {
        return false;
    }

    for (i = 0; i < HDMI_SCDC_READ_SOURCE_VERSION_TIMES; i++) {
        srcVer = HDMI_SCDC_HDMI20_VERSION;
        ret = HdmiScdcWriteSourceVersion(scdc, &srcVer);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("scdc write source version fail");
            return false;
        }
        ret = HdmiScdcReadSourceVersion(scdc, &srcVer);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("scdc read source version fail");
            return false;
        }
    }
    scdc->status.sourceVersion = HDMI_SCDC_HDMI20_VERSION;
    return true;
}

int32_t HdmiScdcOptMsgHandle(const struct HdmiScdc *scdc, enum HdmiScdcOptMsg msg, uint8_t *buffer, uint32_t len)
{
    int32_t ret;

    if (scdc == NULL || buffer == NULL || len == 0) {
        return HDF_ERR_INVALID_PARAM;
    }

    switch (msg) {
        case HDMI_SCDC_OPT_SET_SOURCE_VER:
            ret = HdmiScdcWriteSourceVersion(scdc, buffer);
            break;
        case HDMI_SCDC_OPT_GET_SOURCE_VER:
            ret = HdmiScdcReadSourceVersion(scdc, buffer);
            break;
        case HDMI_SCDC_OPT_GET_SINK_VER:
            ret = HdmiScdcReadSinkVersion(scdc, buffer);
            break;
        case HDMI_SCDC_OPT_SET_FLT_UPDATE:
        case HDMI_SCDC_OPT_SET_FRL_START:
            ret = HdmiScdcWriteUpdate0(scdc, buffer);
            break;
        case HDMI_SCDC_OPT_GET_FLT_UPDATE:
        case HDMI_SCDC_OPT_GET_FRL_START:
            ret = HdmiScdcReadUpdate0(scdc, buffer);
            break;
        case HDMI_SCDC_OPT_SET_CONFIG1:
            ret = HdmiScdcWriteConfig1(scdc, buffer);
            break;
        case HDMI_SCDC_OPT_GET_CONFIG1:
            ret = HdmiScdcReadConfig1(scdc, buffer);
            break;
        case HDMI_SCDC_OPT_GET_TEST_CONFIG_1:
            ret = HdmiScdcReadTestConfig1(scdc, buffer);
            break;
        case HDMI_SCDC_OPT_GET_FLT_READY:
            ret = HdmiScdcReadStatusFlag0(scdc, buffer);
            break;
        case HDMI_SCDC_OPT_GET_LTP_REQ:
            ret = HdmiScdcRead(scdc, HDMI_SCDCS_OFFSET_STASTUS_FLAG_1, buffer, len);
            break;
        default:
            HDF_LOGE("scdc msg handle, msg %d not support.", msg);
            ret = HDF_ERR_NOT_SUPPORT;
            break;
    }

    return ret;
}

int32_t HdmiScdcFillScrambleCap(struct HdmiScdc *scdc, struct HdmiScdcScrambleCap *scramble,
    enum HdmiTmdsModeType *tmdsMode)
{
    struct HdmiCntlr *cntlr = NULL;
    struct HdmiCommonAttr *commAttr = NULL;
    struct HdmiVideoAttr *videoAttr = NULL;

    if (scdc == NULL || scdc->priv == NULL || scramble == NULL || tmdsMode == NULL) {
        HDF_LOGD("scdc fill scramble cap: param is null");
        return HDF_ERR_INVALID_PARAM;
    }

    cntlr = (struct HdmiCntlr *)scdc->priv;
    commAttr = &(cntlr->attr.commAttr);
    videoAttr = &(cntlr->attr.videoAttr);
    if (commAttr->enableHdmi == false) {
        /* DVI mode */
        *tmdsMode = HDMI_TMDS_MODE_DVI;
        scramble->sinkScramble = false;
        scramble->sourceScramble = false;
        scramble->tmdsBitClockRatio40 = false;
        if (videoAttr->tmdsClock > HDMI_HDMI14_MAX_TMDS_RATE) {
            HDF_LOGE("tmds clock %u can't support in DVI mode.", videoAttr->tmdsClock);
            return HDF_ERR_INVALID_PARAM;
        }
    } else if (videoAttr->tmdsClock > HDMI_HDMI14_MAX_TMDS_RATE) {
        *tmdsMode = HDMI_TMDS_MODE_HDMI_2_0;
        scramble->sinkScramble   = true;
        scramble->sourceScramble = true;
        scramble->tmdsBitClockRatio40 = true;
    } else {
        *tmdsMode = HDMI_TMDS_MODE_HDMI_1_4;
        scramble->sinkScramble   = false;
        scramble->sourceScramble = false;
        scramble->tmdsBitClockRatio40 = false;
    }
    return HDF_SUCCESS;
}
