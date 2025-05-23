/*
 * uart_adapter.c
 *
 * linux uart driver adapter.
 *
 * Copyright (c) 2020-2023 Huawei Device Co., Ltd.
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

#include <linux/termios.h>
#include <asm/ioctls.h>
#include <linux/serial.h>
#include <linux/fs.h>
#include <linux/version.h>
#include "device_resource_if.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_io.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "securec.h"
#include "uart_if.h"
#include "uart_core.h"

#define HDF_LOG_TAG hdf_uart_adapter
#define UART_NAME_LEN 20
#define UART_PATHNAME_LEN (UART_NAME_LEN + 20)

static char g_driverName[UART_NAME_LEN];

static int32_t UartAdapterInit(struct UartHost *host)
{
    char name[UART_PATHNAME_LEN] = {0};
    struct file *fp = NULL;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    mm_segment_t oldfs;
#endif

    if (host == NULL) {
        HDF_LOGE("UartAdapterInit: host is null!");
        return HDF_ERR_INVALID_OBJECT;
    }
    if (sprintf_s(name, UART_PATHNAME_LEN - 1, "/dev/%s%d", g_driverName, host->num) < 0) {
        return HDF_FAILURE;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    oldfs = get_fs();
    set_fs(KERNEL_DS);
#endif
    fp = filp_open(name, O_RDWR | O_NOCTTY | O_NDELAY, 0600); /* 0600 : file mode */
    if (IS_ERR(fp)) {
        HDF_LOGE("UartAdapterInit: filp_open %s fail!", name);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
        set_fs(oldfs);
#endif
        return HDF_FAILURE;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    set_fs(oldfs);
#endif
    host->priv = fp;
    return HDF_SUCCESS;
}

static int32_t UartAdapterDeInit(struct UartHost *host)
{
    struct file *fp = NULL;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    mm_segment_t oldfs;
#endif

    if (host == NULL) {
        HDF_LOGE("UartAdapterDeInit: host is null!");
        return HDF_ERR_INVALID_OBJECT;
    }

    fp = (struct file *)host->priv;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    oldfs = get_fs();
    set_fs(KERNEL_DS);
#endif
    if (!IS_ERR(fp) && fp) {
        filp_close(fp, NULL);
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    set_fs(oldfs);
#endif
    host->priv = NULL;
    return HDF_SUCCESS;
}

static int32_t UartAdapterRead(struct UartHost *host, uint8_t *data, uint32_t size)
{
    loff_t pos = 0;
    int ret;
    struct file *fp = NULL;
    char *p = (char *)data;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    mm_segment_t oldfs;
#endif
    uint32_t tmp = 0;

    if (host == NULL || host->priv == NULL || data == NULL || size == 0) {
        HDF_LOGE("UartAdapterRead: invalid parameters!");
        return HDF_ERR_INVALID_OBJECT;
    }

    fp = (struct file *)host->priv;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    oldfs = get_fs();
    set_fs(KERNEL_DS);
#endif
    while (size >= tmp) {
        ret = vfs_read(fp, p + tmp, 1, &pos);
        if (ret < 0) {
            HDF_LOGE("UartAdapterRead: vfs_read fail ret: %d!", ret);
            break;
        }
        tmp++;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    set_fs(oldfs);
#endif
    return tmp;
}

static int32_t UartAdapterWrite(struct UartHost *host, uint8_t *data, uint32_t size)
{
    loff_t pos = 0;
    int ret;
    struct file *fp = NULL;
    char *p = (char *)data;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    mm_segment_t oldfs;
#endif

    if (host == NULL || host->priv == NULL || data == NULL || size == 0) {
        HDF_LOGE("UartAdapterWrite: invalid parameters!");
        return HDF_ERR_INVALID_OBJECT;
    }

    fp = (struct file *)host->priv;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    oldfs = get_fs();
    set_fs(KERNEL_DS);
#endif
    ret = vfs_write(fp, p, size, &pos);
    if (ret < 0) {
        HDF_LOGE("UartAdapterWrite: vfs_write fail, ret: %d!", ret);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
        set_fs(oldfs);
#endif
        return HDF_FAILURE;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    set_fs(oldfs);
#endif
    return HDF_SUCCESS;
}

static int UartAdapterIoctlInner(struct file *fp, unsigned cmd, unsigned long arg)
{
    int ret = HDF_FAILURE;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    mm_segment_t oldfs;
#endif

    if (fp == NULL) {
        HDF_LOGE("UartAdapterIoctlInner: fp is null!");
        return HDF_FAILURE;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    oldfs = get_fs();
    set_fs(KERNEL_DS);
#endif
    if (fp->f_op->unlocked_ioctl != NULL) {
        ret = fp->f_op->unlocked_ioctl(fp, cmd, arg);
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    set_fs(oldfs);
#endif
    return ret;
}

static uint32_t CflagToBaudRate(unsigned short flag)
{
    uint32_t baud;

    switch ((flag | CBAUD)) {
        case B1800:
            baud = 1800;        /* 1800 : baudrate */
            break;
        case B2400:
            baud = 2400;        /* 2400 : baudrate */
            break;
        case B4800:
            baud = 4800;        /* 4800 : baudrate */
            break;
        case B9600:
            baud = 9600;        /* 9600 : baudrate */
            break;
        case B19200:
            baud = 19200;        /* 19200 : baudrate */
            break;
        case B38400:
            baud = 38400;        /* 38400 : baudrate */
            break;
        case B57600:
            baud = 57600;        /* 57600 : baudrate */
            break;
        case B115200:
            baud = 115200;       /* 115200 : baudrate */
            break;
        case B230400:
            baud = 230400;       /* 230400: baudrate */
            break;
        case B460800:
            baud = 460800;       /* 460800: baudrate */
            break;
        case B500000:
            baud = 500000;       /* 500000: baudrate */
            break;
        case B576000:
            baud = 576000;       /* 576000: baudrate */
            break;
        case B921600:
            baud = 921600;       /* 921600: baudrate */
            break;
        default:
            baud = 9600;        /* 9600 : baudrate on default */
            break;
    }
    return baud;
}

static int32_t UartAdapterGetBaud(struct UartHost *host, uint32_t *baudRate)
{
    struct termios termios;
    struct file *fp = NULL;

    if (host == NULL) {
        HDF_LOGE("UartAdapterGetBaud: host is null!");
        return HDF_ERR_INVALID_OBJECT;
    }
    fp = (struct file *)host->priv;
    if (baudRate == NULL) {
        HDF_LOGE("UartAdapterGetBaud: baudRate is null!");
        return HDF_ERR_INVALID_PARAM;
    }
    if (UartAdapterIoctlInner(fp, TCGETS, (unsigned long)&termios) < 0) {
        HDF_LOGE("UartAdapterGetBaud: tcgets fail!");
        return HDF_FAILURE;
    }
    *baudRate = CflagToBaudRate(termios.c_cflag);
    return HDF_SUCCESS;
}

static unsigned short BaudRateToCflag(uint32_t baudRate)
{
    unsigned short ret;
    switch (baudRate) {
        case 1800:    /* 1800 : baudrate */
            ret = B1800;
            break;
        case 2400:    /* 2400 : baudrate */
            ret = B2400;
            break;
        case 4800:    /* 4800 : baudrate */
            ret = B4800;
            break;
        case 9600:    /* 9600 : baudrate */
            ret = B9600;
            break;
        case 19200:   /* 19200 : baudrate */
            ret = B19200;
            break;
        case 38400:   /* 38400 : baudrate */
            ret = B38400;
            break;
        case 57600:   /* 57600 : baudrate */
            ret = B57600;
            break;
        case 115200:  /* 115200 : baudrate */
            ret = B115200;
            break;
        case 230400:  /* 230400 : baudrate */
            ret = B230400;
            break;
        case 460800:  /* 460800 : baudrate */
            ret = B460800;
            break;
        case 500000:  /* 500000 : baudrate */
            ret = B500000;
            break;
        case 576000:  /* 576000 : baudrate */
            ret = B576000;
            break;
        case 921600:  /* 921600 : baudrate */
            ret = B921600;
            break;
        default:
            ret = B9600;
            break;
    }
    return ret;
}

static int32_t UartAdapterSetBaud(struct UartHost *host, uint32_t baudRate)
{
    struct termios termios;
    struct serial_struct serial;
    struct file *fp = NULL;
    int ret;

    if (host == NULL) {
        HDF_LOGE("UartAdapterSetBaud: host is null!");
        return HDF_ERR_INVALID_OBJECT;
    }
    fp = (struct file *)host->priv;

    if (UartAdapterIoctlInner(fp, TCGETS, (unsigned long)&termios) < 0) {
        HDF_LOGE("UartAdapterSetBaud: tcgets fail!");
        return HDF_FAILURE;
    }
    termios.c_cflag &= ~CBAUD;
    termios.c_cflag |= BaudRateToCflag(baudRate);
    termios.c_cc[VMIN] = 0;
    termios.c_cc[VTIME] = 0;
    if (UartAdapterIoctlInner(fp, TCSETS, (unsigned long)&termios) < 0) {
        HDF_LOGE("UartAdapterSetBaud: tcgets fail, line: %d!", __LINE__);
        return HDF_FAILURE;
    }
    /* Set low latency */
    if (UartAdapterIoctlInner(fp, TIOCGSERIAL, (unsigned long)&serial) < 0) {
        HDF_LOGE("UartAdapterSetBaud: tiocgserial fail, line: %d!", __LINE__);
        return HDF_FAILURE;
    }
    serial.flags |= ASYNC_LOW_LATENCY;
    ret = UartAdapterIoctlInner(fp, TIOCSSERIAL, (unsigned long)&serial);
    if (ret < 0) {
        HDF_LOGE("UartAdapterSetBaud: tiocgserial fail, ret :%d, line: %d!", ret, __LINE__);
    }
    return ret;
}

static unsigned char CSToAttr(unsigned short cs)
{
    unsigned short t = cs & ~CSIZE;
    if (t == CS7) {
        return UART_ATTR_DATABIT_7;
    } else if (t == CS8) {
        return UART_ATTR_DATABIT_8;
    } else if (t == CS6) {
        return UART_ATTR_DATABIT_6;
    } else if (t == CS5) {
        return UART_ATTR_DATABIT_5;
    } else {
        /* default value */
        return UART_ATTR_DATABIT_8;
    }
}

static unsigned short AttrToCs(unsigned char attr)
{
    if (attr == UART_ATTR_DATABIT_7) {
        return CS7;
    } else if (attr == UART_ATTR_DATABIT_8) {
        return CS8;
    } else if (attr == UART_ATTR_DATABIT_6) {
        return CS6;
    } else if (attr == UART_ATTR_DATABIT_5) {
        return CS5;
    } else {
        /* default value */
        return CS8;
    }
}

static unsigned char PariTyToAttr(unsigned short ps)
{
    if ((ps & (PARENB | PARODD)) != 0) {
        return UART_ATTR_PARITY_ODD;
    } else if (!(ps & PARODD) && (ps & PARENB)) {
        return UART_ATTR_PARITY_EVEN;
    } else {
        return UART_ATTR_PARITY_NONE;
    }
}

static unsigned char StopBitToAttr(unsigned short st)
{
    if (!(st & CSTOPB)) {
        return UART_ATTR_STOPBIT_1;
    } else {
        return UART_ATTR_STOPBIT_2;
    }
}

static unsigned char CtsRtsToAttr(unsigned short cr)
{
    if ((cr & CRTSCTS) != 0) {
        return UART_ATTR_RTS_EN;
    }
    return UART_ATTR_RTS_DIS;
}

static int32_t UartAdapterGetAttribute(struct UartHost *host, struct UartAttribute *attribute)
{
    struct termios termios;
    struct file *fp = NULL;
    int ret;

    if (host == NULL) {
        HDF_LOGE("UartAdapterGetAttribute: host is null!");
        return HDF_ERR_INVALID_OBJECT;
    }
    fp = (struct file *)host->priv;
    if (attribute == NULL) {
        HDF_LOGE("UartAdapterGetAttribute: attribute is null!");
        return HDF_ERR_INVALID_PARAM;
    }
    ret = UartAdapterIoctlInner(fp, TCGETS, (unsigned long)&termios);
    if (ret < 0) {
        HDF_LOGE("UartAdapterGetAttribute: tcgets fail!");
        return HDF_FAILURE;
    }
    attribute->dataBits = CSToAttr(termios.c_cflag);
    attribute->parity = PariTyToAttr(termios.c_cflag);
    attribute->stopBits = StopBitToAttr(termios.c_cflag);
    attribute->cts = CtsRtsToAttr(termios.c_cflag);
    attribute->rts = CtsRtsToAttr(termios.c_cflag);
    return HDF_SUCCESS;
}

static int32_t UartAdapterSetAttribute(struct UartHost *host, struct UartAttribute *attribute)
{
    struct termios termios;
    struct file *fp = NULL;
    int ret;

    if (host == NULL) {
        HDF_LOGE("UartAdapterSetAttribute: host is null!");
        return HDF_ERR_INVALID_OBJECT;
    }
    fp = (struct file *)host->priv;
    if (attribute == NULL) {
        HDF_LOGE("UartAdapterSetAttribute: attribute is null!");
        return HDF_ERR_INVALID_PARAM;
    }
    ret = UartAdapterIoctlInner(fp, TCGETS, (unsigned long)&termios);
    if (ret < 0) {
        HDF_LOGE("UartAdapterSetAttribute: tcgets fail!");
        return HDF_FAILURE;
    }
    termios.c_cflag |= CLOCAL | CREAD;
    termios.c_cflag &= ~CSIZE;
    termios.c_cflag |= AttrToCs(attribute->dataBits);
    if (attribute->cts || attribute->rts) {
        termios.c_cflag |= CRTSCTS;
    } else {
        termios.c_cflag &= ~CRTSCTS;
    }
    if (attribute->parity == UART_ATTR_PARITY_ODD) {
        termios.c_cflag |= (PARODD | PARENB);
    } else if (attribute->parity == UART_ATTR_PARITY_EVEN) {
        termios.c_cflag |= PARENB;
        termios.c_cflag &= ~PARODD;
    } else {
        termios.c_cflag &= ~(PARENB | PARODD);
    }
    if (attribute->stopBits == UART_ATTR_STOPBIT_2) {
        termios.c_cflag |= CSTOPB;
    } else {
        termios.c_cflag &= ~CSTOPB;
    }
    ret = UartAdapterIoctlInner(fp, TCSETS, (unsigned long)&termios);
    return ret;
}

static int32_t UartAdapterSetTransMode(struct UartHost *host, enum UartTransMode mode)
{
    (void)host;
    (void)mode;
    return HDF_SUCCESS;
}

static struct UartHostMethod g_uartHostMethod = {
    .Init = UartAdapterInit,
    .Deinit = UartAdapterDeInit,
    .Read = UartAdapterRead,
    .Write = UartAdapterWrite,
    .SetBaud = UartAdapterSetBaud,
    .GetBaud = UartAdapterGetBaud,
    .SetAttribute = UartAdapterSetAttribute,
    .GetAttribute = UartAdapterGetAttribute,
    .SetTransMode = UartAdapterSetTransMode,
};

static int32_t HdfUartBind(struct HdfDeviceObject *obj)
{
    HDF_LOGI("HdfUartBind: entry!");
    if (obj == NULL) {
        HDF_LOGE("HdfUartBind: device is null!");
        return HDF_ERR_INVALID_OBJECT;
    }
    return (UartHostCreate(obj) == NULL) ? HDF_FAILURE : HDF_SUCCESS;
}

static int32_t HdfUartInit(struct HdfDeviceObject *obj)
{
    int32_t ret;
    struct DeviceResourceIface *iface = NULL;
    struct UartHost *host = NULL;
    const char *drName = NULL;

    HDF_LOGI("HdfUartInit: entry!");
    if (obj == NULL) {
        HDF_LOGE("HdfUartInit: device is null!");
        return HDF_ERR_INVALID_OBJECT;
    }
    host = UartHostFromDevice(obj);
    if (host == NULL) {
        HDF_LOGE("HdfUartInit: host is null!");
        return HDF_FAILURE;
    }
    iface = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (iface == NULL || iface->GetUint32 == NULL) {
        HDF_LOGE("HdfUartInit: face is invalid!");
        return HDF_FAILURE;
    }

    if (iface->GetUint32(obj->property, "num", &host->num, 0) != HDF_SUCCESS) {
        HDF_LOGE("HdfUartInit: read num fail!");
        return HDF_FAILURE;
    }
    if (iface->GetString(obj->property, "driver_name", &drName, "ttyAMA") != HDF_SUCCESS) {
        HDF_LOGE("HdfUartInit: read driver_name fail!");
        return HDF_FAILURE;
    }
    g_driverName[UART_NAME_LEN - 1] = 0;
    if (strlen(drName) > (UART_NAME_LEN - 1)) {
        HDF_LOGE("HdfUartInit: illegal length of drName!");
        return HDF_FAILURE;
    }
    ret = memcpy_s(g_driverName, UART_NAME_LEN, drName, strlen(drName));
    if (ret != EOK) {
        HDF_LOGE("HdfUartInit: memcpy_s fail!");
        return HDF_FAILURE;
    }
    host->method = &g_uartHostMethod;
    return HDF_SUCCESS;
}

static void HdfUartRelease(struct HdfDeviceObject *obj)
{
    struct UartHost *host = NULL;

    HDF_LOGI("HdfUartRelease: entry!");
    if (obj == NULL) {
        HDF_LOGE("HdfUartRelease: obj is null!");
        return;
    }
    host = UartHostFromDevice(obj);
    UartHostDestroy(host);
}

struct HdfDriverEntry g_hdfUartchdog = {
    .moduleVersion = 1,
    .moduleName = "HDF_PLATFORM_UART",
    .Bind = HdfUartBind,
    .Init = HdfUartInit,
    .Release = HdfUartRelease,
};

HDF_INIT(g_hdfUartchdog);
