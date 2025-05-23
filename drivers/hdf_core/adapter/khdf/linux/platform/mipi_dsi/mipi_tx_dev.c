/*
 * mipi_tx_dev.c
 *
 * create vfs node for mipi
 *
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "mipi_tx_dev.h"
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>
#include <linux/seq_file.h>
#include <linux/version.h>
#include "hdf_base.h"
#include "hdf_log.h"
#include "mipi_dsi_core.h"
#include "mipi_tx_reg.h"
#include "osal_io.h"
#include "osal_mem.h"
#include "osal_uaccess.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/****************************************************************************
 * macro definition                                                         *
 ****************************************************************************/
#define HDF_LOG_TAG                mipi_tx_dev
#define MIPI_TX_DEV_NAME           "hi_mipi_tx"
#define MIPI_TX_PROC_NAME          "mipi_tx"
#define NAME_LEN                   20

struct MipiDsiVfsPara {
    struct MipiDsiCntlr *cntlr;
    struct miscdevice *miscdev;
    struct semaphore sem;
    void *priv;
};

static struct MipiDsiVfsPara g_vfsPara[MAX_CNTLR_CNT];
static uint8_t g_curId = 0;
static int32_t RegisterDevice(const char *name, uint8_t id, unsigned short mode, struct file_operations *ops)
{
    int32_t error;
    struct miscdevice *dev = NULL;

    if ((name == NULL) || (ops == NULL) || (id >= MAX_CNTLR_CNT)) {
        HDF_LOGE("RegisterDevice: name, ops or id is error!");
        return HDF_ERR_INVALID_PARAM;
    }
    dev = OsalMemCalloc(sizeof(struct miscdevice));
    if (dev == NULL) {
        HDF_LOGE("RegisterDevice: [OsalMemCalloc] fail!");
        return HDF_ERR_MALLOC_FAIL;
    }
    dev->fops = ops;
    dev->name = OsalMemCalloc(NAME_LEN + 1);
    if (dev->name == NULL) {
        OsalMemFree(dev);
        HDF_LOGE("RegisterDevice: [OsalMemCalloc] fail!");
        return HDF_ERR_MALLOC_FAIL;
    }
    if (id != 0) { /* 0 : id */
        if (snprintf_s((char *)dev->name, NAME_LEN + 1, NAME_LEN, "%s%u", name, id) < 0) {
            OsalMemFree((char *)dev->name);
            OsalMemFree(dev);
            HDF_LOGE("RegisterDevice: [snprintf_s] fail!");
            return HDF_FAILURE;
        }
    } else {
        if (memcpy_s((char *)dev->name, NAME_LEN, name, strlen(name)) != EOK) {
            OsalMemFree((char *)dev->name);
            OsalMemFree(dev);
            HDF_LOGE("RegisterDevice: [memcpy_s] fail!");
            return HDF_ERR_IO;
        }
    }
    ops->owner = THIS_MODULE;
    dev->minor = MISC_DYNAMIC_MINOR;
    dev->mode = mode;
    error = misc_register(dev);
    if (error < 0) {
        printk("RegisterDevice: id %u cannot register miscdev on minor=%d (err=%d)",
            id, MISC_DYNAMIC_MINOR, error);
        OsalMemFree((char *)dev->name);
        OsalMemFree(dev);
        return error;
    }

    g_vfsPara[id].miscdev = dev;
    g_curId = id;
    printk("RegisterDevice: create inode ok %s %d", dev->name, dev->minor);
    HDF_LOGI("RegisterDevice: success!");

    return HDF_SUCCESS;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static int32_t ProcRegister(const char *name, uint8_t id, unsigned short mode, const struct proc_ops *ops)
#else
static int32_t ProcRegister(const char *name, uint8_t id, unsigned short mode, const struct file_operations *ops)
#endif
{
    char procName[NAME_LEN + 1];
    struct proc_dir_entry* entry = NULL;
    int32_t ret;

    if ((name == NULL) || (ops == NULL) || (id >= MAX_CNTLR_CNT)) {
        HDF_LOGE("ProcRegister: name, ops or id is error!");
        return HDF_ERR_INVALID_PARAM;
    }
    if (memset_s(procName, NAME_LEN + 1, 0, NAME_LEN + 1) != EOK) {
        HDF_LOGE("ProcRegister: [memcpy_s] fail!");
        return HDF_ERR_IO;
    }
    if (id != 0) {
        ret = snprintf_s(procName, NAME_LEN + 1, NAME_LEN, "%s%u", name, id);
    } else {
        ret = snprintf_s(procName, NAME_LEN + 1, NAME_LEN, "%s", name);
    }
    if (ret < 0) {
        printk(KERN_ERR "ProcRegister: procName %s snprintf_s fail!", procName);
        return HDF_FAILURE;
    }
    entry = proc_create(procName, mode, NULL, ops);
    if (entry == NULL) {
        printk(KERN_ERR "ProcRegister: proc_create name %s fail!", procName);
        return HDF_FAILURE;
    }
    HDF_LOGI("ProcRegister: success!");
    return HDF_SUCCESS;
}

static void UnregisterDevice(uint8_t id)
{
    struct miscdevice *dev = NULL;

    if (id >= MAX_CNTLR_CNT) {
        HDF_LOGE("UnregisterDevice: id error!");
        return;
    }
    dev = g_vfsPara[id].miscdev;
    if (dev == NULL) {
        HDF_LOGE("UnregisterDevice: dev is null!");
        return;
    }

    misc_deregister(dev);
    OsalMemFree((void *)dev->name);
    dev->name = NULL;
    OsalMemFree(dev);
    dev = NULL;
    g_curId = 0;
    HDF_LOGI("UnregisterDevice: success!");
}

static void ProcUnregister(const char *name, uint8_t id)
{
    char procName[NAME_LEN + 1];
    int32_t ret;

    if (id >= MAX_CNTLR_CNT) {
        HDF_LOGE("ProcUnregister: id error!");
        return;
    }
    if (memset_s(procName, NAME_LEN + 1, 0, NAME_LEN + 1) != EOK) {
        HDF_LOGE("ProcUnregister: [memcpy_s] fail!");
        return;
    }
    if (id != 0) {
        ret = snprintf_s(procName, NAME_LEN + 1, NAME_LEN, "%s%hhu", name, id);
    } else {
        ret = snprintf_s(procName, NAME_LEN + 1, NAME_LEN, "%s", name);
    }
    if (ret < 0) {
        printk(KERN_ERR "ProcUnregister: procName format fail!");
        return;
    }
    remove_proc_entry(procName, NULL);
    HDF_LOGI("ProcUnregister: success!");
}

static int32_t SemaInit(struct semaphore *sem, uint16_t val)
{
    if (sem == NULL) {
        HDF_LOGE("SemaInit: sem is null!");
        return HDF_ERR_INVALID_OBJECT;
    }
    sema_init(sem, val);
    return HDF_SUCCESS;
}

static void SemaDestroy(struct semaphore *sem)
{
    // don't support sema_destory(sem)!
    (void)sem;
}

static int32_t SemaDownInterruptable(struct semaphore *sem)
{
    return down_interruptible(sem);
}

static void SemaUp(struct semaphore *sem)
{
    up(sem);
}

static uint8_t GetId()
{
    if (g_curId >= MAX_CNTLR_CNT) {
        HDF_LOGE("GetId: fail g_curId = %hhu!", g_curId);
        return 0;
    }

    HDF_LOGI("GetId: success!");
    return g_curId;
}

static uint8_t GetIdFromFilep(struct file *filep)
{
    uint8_t id;
    if (filep == NULL) {
        HDF_LOGE("GetIdFromFilep: filep is invalid!");
        return 0;
    }

    if (filep->private_data == NULL) {
        HDF_LOGE("GetIdFromFilep: private_data is null!");
        return 0;
    }

    id = (uint8_t)(filep->private_data);
    if (id >= MAX_CNTLR_CNT) {
        HDF_LOGE("GetIdFromFilep: id error!");
        return 0;
    }

    return id;
}

static struct MipiDsiCntlr *GetCntlrFromFilep(struct file *filep)
{
    uint8_t id;
    if (filep == NULL) {
        HDF_LOGE("GetCntlrFromFilep: filep is invalid!");
        return NULL;
    }
    id = GetId();

    return g_vfsPara[id].cntlr;
}

static struct semaphore *GetSemaFromFilep(struct file *filep)
{
    uint8_t id;
    if (filep == NULL) {
        HDF_LOGE("GetSemaFromFilep: filep is invalid!");
        return NULL;
    }
    id = GetId();

    return &g_vfsPara[id].sem;
}

static struct MipiCfg *GetCfgFromFilep(struct file *filep)
{
    uint8_t id;
    if (filep == NULL) {
        HDF_LOGE("GetCfgFromFilep: filep is invalid!");
        return NULL;
    }
    id = GetId();
    if (g_vfsPara[id].cntlr == NULL) {
        HDF_LOGE("GetCfgFromFilep: g_vfsPara[id].cntlr is null!");
        return NULL;
    }

    return &(g_vfsPara[id].cntlr->cfg);
}

static int32_t MipiDsiDevSetCfg(struct MipiDsiCntlr *cntlr, struct MipiCfg *arg)
{
    int32_t ret;
    struct MipiCfg *temp = NULL;
    uint32_t size;

    if (arg == NULL) {
        HDF_LOGE("MipiDsiDevSetCfg: arg is invalid!");
        return HDF_ERR_INVALID_PARAM;
    }

    if (cntlr == NULL) {
        HDF_LOGE("MipiDsiDevSetCfg: cntlr is null!");
        return HDF_ERR_INVALID_PARAM;
    }

    size = sizeof(struct MipiCfg);
    temp = (struct MipiCfg *)OsalMemCalloc(size);
    if (temp == NULL) {
        HDF_LOGE("MipiDsiDevSetCfg: OsalMemCalloc error!");
        return HDF_ERR_MALLOC_FAIL;
    }

    if (access_ok(
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
        VERIFY_READ,
#endif
        arg, size)) { /* user space */
        if (CopyFromUser(temp, arg, size) != 0) {
            OsalMemFree(temp);
            temp = NULL;
            HDF_LOGE("MipiDsiDevSetCfg: [CopyFromUser] fail!");
            return HDF_ERR_IO;
        }
    } else {
        OsalMemFree(temp);
        temp = NULL;
        HDF_LOGE("MipiDsiDevSetCfg: illegal user space address!");
        return HDF_FAILURE;
    }

    ret = MipiDsiCntlrSetCfg(cntlr, temp);
    g_curId = cntlr->devNo;
    OsalMemFree(temp);
    HDF_LOGI("MipiDsiDevSetCfg: success!");

    return ret;
}

static int32_t MipiDsiDevSetCmd(struct MipiDsiCntlr *cntlr, struct DsiCmdDesc *arg)
{
    int32_t ret;
    struct DsiCmdDesc *temp = NULL;
    uint32_t size;

    if (arg == NULL) {
        HDF_LOGE("MipiDsiDevSetCmd: arg is invalid!");
        return HDF_ERR_INVALID_PARAM;
    }

    if (cntlr == NULL) {
        HDF_LOGE("MipiDsiDevSetCmd: cntlr is null!");
        return HDF_ERR_INVALID_PARAM;
    }

    if (!access_ok(
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
        VERIFY_WRITE,
#endif
        arg->payload, arg->dataLen)) {
        HDF_LOGE("MipiDsiDevSetCmd: payload is illegal user space address!");
        return HDF_ERR_INVALID_PARAM;
    }

    size = sizeof(struct DsiCmdDesc);
    temp = (struct DsiCmdDesc *)OsalMemCalloc(size);
    if (temp == NULL) {
        HDF_LOGE("MipiDsiDevSetCmd: [OsalMemCalloc] error!");
        return HDF_ERR_MALLOC_FAIL;
    }

    if (access_ok(
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
        VERIFY_READ,
#endif
        arg, size)) { /* user space */
        if (CopyFromUser(temp, arg, size) != 0) {
            OsalMemFree(temp);
            temp = NULL;
            HDF_LOGE("MipiDsiDevSetCmd: [CopyFromUser] fail!");
            return HDF_ERR_IO;
        }
    } else {
        OsalMemFree(temp);
        temp = NULL;
        HDF_LOGE("MipiDsiDevSetCmd: illegal user space address!");
        return HDF_FAILURE;
    }

    ret = MipiDsiCntlrTx(cntlr, temp);
    OsalMemFree(temp);
    HDF_LOGI("MipiDsiDevSetCmd: success!");

    return ret;
}

static int32_t MipiDsiDevCmdCopyFromUser(GetDsiCmdDescTag *arg, GetDsiCmdDescTag *temp, uint32_t *size)
{
    if (access_ok(
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
        VERIFY_READ,
#endif
        arg, *size)) { /* user space */
        if (CopyFromUser(temp, arg, *size) != 0) {
            HDF_LOGE("MipiDsiDevCmdCopyFromUser: [CopyFromUser] fail!");
            return HDF_ERR_IO;
        }
    } else {
        HDF_LOGE("MipiDsiDevCmdCopyFromUser: illegal user space address!");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t MipiDsiDevCmdCopyToUser(GetDsiCmdDescTag *arg, GetDsiCmdDescTag *temp, uint32_t *size)
{
    if (access_ok(
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
        VERIFY_WRITE,
#endif
        arg, *size)) { /* user space */
        if (CopyToUser(arg, temp, *size) != 0) {
            HDF_LOGE("MipiDsiDevCmdCopyToUser: [CopyToUser] fail!");
            return HDF_ERR_IO;
        }
    } else {
        HDF_LOGE("MipiDsiDevCmdCopyToUser: illegal user space address!");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t MipiDsiDevGetCmd(struct MipiDsiCntlr *cntlr, GetDsiCmdDescTag *arg)
{
    int32_t ret;
    GetDsiCmdDescTag *temp = NULL;
    uint32_t size;

    if ((cntlr == NULL) || (arg == NULL)) {
        HDF_LOGE("MipiDsiDevGetCmd: cntlr or arg is null!");
        return HDF_ERR_INVALID_PARAM;
    }

    if (!access_ok(
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
        VERIFY_WRITE,
#endif
        arg->out, arg->readLen)) {
        HDF_LOGE("MipiDsiDevGetCmd: out is illegal user space address!");
        return HDF_ERR_INVALID_PARAM;
    }

    size = sizeof(GetDsiCmdDescTag);
    temp = (GetDsiCmdDescTag *)OsalMemCalloc(size);
    if (temp == NULL) {
        HDF_LOGE("MipiDsiDevGetCmd: [OsalMemCalloc] error!");
        return HDF_ERR_MALLOC_FAIL;
    }
    if (MipiDsiDevCmdCopyFromUser(arg, temp, &size) != HDF_SUCCESS) {
        goto fail0;
    }
    ret = MipiDsiCntlrRx(cntlr, &temp->readCmd, temp->readLen, temp->out);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("MipiDsiDevGetCmd: [MipiDsiCntlrRx] fail!");
        goto fail0;
    }
    if (MipiDsiDevCmdCopyToUser(arg, temp, &size) != HDF_SUCCESS) {
        goto fail0;
    }
    OsalMemFree(temp);
    temp = NULL;
    HDF_LOGI("MipiDsiDevGetCmd: success!");
    return HDF_SUCCESS;
fail0:
    OsalMemFree(temp);
    temp = NULL;
    return HDF_FAILURE;
}

static long MipiDsiDevIoctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int32_t ret = HDF_SUCCESS;
    void *pArg = (void *)arg;
    struct MipiDsiCntlr *cntlr = NULL;
    struct semaphore *sem = NULL;

    if (filep == NULL || pArg == NULL) {
        HDF_LOGE("MipiDsiDevIoctl: filep or pArg is null!");
        return HDF_ERR_INVALID_OBJECT;
    }
    cntlr = GetCntlrFromFilep(filep);
    if (cntlr == NULL) {
        HDF_LOGE("MipiDsiDevIoctl: cntlr is null.");
        return HDF_ERR_INVALID_OBJECT;
    }

    sem = GetSemaFromFilep(filep);
    if (sem == NULL) {
        HDF_LOGE("MipiDsiDevIoctl: sem is null.");
        return HDF_ERR_INVALID_OBJECT;
    }

    (void)SemaDownInterruptable(sem);
    switch (cmd) {
        case HI_MIPI_TX_SET_DEV_CFG:
            ret = MipiDsiDevSetCfg(cntlr, (struct MipiCfg *)pArg);
            break;
        case HI_MIPI_TX_SET_CMD:
            ret = MipiDsiDevSetCmd(cntlr, (struct DsiCmdDesc *)pArg);
            break;
        case HI_MIPI_TX_GET_CMD:
            ret = MipiDsiDevGetCmd(cntlr, (GetDsiCmdDescTag *)pArg);
            break;
        case HI_MIPI_TX_ENABLE:
            MipiDsiCntlrSetHsMode(cntlr);
            HDF_LOGI("MipiDsiDevIoctl: [MipiDsiCntlrSetHsMode] done!");
            break;
        case HI_MIPI_TX_DISABLE:
            MipiDsiCntlrSetLpMode(cntlr);
            HDF_LOGI("MipiDsiDevIoctl: [MipiDsiCntlrSetLpMode] done!");
            break;
        default:
            HDF_LOGE("MipiDsiDevIoctl: [default] fail!");
            ret = -1;
            break;
    }
    SemaUp(sem);

    return ret;
}

static int MipiDsiDevOpen(struct inode *inode, struct file *filep)
{
    uint8_t id;
    (void)inode;
    (void)filep;

    id = GetId();
    g_vfsPara[id].cntlr = MipiDsiCntlrOpen(id);
    HDF_LOGI("MipiDsiDevOpen: success!");

    return 0;
}

static int MipiDsiDevRelease(struct inode *inode, struct file *filep)
{
    uint8_t id;
    (void)inode;
    (void)filep;

    id = GetId();
    if (g_vfsPara[id].cntlr != NULL) {
        MipiDsiCntlrClose(g_vfsPara[id].cntlr);
    }
    HDF_LOGI("MipiDsiDevRelease: success!");
    return 0;
}

static void MipiDsiDevProcDevShow(struct seq_file *s)
{
    struct MipiCfg *cfg = NULL;
    struct DsiTimingInfo *t = NULL;
    uint8_t id;

    id = GetId();
    if (g_vfsPara[id].cntlr == NULL) {
        HDF_LOGE("MipiDsiDevProcDevShow: g_vfsPara[id].cntlr is null!");
        return;
    }
    cfg = &(g_vfsPara[id].cntlr->cfg);
    t = &(cfg->timing);

    /* mipi tx device config */
    seq_printf(s, "MIPI_Tx DEV CONFIG\n");
    seq_printf(s, "%8s%15s%15s%15s%15s%15s\n",
        "lane", "output_mode", "phy_data_rate", "pixel_clk(KHz)",
        "video_mode", "output_fmt");
    seq_printf(s, "%8d%15d%15d%15d%15d%15d\n",
        cfg->lane,
        cfg->mode,
        cfg->phyDataRate,
        cfg->pixelClk,
        cfg->burstMode,
        cfg->format);
    seq_printf(s, "\r\n");
    /* mipi tx device sync config */
    seq_printf(s, "MIPI_Tx SYNC CONFIG\n");
    seq_printf(s, "%14s%14s%14s%14s%14s%14s%14s%14s%14s\n",
        "pkt_size", "hsa_pixels", "hbp_pixels", "hline_pixels", "vsa_lines", "vbp_lines",
        "vfp_lines", "active_lines", "edpi_cmd_size");
    seq_printf(s, "%14d%14d%14d%14d%14d%14d%14d%14d%14d\n",
        t->xPixels,
        t->hsaPixels,
        t->hbpPixels,
        t->hlinePixels,
        t->vsaLines,
        t->vbpLines,
        t->vfpLines,
        t->ylines,
        t->edpiCmdSize);
    seq_printf(s, "\r\n");
    HDF_LOGI("MipiDsiDevProcDevShow: success!");
}

static int MipiDsiDevProcShow(struct seq_file *m, void *v)
{
    seq_printf(m, "\nModule: [MIPI_TX], Build Time["__DATE__", "__TIME__"]\n");
    MipiDsiDevProcDevShow(m);
    HDF_LOGI("MipiDsiDevProcShow: success!");
    return 0;
}

static int MipiDsiDevProcOpen(struct inode *inode, struct file *file)
{
    (void)inode;
    HDF_LOGI("MipiDsiDevProcOpen: enter!");
    return single_open(file, MipiDsiDevProcShow, NULL);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static struct proc_ops g_procMipiDsiDevOps = {
    .proc_open = MipiDsiDevProcOpen,
    .proc_read = seq_read,
};
#else
static struct file_operations g_procMipiDsiDevOps = {
    .open = MipiDsiDevProcOpen,
    .read = seq_read,
};
#endif

static struct file_operations g_mipiTxfOps = {
    .open = MipiDsiDevOpen,
    .release = MipiDsiDevRelease,
    .unlocked_ioctl = MipiDsiDevIoctl,
};

int32_t MipiDsiDevModuleInit(uint8_t id)
{
    int32_t ret;

    /* 0660 : node mode */
    ret = RegisterDevice(MIPI_TX_DEV_NAME, id, 0660, (struct file_operations *)&g_mipiTxfOps);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("MipiDsiDevModuleInit: [RegisterDevice] fail, ret: %d!", ret);
        return ret;
    }
    ret = ProcRegister(MIPI_TX_PROC_NAME, id, 0440, &g_procMipiDsiDevOps); /* 0440 : proc file mode */
    if (ret != HDF_SUCCESS) {
        UnregisterDevice(id);
        HDF_LOGE("MipiDsiDevModuleInit: [ProcRegister] fail, ret: %d!", ret);
        return ret;
    }

    ret = SemaInit(&g_vfsPara[id].sem, 1);
    if (ret != HDF_SUCCESS) {
        UnregisterDevice(id);
        ProcUnregister(MIPI_TX_PROC_NAME, id);
        HDF_LOGE("MipiDsiDevModuleInit: [SemaInit] fail!");
        return ret;
    }
    HDF_LOGI("MipiDsiDevModuleInit: success!");
    return ret;
}

void MipiDsiDevModuleExit(uint8_t id)
{
    SemaDestroy(&g_vfsPara[id].sem);
    UnregisterDevice(id);
    ProcUnregister(MIPI_TX_PROC_NAME, id);

    HDF_LOGI("MipiDsiDevModuleExit: success!");
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
