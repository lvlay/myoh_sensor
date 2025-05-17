/*
 * Copyright (c) 2024 Archermind Technology (Nanjing) Co. Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef HDF_NET_USB_ADAPTER_H
#define HDF_NET_USB_ADAPTER_H

#include "osal_mutex.h"
#include "hdf_log.h"
#include "hdf_usb_net_manager.h"

#define MODULE_PARAM module_param
#define HARCH_LOG_TAG "[-net-hdf-]"
#define HARCH_NET_INFO_PRINT(fmt, ...) \
do { \
    if (0) { \
        HDF_LOGI(HARCH_LOG_TAG"[%{public}s][%{public}d]:" fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);} \
} while (0)

typedef struct pcpu_sw_netstats     PcpuSwNetstats;
typedef struct net_device           NetDevice;
typedef struct timer_list           TimerList;
typedef struct work_struct          WorkStruct;
typedef struct tasklet_struct       TaskletStruct;
typedef struct device_type          DeviceType;
typedef struct ethtool_ops          EthtoolOps;
typedef struct rndis_data_hdr       RndisDataHdr;
typedef struct sk_buff              SkBuff;
typedef struct sk_buff_head         SkBuffHead;
typedef struct net_device_ops       NetDeviceOps;

struct rndis_data_hdr {
    __le32    msg_type;        /* RNDIS_MSG_PACKET */
    __le32    msg_len;        /* rndis_data_hdr + data_len + pad */
    __le32    data_offset;        /* 36 -- right after header */
    __le32    data_len;        /* ... real packet size */

    __le32    oob_data_offset;    /* zero */
    __le32    oob_data_len;        /* zero */
    __le32    num_oob;        /* zero */
    __le32    packet_data_offset;    /* zero */

    __le32    packet_data_len;    /* zero */
    __le32    vc_handle;        /* zero */
    __le32    reserved;        /* zero */
} __attribute__ ((packed));

struct UsbnetAdapter {
    struct IDeviceIoService service;
    struct HdfDeviceObject *deviceObject;
    unsigned        canDmaSg : 1;
    NetDevice       *net;
    int32_t         msg_enable;
    struct UsbnetTransInfo usbnetInfo;
    struct OsalMutex sendSkbClock;

    unsigned char pktCnt, pktErr;

    /* various kinds of pending driver work */
    wait_queue_head_t    wait;
    TimerList           delay;

    SkBuffHead    rxq;
    SkBuffHead    txq;

    SkBuffHead    done;
    SkBuffHead    rxqPause;
    TaskletStruct  bh;

    struct pcpu_sw_netstats __percpu *stats64;
    
    WorkStruct    kevent;
    WorkStruct    TxCompleteWorkqueue;
    WorkStruct    RxCompleteWorkqueue;
    unsigned int        txLen;
    unsigned int        rxLen;
    unsigned long       flags;
#        define EVENT_TX_HALT       0
#        define EVENT_RX_HALT       1
#        define EVENT_RX_MEMORY     2
#        define EVENT_STS_SPLIT     3
#        define EVENT_LINK_RESET    4
#        define EVENT_RX_PAUSED     5
#        define EVENT_DEV_ASLEEP    6
#        define EVENT_DEV_OPEN      7
#        define EVENT_DEVICE_REPORT_IDLE    8
#        define EVENT_NO_RUNTIME_PM         9
#        define EVENT_RX_KILL               10
#        define EVENT_LINK_CHANGE           11
#        define EVENT_SET_RX_MODE           12
#        define EVENT_NO_IP_ALIGN           13
};

/* we record the state for each of our queued skbs */
enum SkbState {
    ILLEGAL = 0,
    TX_START,
    TX_DONE,
    RX_START,
    RX_DONE,
    RX_CLEANUP,
    UNLINK_START
};

struct SkbData {    /* skb->cb is one of these */
    enum SkbState       state;
    long                length;
    unsigned long       packets;
};

#endif
