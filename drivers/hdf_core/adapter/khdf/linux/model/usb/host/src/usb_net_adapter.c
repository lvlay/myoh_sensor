/*
 * Copyright (c) 2024 Archermind Technology (Nanjing) Co. Ltd.
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
#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <linux/etherdevice.h>

#include <linux/ethtool.h>
#include <linux/rtnetlink.h>
#include <linux/version.h>
#if defined(CONFIG_DRIVERS_HDF_IMX8MM_ETHERNET)
#include <linux/phy.h>
#include <linux/of_mdio.h>
#endif

#include <net/ip.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
#include <stddef.h>
#else
#include <linux/stddef.h>
#endif
#include "osal_mem.h"
#include "securec.h"

#include "hdf_device_desc.h"
#include "usb_net_adapter.h"

#define HDF_LOG_TAG         UsbnetAdapter
#define TX_TIMEOUT_JIFFIES    (5*HZ)
#define RNDIS_MSG_PACKET    0x00000001    /* 1-N packets */

// between wakeups
#define UNLINK_TIMEOUT_MS    3
#define PKT_COUNT_CLEAR     30
#define PKT_COUNT_RXKILL    20
#define SHIFT_LINE_NUM      32

/* use ethtool to change the level for any given device */
static int g_msgLevel = -1;
MODULE_PARAM(g_msgLevel, int, 0);
MODULE_PARM_DESC(g_msgLevel, "Override default message level");

static inline int UsbnetAdapterGetTxQlen(struct UsbnetAdapter *usbNet)
{
    return usbNet->usbnetInfo.txQlen;
}

static int printf_char_buffer(char *buff, int size, bool isPrint)
{
    if (isPrint) {
        int i = 0;
        printk("===-harch-=== printf_char_buffer begin\n");
        for (i = 0; i < size; i++) {
            printk(KERN_CONT"%02x ", buff[i]);
            if ((i + 1) % SHIFT_LINE_NUM == 0) {
                printk(KERN_CONT"\n");
            }
        }
        printk("===-harch-=== printf_char_buffer end\n");
    }
    return 0;
}

static int32_t UsbnetAdapterSendBufToUsb(const struct HdfDeviceObject *device, uint32_t id,
    const void *buf, uint32_t writeSize)
{
    HARCH_NET_INFO_PRINT("writeSize=%d", writeSize);
    int32_t ret = HDF_SUCCESS;
    if ((device == NULL) || (buf == NULL)) {
        HDF_LOGE("%s param is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSbufObtainDefaultSize();
    if (data == NULL) {
        HDF_LOGE("fail to obtain sbuf data");
        return HDF_FAILURE;
    }

    if (!HdfSbufWriteBuffer(data, buf, writeSize)) {
        HDF_LOGE("fail to write sbuf");
        ret = HDF_FAILURE;
        goto out;
    }

    if (HdfDeviceSendEvent(device, id, data) != HDF_SUCCESS) {
        HDF_LOGE("%s: send sensor data event failed", __func__);
        ret = HDF_FAILURE;
        goto out;
    }
    ret = HDF_SUCCESS;

out:
    HdfSbufRecycle(data);
    return ret;
}

static void RxComplete(WorkStruct *work)
{
    unsigned long flags = 0;
    struct UsbnetAdapter *usbNet = container_of(work, struct UsbnetAdapter, RxCompleteWorkqueue);
    PcpuSwNetstats *stats64 = this_cpu_ptr(usbNet->stats64);

    HARCH_NET_INFO_PRINT("rx_complete stats64->rx_packets = %lu,usbNet stats64->rx_bytes = %lu",
        stats64->rx_packets, stats64->rx_bytes);

    HARCH_NET_INFO_PRINT("rx_complete stats64->tx_packets = %lu,usbNet stats64->tx_bytes = %lu",
        stats64->tx_packets, stats64->tx_bytes);
    
    flags = u64_stats_update_begin_irqsave(&stats64->syncp);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    stats64->rx_packets++;
    stats64->rx_bytes += usbNet->rxLen;
#else
    u64_stats_inc(&stats64->rx_packets);
    u64_stats_add(&stats64->rx_bytes, usbNet->rxLen);
#endif
    u64_stats_update_end_irqrestore(&stats64->syncp, flags);

    HARCH_NET_INFO_PRINT("rx_complete stats64->rx_packets = %lu,usbNet stats64->rx_bytes = %lu",
        stats64->rx_packets, stats64->rx_bytes);

    HARCH_NET_INFO_PRINT("rx_complete stats64->tx_packets = %lu,usbNet stats64->tx_bytes = %lu",
        stats64->tx_packets, stats64->tx_bytes);
}

/* Passes this packet up the stack, updating its accounting.
 * Some link protocols batch packets, so their rx_fixup paths
 * can return clones as well as just modify the original skb.
 */
static void UsbnetAdapterSkbReturn(struct UsbnetAdapter *usbNet, SkBuff *skb)
{
    if (test_bit(EVENT_RX_PAUSED, &usbNet->flags)) {
        skb_queue_tail(&usbNet->rxqPause, skb);
        return;
    }

    /* only update if unset to allow minidriver rx_fixup override */
    if (skb->protocol == 0) {
        skb->protocol = eth_type_trans (skb, usbNet->net);
    }
    
    usbNet->rxLen = skb->len;
    schedule_work_on(0, &usbNet->RxCompleteWorkqueue);
    HARCH_NET_INFO_PRINT("< rx, len %zu, type 0x%x\n", skb->len + sizeof (struct ethhdr), skb->protocol);
    (void)memset_s(skb->cb, sizeof(struct SkbData), 0, sizeof(struct SkbData));

    if (skb_defer_rx_timestamp(skb)) {
        return;
    }
    HARCH_NET_INFO_PRINT("-------rx before netif_rx");
    printf_char_buffer(skb->data, skb->len, false);
 
    int    status = NET_RX_SUCCESS;
    status = netif_rx (skb);
    
    HARCH_NET_INFO_PRINT("netif_rx status %d\n", status);
    if (status != NET_RX_SUCCESS) {
        HARCH_NET_INFO_PRINT("netif_rx status %d\n", status);
    }
}


/*
 * DATA -- host must not write zlps
 */
static int RndisRxFixup(struct UsbnetAdapter *usbNet, SkBuff *skb)
{
    /* This check is no longer done by usbnet */
    if (skb->len < usbNet->net->hard_header_len) {
        return 0;
    }
    /* peripheral may have batched packets to us... */
    while (likely(skb->len)) {
        RndisDataHdr *hdr = (void *)skb->data;
        SkBuff *skb2 = NULL;
        u32 msg_type = 0;
        u32 msg_len  = 0;
        u32 data_len = 0;
        u32 data_offset = 0;

        msg_type = le32_to_cpu(hdr->msg_type);
        msg_len = le32_to_cpu(hdr->msg_len);
        data_offset = le32_to_cpu(hdr->data_offset);
        data_len = le32_to_cpu(hdr->data_len);
        unsigned int totalLen = data_offset + data_len + 8;
        
        /* don't choke if we see oob, per-packet data, etc */
        if (unlikely(msg_type != RNDIS_MSG_PACKET || skb->len < msg_len
                || totalLen > msg_len)) {
            usbNet->net->stats.rx_frame_errors++;
            netdev_dbg(usbNet->net, "bad rndis message %d/%d/%d/%d, len %d\n",
                le32_to_cpu(hdr->msg_type), msg_len, data_offset, data_len, skb->len);
            return 0;
        }
        unsigned int offset = 8 + data_offset;
        skb_pull(skb, offset);

        /* at most one packet left? */
        if (likely((data_len - skb->len) <= sizeof(RndisDataHdr))) {
            skb_trim(skb, data_len);
            break;
        }

        /* try to return all the packets in the batch */
        skb2 = skb_clone(skb, GFP_ATOMIC);
        if (unlikely(!skb2)) {
            break;
        }
        skb_pull(skb, msg_len - sizeof(RndisDataHdr));
        skb_trim(skb2, data_len);
        UsbnetAdapterSkbReturn(usbNet, skb2);
    }
    return 1;
}

static void UsbnetAdapterRxProcess(struct UsbnetAdapter *usbNet, SkBuff *skb)
{
    HARCH_NET_INFO_PRINT("-------rx before fix");
    printf_char_buffer(skb->data, skb->len, false);

    if (!RndisRxFixup(usbNet, skb)) {
        /* With RX_ASSEMBLE, rx_fixup() must update counters */
        if (!(usbNet->usbnetInfo.usbFlags & FLAG_RX_ASSEMBLE)) {
            usbNet->net->stats.rx_errors++;
        }
        goto done;
    }
    // else network stack removes extra byte if we forced a short packet
    HARCH_NET_INFO_PRINT("--------rx after fix");
    printf_char_buffer(skb->data, skb->len, false);

    /* all data was already cloned from skb inside the driver */
    if (usbNet->usbnetInfo.usbFlags & FLAG_MULTI_PACKET) {
        HARCH_NET_INFO_PRINT("usbNet->driver_info->flags = %d", usbNet->usbnetInfo.usbFlags);
        goto done;
    }

    if (skb->len < ETH_HLEN) {
        usbNet->net->stats.rx_errors++;
        usbNet->net->stats.rx_length_errors++;
        HARCH_NET_INFO_PRINT("rx length %d\n", skb->len);
    } else {
        HARCH_NET_INFO_PRINT("UsbnetAdapterSkbReturn");
        UsbnetAdapterSkbReturn(usbNet, skb);
        return;
    }

done:
    skb_queue_tail(&usbNet->done, skb);
}

static void UsbnetAdapterBh(TimerList *t)
{
    HARCH_NET_INFO_PRINT("begin");
    struct UsbnetAdapter *usbNet = from_timer(usbNet, t, delay);
    
    SkBuff    *skb = NULL;
    struct SkbData    *entry = NULL;

    while ((skb = skb_dequeue (&usbNet->done))) {
        entry = (struct SkbData *) skb->cb;
        HARCH_NET_INFO_PRINT("entry->state = %d", entry->state);

        switch (entry->state) {
            case RX_DONE:
                HARCH_NET_INFO_PRINT("rx_done");
                entry->state = RX_CLEANUP;
                UsbnetAdapterRxProcess (usbNet, skb);
                continue;
            case TX_DONE:
                HARCH_NET_INFO_PRINT("tx_done");
                /* fall-through */
                fallthrough;
            case RX_CLEANUP:
                HARCH_NET_INFO_PRINT("rx_cleanup");
                dev_kfree_skb (skb);
                continue;
            default:
                HARCH_NET_INFO_PRINT("bogus skb state %d\n", entry->state);
                /* fall-through */
        }
    }
    HARCH_NET_INFO_PRINT();
    /* restart RX again after disabling due to high error rate */
    clear_bit(EVENT_RX_KILL, &usbNet->flags);

    /* waiting for all pending urbs to complete?
     * only then can we forgo submitting anew
     */
    if (waitqueue_active(&usbNet->wait)) {
        HARCH_NET_INFO_PRINT("waitqueue_active");
        if (usbNet->rxq.qlen + usbNet->txq.qlen + usbNet->done.qlen == 0) {
            HARCH_NET_INFO_PRINT();
            wake_up_all(&usbNet->wait);
        }
    }

    if (usbNet->txq.qlen < UsbnetAdapterGetTxQlen(usbNet)) {
        netif_wake_queue (usbNet->net);
        HARCH_NET_INFO_PRINT("usbNet->txq.qlen = %d, TX_QLEN (usbNet) = %d",
            usbNet->txq.qlen, UsbnetAdapterGetTxQlen(usbNet));
    }
    HARCH_NET_INFO_PRINT("end");
}

static void UsbnetAdapterBhTasklet(unsigned long data)
{
    HARCH_NET_INFO_PRINT("begin");
    TimerList *t = (TimerList *)data;

    UsbnetAdapterBh(t);
    HARCH_NET_INFO_PRINT("end");
}

/* The caller must hold list->lock */
static void UsbnetAdapterQueueSkb(SkBuffHead *list,
    SkBuff *newsk, enum SkbState state)
{
    struct SkbData *entry = (struct SkbData *) newsk->cb;

    __skb_queue_tail(list, newsk);
    entry->state = state;
}

static enum SkbState UsbnetAdapterDeferBh(struct UsbnetAdapter *usbNet, SkBuff *skb,
    SkBuffHead *list, enum SkbState state)
{
    unsigned long flags = 0;
    enum SkbState old_state = ILLEGAL;
    struct SkbData *entry = (struct SkbData *) skb->cb;

    spin_lock_irqsave(&list->lock, flags);
    old_state = entry->state;
    entry->state = state;
    __skb_unlink(skb, list);

    /* UsbnetAdapterDeferBh() is never called with list == &usbNet->done.
     * spin_lock_nested() tells lockdep that it is OK to take
     * usbNet->done.lock here with list->lock held.
     */
    spin_lock_nested(&usbNet->done.lock, SINGLE_DEPTH_NESTING);

    __skb_queue_tail(&usbNet->done, skb);
    if (usbNet->done.qlen == 1) {
        tasklet_schedule(&usbNet->bh);
    }

    spin_unlock(&usbNet->done.lock);
    spin_unlock_irqrestore(&list->lock, flags);
    return old_state;
}

static int32_t UsbnetAdapterOpen(NetDevice *net)
{
    HARCH_NET_INFO_PRINT("begin");
    if (NULL == net) {
        HDF_LOGE("net device object is invalid");
        return HDF_FAILURE;
    }
    struct UsbnetAdapter *usbNet = netdev_priv(net);
    if (NULL == usbNet) {
        HDF_LOGE("usb net adapter object is invalid");
        return HDF_FAILURE;
    }

    set_bit(EVENT_DEV_OPEN, &usbNet->flags);

    netif_start_queue (net);

    /* set device param, if find usb net host need, then it should send to it */
    /* reset rx error state */
    usbNet->pktCnt = 0;
    usbNet->pktErr = 0;
    clear_bit(EVENT_RX_KILL, &usbNet->flags);

    /* send info to usb start usb function */
    /* need transform usb_net_host change about usbNet->driverInfo->flags and set it on usb_net_host */
    OsalMutexLock(&usbNet->sendSkbClock);
    int32_t ret = UsbnetAdapterSendBufToUsb(usbNet->deviceObject, USB_NET_OPEN_USB,
        (const void *)&usbNet->flags, sizeof(usbNet->flags));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("fail to UsbnetAdapterSendBufToUsb SendEvent!");
        OsalMutexUnlock(&usbNet->sendSkbClock);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&usbNet->sendSkbClock);
    
    tasklet_schedule (&usbNet->bh);
    return HDF_SUCCESS;
}

static void WaitSkbQueueEmpty(SkBuffHead *q)
{
    unsigned long flags = 0;
    HARCH_NET_INFO_PRINT();
    spin_lock_irqsave(&q->lock, flags);
    while (!skb_queue_empty(q)) {
        spin_unlock_irqrestore(&q->lock, flags);
        schedule_timeout(msecs_to_jiffies(UNLINK_TIMEOUT_MS));
        set_current_state(TASK_UNINTERRUPTIBLE);
        spin_lock_irqsave(&q->lock, flags);
    }
    spin_unlock_irqrestore(&q->lock, flags);
}

// precondition: never called in_interrupt
static void UsbnetAdapterTerminateUrbs(struct UsbnetAdapter *usbNet)
{
    HARCH_NET_INFO_PRINT();

    DECLARE_WAITQUEUE(wait, current);
    /* ensure there are no more active urbs */
    add_wait_queue(&usbNet->wait, &wait);
    set_current_state(TASK_UNINTERRUPTIBLE);
    /* maybe wait for deletions to finish. */
    WaitSkbQueueEmpty(&usbNet->rxq);
    WaitSkbQueueEmpty(&usbNet->txq);

    WaitSkbQueueEmpty(&usbNet->done);
    set_current_state(TASK_RUNNING);
    remove_wait_queue(&usbNet->wait, &wait);
}

static int32_t UsbnetAdapterStop(NetDevice *net)
{
    HARCH_NET_INFO_PRINT("begin");
    if (NULL == net) {
        HDF_LOGE("net device object is invalid");
        return HDF_FAILURE;
    }

    struct UsbnetAdapter  *usbNet = netdev_priv(net);
    if (NULL == usbNet) {
        HDF_LOGE("usb net adapter object is invalid");
        return HDF_FAILURE;
    }

    PcpuSwNetstats *stats64 = this_cpu_ptr(usbNet->stats64);

    HARCH_NET_INFO_PRINT("usbNet stats64->rx_packets = %lu,usbNet stats64->rx_bytes = %lu",
        stats64->rx_packets, stats64->rx_bytes);

    HARCH_NET_INFO_PRINT("usbNet stats64->tx_packets = %lu,usbNet stats64->tx_bytes = %lu",
        stats64->tx_packets, stats64->tx_bytes);

    /* 1. clear_bit EVENT_DEV_OPEN dev->flags */
    clear_bit(EVENT_DEV_OPEN, &usbNet->flags);
    /* 2. netif_stop_queue net */
    netif_stop_queue (net);
    HARCH_NET_INFO_PRINT("stop stats: rx/tx %lu/%lu, errs %lu/%lu\n",
        net->stats.rx_packets, net->stats.tx_packets,
        net->stats.rx_errors, net->stats.tx_errors);
    
    /* 3. pm = usb_autopm_get_interface(dev->intf); do nothing */
    /* 4. if (info->stop) { no stop interface impl in rndis driver info */
    /* 5. if (!(info->flags & FLAG_AVOID_UNLINK_URBS)) do nothing*/
    if (!(usbNet->usbnetInfo.usbFlags & FLAG_AVOID_UNLINK_URBS)) {
        UsbnetAdapterTerminateUrbs(usbNet);
    }
    
    /* 6. usbnet_status_stop none */
    /* 7. usbnet_purge_paused_rxq none */
    skb_queue_purge(&usbNet->rxqPause);
    /* 8. test_and_clear_bit and no manage_power interface impl in rndis driver info*/
    /* 9. dev flags */
    usbNet->flags = 0;
    OsalMutexLock(&usbNet->sendSkbClock);
    int32_t ret = UsbnetAdapterSendBufToUsb(usbNet->deviceObject, USB_NET_CLOSE_USB,
        (const void *)&usbNet->flags, sizeof(usbNet->flags));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("fail to UsbnetAdapterSendBufToUsb SendEvent!");
        OsalMutexUnlock(&usbNet->sendSkbClock);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&usbNet->sendSkbClock);
    /* 10. del_timer_sync  */
    del_timer_sync (&usbNet->delay);
    /* 11. tasklet_kill */
    tasklet_kill (&usbNet->bh);
    /* 12. cancel_work_sync dev->kevent */
    cancel_work_sync(&usbNet->TxCompleteWorkqueue);
    cancel_work_sync(&usbNet->RxCompleteWorkqueue);
    return HDF_SUCCESS;
}

static SkBuff *RndisTxFixup(SkBuff *skb, gfp_t flags)
{
    RndisDataHdr *hdr = NULL;
    SkBuff *skb2 = NULL;
    unsigned len = skb->len;

    if (likely(!skb_cloned(skb))) {
        int room = skb_headroom(skb);
        /* enough head room as-is? */
        if (unlikely((sizeof(RndisDataHdr)) <= room)) {
            goto fill;
        }
        /* enough room, but needs to be readjusted? */
        room += skb_tailroom(skb);
        if (likely((sizeof(RndisDataHdr)) <= room)) {
            skb->data = memmove_s(skb->head + sizeof(RndisDataHdr), len, skb->data, len);
            skb_set_tail_pointer(skb, len);
            goto fill;
        }
    }

    /* create a new skb, with the correct size (and tailpad) */
    skb2 = skb_copy_expand(skb, sizeof(RndisDataHdr), 1, flags);
    dev_kfree_skb_any(skb);
    if (unlikely(!skb2)) {
        return skb2;
    }
    skb = skb2;

    /* fill out the RNDIS header.  we won't bother trying to batch
     * packets; Linux minimizes wasted bandwidth through tx queues.
     */
fill:
    HARCH_NET_INFO_PRINT("%s:%d fill skb by rndis host", __func__, __LINE__);
    hdr = __skb_push(skb, sizeof(*hdr));
    (void)memset_s(hdr, sizeof(*hdr), 0, sizeof(*hdr));
    hdr->msg_type = cpu_to_le32(RNDIS_MSG_PACKET);
    hdr->msg_len = cpu_to_le32(skb->len);
    unsigned int offset = sizeof(*hdr) - 8;
    hdr->data_offset = cpu_to_le32(offset);
    hdr->data_len = cpu_to_le32(len);
    
    return skb;
}

static void TxComplete(WorkStruct *work)
{
    unsigned long flags = 0;
    struct UsbnetAdapter *usbNet = container_of(work, struct UsbnetAdapter, TxCompleteWorkqueue);
    PcpuSwNetstats *stats64 = this_cpu_ptr(usbNet->stats64);
    HARCH_NET_INFO_PRINT ("tx_complete stats64->rx_packets = %lu,usbNet stats64->rx_bytes = %lu",
        stats64->rx_packets, stats64->rx_bytes);

    HARCH_NET_INFO_PRINT ("tx_complete stats64->tx_packets = %lu,usbNet stats64->tx_bytes = %lu",
        stats64->tx_packets, stats64->tx_bytes);

    flags = u64_stats_update_begin_irqsave(&stats64->syncp);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
    stats64->tx_packets++;
    stats64->tx_bytes += usbNet->txLen;
#else
    u64_stats_inc(&stats64->tx_packets);
    u64_stats_add(&stats64->tx_bytes, usbNet->txLen);
#endif
    u64_stats_update_end_irqrestore(&stats64->syncp, flags);

    HARCH_NET_INFO_PRINT ("tx_complete stats64->rx_packets = %lu,usbNet stats64->rx_bytes = %lu",
        stats64->rx_packets, stats64->rx_bytes);

    HARCH_NET_INFO_PRINT ("tx_complete stats64->tx_packets = %lu,usbNet stats64->tx_bytes = %lu",
        stats64->tx_packets, stats64->tx_bytes);
}

static netdev_tx_t UsbnetAdapterStartXmit(SkBuff *skb, NetDevice *net)
{
    HARCH_NET_INFO_PRINT ("netif_running=%d, netif_device_present=%d, netif_carrier_ok=%d", netif_running(net),
        netif_device_present(net), netif_carrier_ok(net));
    HARCH_NET_INFO_PRINT ("skb->data = %x, len = %d", skb->data, skb->len);
    struct UsbnetAdapter *usbNet = netdev_priv(net);
    //1.time tamp
    if (skb) {
        skb_tx_timestamp(skb);
    }
    //2.fix up
    SkBuff* skbFixup = RndisTxFixup(skb, GFP_ATOMIC);
    if (NULL == skbFixup) {
        if (usbNet->usbnetInfo.usbFlags & FLAG_MULTI_PACKET) {
            goto not_drop;
        }
        HDF_LOGE("fail to tx fixup by rndis host");
        goto drop;
    }
    HARCH_NET_INFO_PRINT("skb_fixup->data = %x, len = %d", skbFixup->data, skbFixup->len);
    /* print org skb data info */
    printf_char_buffer(skbFixup->data, skbFixup->len, false);
    //3.send msg to usb
    unsigned long flags = 0;
    spin_lock_irqsave(&usbNet->txq.lock, flags);
    if (netif_queue_stopped(usbNet->net)) {
        goto drop;
    }

    int32_t ret = UsbnetAdapterSendBufToUsb(usbNet->deviceObject, USB_NET_SEND_DATA_TO_USB,
        (unsigned char *)(skbFixup->data), skbFixup->len);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("fail to UsbnetHost SendEvent!");
drop:
        net->stats.tx_dropped++;
not_drop:
        spin_unlock_irqrestore(&usbNet->txq.lock, flags);
        if (skbFixup) {
            dev_kfree_skb_any (skbFixup);
        }
    } else {
        netif_trans_update(usbNet->net);
        UsbnetAdapterQueueSkb(&usbNet->txq, skbFixup, TX_START);

        if (usbNet->txq.qlen >= UsbnetAdapterGetTxQlen(usbNet)) {
            HARCH_NET_INFO_PRINT("usbNet->txq.qlen = %d, TX_QLEN (usbNet) = %d",
                usbNet->txq.qlen, UsbnetAdapterGetTxQlen(usbNet));
            netif_stop_queue (usbNet->net);
        }
        spin_unlock_irqrestore(&usbNet->txq.lock, flags);
        usbNet->txLen = skbFixup->len;
        schedule_work_on(0, &usbNet->TxCompleteWorkqueue);
        
        enum SkbState state = UsbnetAdapterDeferBh(usbNet, skbFixup, &usbNet->txq, TX_DONE);
        HARCH_NET_INFO_PRINT("state= %d", state);
    }
    return NETDEV_TX_OK;
}

static void UsbnetAdapterTXTimeout(NetDevice *net, unsigned int txqueue)
{
    //send to device
    HARCH_NET_INFO_PRINT("begin");
    if (NULL == net) {
        HDF_LOGE("%s net device object is invalid", __func__);
        return;
    }
    struct UsbnetAdapter  *usbNet = netdev_priv(net);
    if (NULL == usbNet) {
        HDF_LOGE("%s UsbnetAdapter object is invalid", __func__);
        return;
    }
    /* 1. unlink_urbs (dev, &dev->txq); maybe later be used */

    /* 2. tasklet_schedule (&dev->bh) */
    tasklet_schedule (&usbNet->bh);

    /* 3. if (dev->driver_info->recover) none, rndis host no recover impl */
    
    return;
}

/* some work can't be done in tasklets, so we use keventd
 *
 * NOTE:  annoying asymmetry:  if it's active, schedule_work() fails,
 * but tasklet_schedule() doesn't.  hope the failure is rare.
 */
static void UsbnetAdapterDeferKevent(struct UsbnetAdapter *usbNet, int work)
{
    HARCH_NET_INFO_PRINT("begin");
    if (NULL == usbNet) {
        HDF_LOGE("%s UsbnetAdapter object is invalid", __func__);
        return;
    }
    set_bit (work, &usbNet->flags);
    /* need update flags to usb_net_host */
    OsalMutexLock(&usbNet->sendSkbClock);
    int32_t ret = UsbnetAdapterSendBufToUsb(usbNet->deviceObject, USB_NET_UPDATE_FLAGS,
        (const void *)&usbNet->flags, sizeof(usbNet->flags));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("fail to UsbnetAdapterSendBufToUsb SendEvent!");
        OsalMutexUnlock(&usbNet->sendSkbClock);
        return;
    }
    OsalMutexUnlock(&usbNet->sendSkbClock);
    
    if (!schedule_work (&usbNet->kevent)) {
        HARCH_NET_INFO_PRINT("kevent %d may have been dropped\n", work);
    } else {
        HARCH_NET_INFO_PRINT("kevent %d scheduled\n", work);
    }
}

static void UsbnetAdapterSetRxMode(NetDevice *net)
{
    return;
}

static void UsbnetAdapterPauseRx(struct UsbnetAdapter *usbNet)
{
    HARCH_NET_INFO_PRINT("begin");
    if (NULL == usbNet) {
        HDF_LOGE("%s UsbnetAdapter object is invalid", __func__);
        return;
    }
    set_bit(EVENT_RX_PAUSED, &usbNet->flags);
    /* need update flags to usb_net_host */
    OsalMutexLock(&usbNet->sendSkbClock);
    int32_t ret = UsbnetAdapterSendBufToUsb(usbNet->deviceObject, USB_NET_UPDATE_FLAGS,
        (const void *)&usbNet->flags, sizeof(usbNet->flags));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("fail to UsbnetAdapterSendBufToUsb SendEvent!");
        OsalMutexUnlock(&usbNet->sendSkbClock);
        return;
    }
    OsalMutexUnlock(&usbNet->sendSkbClock);

    HARCH_NET_INFO_PRINT("paused rx queue enabled\n");
}

static void UsbnetAdapterUnlinkRxUrbs(struct UsbnetAdapter *usbNet)
{
    HARCH_NET_INFO_PRINT("begin");
    if (NULL == usbNet) {
        HDF_LOGE("%s UsbnetAdapter object is invalid", __func__);
        return;
    }
    
    if (netif_running(usbNet->net)) {
        /* unlink_urbs usbNet usbNet->rxq */
        tasklet_schedule(&usbNet->bh);
    }
}

static void UsbnetAdapterResumeRx(struct UsbnetAdapter *usbNet)
{
    HARCH_NET_INFO_PRINT("begin");
    int num = 0;
    SkBuff *skb = NULL;
    if (NULL == usbNet) {
        HDF_LOGE("%s UsbnetAdapter object is invalid", __func__);
        return;
    }
    
    clear_bit(EVENT_RX_PAUSED, &usbNet->flags);
    /* need update flags to usb_net_host */
    OsalMutexLock(&usbNet->sendSkbClock);
    int32_t ret = UsbnetAdapterSendBufToUsb(usbNet->deviceObject, USB_NET_UPDATE_FLAGS,
        (const void *)&usbNet->flags, sizeof(usbNet->flags));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("fail to UsbnetAdapterSendBufToUsb SendEvent!");
        OsalMutexUnlock(&usbNet->sendSkbClock);
        return;
    }
    OsalMutexUnlock(&usbNet->sendSkbClock);

    while ((skb = skb_dequeue(&usbNet->rxqPause)) != NULL) {
        UsbnetAdapterSkbReturn(usbNet, skb);
        num++;
    }

    tasklet_schedule(&usbNet->bh);
    HARCH_NET_INFO_PRINT("paused rx queue disabled, %d skbs requeued\n", num);
}


static int32_t UsbnetAdapterChangeMtu(NetDevice *net, int newMtu)
{
    //send to device
    HARCH_NET_INFO_PRINT("begin");
    if (NULL == net) {
        HDF_LOGE("%s net device object is invalid", __func__);
        return HDF_FAILURE;
    }
    
    struct UsbnetAdapter  *usbNet = netdev_priv(net);
    if (NULL == usbNet) {
        HDF_LOGE("%s UsbnetAdapter object is invalid", __func__);
        return HDF_FAILURE;
    }

    /* 1. ll_mtu old_hard_mtu old_rx_urb_size */
    HARCH_NET_INFO_PRINT("newMtu:%d", newMtu);
    int llMtu = newMtu + net->hard_header_len;
    int oldHardMtu = usbNet->usbnetInfo.hardMtu;
    int oldRxUrbSize = usbNet->usbnetInfo.rxUrbSize;

    /* 2. no second zero-length packet read wanted after mtu-sized packets */
    if ((llMtu % usbNet->usbnetInfo.maxpacket) == 0) {
        HARCH_NET_INFO_PRINT("");
        return -EDOM;
    }

    /* 3. set usbNet->hard_mtu */
    net->mtu = newMtu;
    usbNet->usbnetInfo.hardMtu = net->mtu + net->hard_header_len;
    
    /* 4. pause and resume usbnet */
    if (usbNet->usbnetInfo.rxUrbSize == oldHardMtu) {
        usbNet->usbnetInfo.rxUrbSize = usbNet->usbnetInfo.hardMtu;
        if (usbNet->usbnetInfo.rxUrbSize > oldRxUrbSize) {
            UsbnetAdapterPauseRx(usbNet);
            UsbnetAdapterUnlinkRxUrbs(usbNet);
            UsbnetAdapterResumeRx(usbNet);
        }
    }

    /* need transmit hard_mtu to usb net host, UsbnetHostUpdateMaxQlen need this param */
    HARCH_NET_INFO_PRINT("name = %s, flags = %d, usbFlags = %x mtu = %d, hardHeaderLen = %d,\
        link = %d needReset = %d, hardMtu = %d, rxUrbSize = %d, maxpacket =%d",
        usbNet->usbnetInfo.name,usbNet->usbnetInfo.flags,usbNet->usbnetInfo.usbFlags,\
        usbNet->usbnetInfo.mtu,usbNet->usbnetInfo.hardHeaderLen,usbNet->usbnetInfo.link,\
        usbNet->usbnetInfo.needReset,usbNet->usbnetInfo.hardMtu,usbNet->usbnetInfo.rxUrbSize,\
        usbNet->usbnetInfo.maxpacket);
    /* 5. max qlen depend on hard_mtu and rx_urb_size */
    /* need update flags to usb_net_host */
    OsalMutexLock(&usbNet->sendSkbClock);
    int32_t ret = UsbnetAdapterSendBufToUsb(usbNet->deviceObject, USB_NET_UPDATE_MAXQLEN,
        (const void *)&usbNet->usbnetInfo, sizeof(usbNet->usbnetInfo));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("fail to UsbnetAdapterSendBufToUsb SendEvent!");
        OsalMutexUnlock(&usbNet->sendSkbClock);
        return ret;
    }
    OsalMutexUnlock(&usbNet->sendSkbClock);
    return HDF_SUCCESS;
}

static void UsbnetAdapterGetStats64(NetDevice *net, struct rtnl_link_stats64 *stats)
{
    //send to device
    HARCH_NET_INFO_PRINT("begin");

    struct UsbnetAdapter *usbNet = netdev_priv(net);
    netdev_stats_to_stats64(stats, &net->stats);
    dev_fetch_sw_netstats(stats, usbNet->stats64);

    PcpuSwNetstats *stats64 = this_cpu_ptr(usbNet->stats64);

    HARCH_NET_INFO_PRINT ("usbNet stats64->rx_packets = %lu,usbNet stats64->rx_bytes = %lu",
        stats64->rx_packets, stats64->rx_bytes);

    HARCH_NET_INFO_PRINT ("usbNet stats64->tx_packets = %lu,usbNet stats64->tx_bytes = %lu",
        stats64->tx_packets, stats64->tx_bytes);

    HARCH_NET_INFO_PRINT("end");
    return;
}

static NetDeviceOps g_UsbnetAdapterDeviceOps = {
    .ndo_open               = UsbnetAdapterOpen,
    .ndo_stop               = UsbnetAdapterStop,
    .ndo_start_xmit         = UsbnetAdapterStartXmit,
    .ndo_tx_timeout         = UsbnetAdapterTXTimeout,
    .ndo_set_rx_mode        = UsbnetAdapterSetRxMode,
    .ndo_change_mtu         = UsbnetAdapterChangeMtu,
    .ndo_get_stats64        = UsbnetAdapterGetStats64,
    .ndo_set_mac_address    = eth_mac_addr,
    .ndo_validate_addr      = eth_validate_addr,
};

/* drivers may override default ethtool_ops in their bind() routine */
static const EthtoolOps g_UsbnetAdapterEthtoolOps = {
};

static void UsbnetAdapterLinkChange(struct UsbnetAdapter *usbNet, bool link, bool need_reset)
{
    HARCH_NET_INFO_PRINT("begin");
    /* update link after link is reseted */
    if (link && !need_reset) {
        netif_carrier_on(usbNet->net);
    } else {
        netif_carrier_off(usbNet->net);
    }
}

static int32_t UsbnetAdapterSetSkb(struct UsbnetAdapter *usbNet, gfp_t flags,
    uint32_t infoSize, unsigned char *buff, SkBuff *skb)
{
    if (!skb) {
        HARCH_NET_INFO_PRINT("no rx skb\n");
        UsbnetAdapterDeferKevent(usbNet, EVENT_RX_MEMORY);
        return HDF_DEV_ERR_NO_MEMORY;
    }

    skb_put(skb, infoSize);
    if (memcpy_s(skb->data, infoSize, buff, infoSize) != EOK) {
        HARCH_NET_INFO_PRINT("memcpy_s error ");
        return HDF_ERR_INVALID_PARAM;
    }
    
    printf_char_buffer(skb->data, skb->len, false);
    return HDF_SUCCESS;
}

static void UsbnetAdapterSetUsbNetInfo(struct UsbnetAdapter *usbNet, SkBuff *skb, enum SkbState state)
{
    unsigned long lockflags = 0;
    spin_lock_irqsave (&usbNet->rxq.lock, lockflags);
    UsbnetAdapterQueueSkb(&usbNet->rxq, skb, RX_START);
    spin_unlock_irqrestore (&usbNet->rxq.lock, lockflags);

    PcpuSwNetstats *stats64 = this_cpu_ptr(usbNet->stats64);
    HARCH_NET_INFO_PRINT("usbNet stats64->rx_packets = %lu,usbNet stats64->rx_bytes = %lu",
        stats64->rx_packets, stats64->rx_bytes);
    
    if (++usbNet->pktCnt > PKT_COUNT_CLEAR) {
        HARCH_NET_INFO_PRINT("usbNet->pktCnt = %d, pktErr = %d", usbNet->pktCnt, usbNet->pktErr);
        usbNet->pktCnt = 0;
        usbNet->pktErr = 0;
    } else {
        HARCH_NET_INFO_PRINT("usbNet->pktCnt = %d,pktErr = %d", usbNet->pktCnt, usbNet->pktErr);
        if (state == RX_CLEANUP) {
            usbNet->pktErr++;
        }

        if (usbNet->pktErr > PKT_COUNT_RXKILL) {
            set_bit(EVENT_RX_KILL, &usbNet->flags);
        }
    }
}

static int32_t UsbnetAdapterRxComplete(struct HdfDeviceObject *device, struct HdfSBuf *data, gfp_t flags)
{
    HARCH_NET_INFO_PRINT("begin");
    unsigned char *buff = NULL;
    uint32_t infoSize   = 0;
    bool flag = HdfSbufReadBuffer(data, (const void **)(&(buff)), &infoSize);
    if ((!flag) || buff == NULL) {
        HDF_LOGE("%s: fail to read infoTable in event data, flag = %d", __func__, flag);
        return HDF_ERR_INVALID_PARAM;
    }
    
    NetDevice *net = (NetDevice *)device->priv;
    if (net == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    printf_char_buffer(buff, infoSize, false);
    struct UsbnetAdapter *usbNet = netdev_priv(net);
    if (usbNet == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
   
    SkBuff *skb   = NULL;
    uint32_t skb_size = sizeof(struct iphdr) + sizeof(struct udphdr) + infoSize;
    size_t size = skb_size > usbNet->usbnetInfo.rxUrbSize ? usbNet->usbnetInfo.rxUrbSize : skb_size;
    if (test_bit(EVENT_NO_IP_ALIGN, &usbNet->flags)) {
        skb = __netdev_alloc_skb(usbNet->net, size, flags);
    } else {
        skb = __netdev_alloc_skb_ip_align(usbNet->net, size, flags);
    }
    int32_t ret = UsbnetAdapterSetSkb(usbNet, flags, infoSize, buff, skb);
    if (ret != HDF_SUCCESS || skb == NULL) {
        HDF_LOGE("%s: fail to set skb, flag = %d", __func__, flag);
        return ret;
    }
    HARCH_NET_INFO_PRINT("infoSize= %d, size = %d, skb->len = %d", infoSize, size, skb->len);
    
    struct SkbData *entry = (struct SkbData *)skb->cb;
    entry->length = 0;
    
    //success recv
    enum SkbState state = RX_DONE;
    UsbnetAdapterSetUsbNetInfo(usbNet, skb, state);
    
    state = UsbnetAdapterDeferBh(usbNet, skb, &usbNet->rxq, state);
    HARCH_NET_INFO_PRINT("usbNet->pktCnt = %d, pktErr = %d, state = %d", usbNet->pktCnt, usbNet->pktErr, state);
    return HDF_SUCCESS;
}

// precondition: never called in_interrupt
static DeviceType wlanType = {
    .name    = "wlan",
};

static DeviceType wwanType = {
    .name    = "wwan",
};

static void UsbnetAdapterSetUsbNet(struct HdfDeviceObject *device,
    NetDevice *net, struct UsbnetTransInfo *uNetTransInfo)
{
    struct UsbnetAdapter *usbNet = netdev_priv(net);
    usbNet->deviceObject = device;
    
    //info from usb
    if (memcpy_s(&(usbNet->usbnetInfo), sizeof(struct UsbnetTransInfo),
        uNetTransInfo, sizeof(struct UsbnetTransInfo)) != EOK) {
        HARCH_NET_INFO_PRINT("memcpy_s error ");
    }

    HARCH_NET_INFO_PRINT("name = %s, flags = %d, usbFlags = %x, mtu = %d, \
        hardHeaderLen = %d, link = %d, needReset = %d, \
        hardMtu = %d, rxUrbSize = %d, maxpacket = %d",
        usbNet->usbnetInfo.name, usbNet->usbnetInfo.flags,
        usbNet->usbnetInfo.usbFlags, usbNet->usbnetInfo.mtu,
        usbNet->usbnetInfo.hardHeaderLen, usbNet->usbnetInfo.link,
        usbNet->usbnetInfo.needReset, usbNet->usbnetInfo.hardMtu,
        usbNet->usbnetInfo.rxUrbSize, usbNet->usbnetInfo.maxpacket);
    
    usbNet->stats64 = netdev_alloc_pcpu_stats(PcpuSwNetstats);
    if (!usbNet->stats64) {
        return;
    }
    
    usbNet->msg_enable = netif_msg_init (g_msgLevel, NETIF_MSG_DRV
           | NETIF_MSG_PROBE | NETIF_MSG_LINK| NETIF_MSG_RX_ERR|NETIF_MSG_TX_ERR);
    
    OsalMutexInit(&usbNet->sendSkbClock);

    init_waitqueue_head(&usbNet->wait);
    skb_queue_head_init (&usbNet->rxq);
    skb_queue_head_init (&usbNet->txq);
    skb_queue_head_init (&usbNet->done);
    skb_queue_head_init(&usbNet->rxqPause);

    // init work for tx_complete rx_complete
    INIT_WORK(&usbNet->TxCompleteWorkqueue, TxComplete);
    INIT_WORK(&usbNet->RxCompleteWorkqueue, RxComplete);

    usbNet->bh.func = (void (*)(unsigned long))UsbnetAdapterBhTasklet;
    usbNet->bh.data = (unsigned long)&usbNet->delay;    //TimerList    delay;
   
    timer_setup(&usbNet->delay, UsbnetAdapterBh, 0);

    usbNet->net = net;
}


static void UsbnetAdapterSetNetDevice(struct UsbnetAdapter *usbNet, NetDevice *net)
{
    // 1.addr
    static u8 node_id[ETH_ALEN];
    if (usbNet->usbnetInfo.isGetmacAddr) {
        if (memcpy_s(node_id, ETH_ALEN, usbNet->usbnetInfo.macAddr, ETH_ALEN) != EOK) {
            return;
        }
    } else {
        eth_random_addr(node_id);
    }
    memcpy_s(net->dev_addr, sizeof(node_id)/sizeof(node_id[0]), node_id, sizeof(node_id)/sizeof(node_id[0]));
    HARCH_NET_INFO_PRINT("macAddr addr %pM", net->dev_addr);
    // 2. mtu
    net->min_mtu = 0;
    net->max_mtu = ETH_MAX_MTU;
    net->netdev_ops = &g_UsbnetAdapterDeviceOps;
    net->watchdog_timeo = TX_TIMEOUT_JIFFIES;
    net->ethtool_ops = &g_UsbnetAdapterEthtoolOps;

    //3. name and mtu
    strcpy_s (net->name, sizeof(net->name)/sizeof(net->name[0]), "usb%d");
    if (usbNet->usbnetInfo.isBindDevice) {
        if ((usbNet->usbnetInfo.usbFlags & HDF_FLAG_ETHER) != 0 &&
            ((usbNet->usbnetInfo.usbFlags & HDF_FLAG_POINTTOPOINT) == 0 ||
            (net->dev_addr[0] & 0x02) == 0)) {
            HARCH_NET_INFO_PRINT();
            strcpy_s (net->name, sizeof(net->name)/sizeof(net->name[0]), "eth%d");
        }
        
        /* WLAN devices should always be named "wlan%d" */
        if ((usbNet->usbnetInfo.usbFlags & FLAG_WLAN) != 0) {
            HARCH_NET_INFO_PRINT();
            strcpy_s(net->name, sizeof(net->name)/sizeof(net->name[0]), "wlan%d");
        }
        /* WWAN devices should always be named "wwan%d" */
        if ((usbNet->usbnetInfo.usbFlags & FLAG_WWAN) != 0) {
            HARCH_NET_INFO_PRINT();
            strcpy_s(net->name, sizeof(net->name)/sizeof(net->name[0]), "wwan%d");
        }

        /* devices that cannot do ARP */
        if ((usbNet->usbnetInfo.usbFlags & FLAG_NOARP) != 0) {
            net->flags |= IFF_NOARP;
        }

        HARCH_NET_INFO_PRINT("usbNet->usbnetInfo.mtu = %d, net->name= %s", usbNet->usbnetInfo.mtu, net->name);
        /* maybe the remote can't receive an Ethernet MTU */
        net->mtu = usbNet->usbnetInfo.mtu;
        net->hard_header_len = usbNet->usbnetInfo.hardHeaderLen;
    }
    
    /* let userspace know we have a random address */
    if (ether_addr_equal(net->dev_addr, node_id)) {
        net->addr_assign_type = NET_ADDR_RANDOM;
    }
}

static int32_t UsbnetAdapterProbe(struct HdfDeviceObject *device, struct HdfSBuf *data)
{
    HARCH_NET_INFO_PRINT("begin");
    int    ret = HDF_SUCCESS;
    uint32_t infoSize = 0;
    struct UsbnetTransInfo *uNetTransInfo = NULL;
    
    bool flag = HdfSbufReadBuffer(data, (const void **)(&(uNetTransInfo)), &infoSize);
    if ((!flag) || uNetTransInfo == NULL) {
        ret = HDF_ERR_INVALID_PARAM;
        HDF_LOGE("%s: fail to read infoTable in event data, flag = %d", __func__, flag);
        return ret;
    }
    NetDevice *net = alloc_etherdev(sizeof(struct UsbnetAdapter));
    if (net == NULL) {
        HARCH_NET_INFO_PRINT();
        goto out;
    }
    device->priv = net;
    struct UsbnetAdapter *usbNet = netdev_priv(net);
    UsbnetAdapterSetUsbNet(device, net, uNetTransInfo);
    UsbnetAdapterSetNetDevice(usbNet, net);
    HARCH_NET_INFO_PRINT("after usbNet->usbnetInfo.mtu = %d, net->name= %s", usbNet->usbnetInfo.mtu, net->name);
    ret = register_netdev(net);
    if (ret) {
        goto out0;
    }
    HARCH_NET_INFO_PRINT("register net %pM\n", net->dev_addr);
    netif_device_attach(net);
    HARCH_NET_INFO_PRINT("netif_device_attach net %pM\n", net->dev_addr);

    if (usbNet->usbnetInfo.usbFlags & FLAG_LINK_INTR) {
        HARCH_NET_INFO_PRINT();
        UsbnetAdapterLinkChange(usbNet, 0, 0);
    }
    return HDF_SUCCESS;

out0:
    cancel_work_sync(&usbNet->TxCompleteWorkqueue);
    cancel_work_sync(&usbNet->RxCompleteWorkqueue);
    free_netdev(net);
out:
    return ret;
}


static int32_t UsbnetAdapterDisconnect(struct HdfDeviceObject *device)
{
    HARCH_NET_INFO_PRINT("begin");
    if (device == NULL) {
        HDF_LOGI("%s: device is null", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    //free net
    NetDevice *net = (NetDevice *)device->priv;
    unregister_netdev (net);

    struct UsbnetAdapter *usbNet = netdev_priv(net);
    if (usbNet->stats64) {
        free_percpu(usbNet->stats64);
    }
    
    UsbnetAdapterTerminateUrbs(usbNet);
    skb_queue_purge(&usbNet->rxqPause);
    OsalMutexDestroy(&usbNet->sendSkbClock);
   
    free_netdev(net);
    return HDF_SUCCESS;
}

static int32_t UsbnetAdapterDispatch(
    struct HdfDeviceIoClient *client, int32_t cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    HARCH_NET_INFO_PRINT("begin, received cmd = %d", cmd);

    int32_t ret = HDF_ERR_INVALID_PARAM;
    switch (cmd) {
        case USB_NET_REGISTER_NET:
            //add new usbnet device
            ret = UsbnetAdapterProbe(client->device, data);
            break;
        case USB_NET_CLOSE_NET:
            ret = UsbnetAdapterDisconnect(client->device);
            break;
        case USB_NET_RECIVE_DATA_FROM_USB:
            ret = UsbnetAdapterRxComplete(client->device, data, GFP_ATOMIC);
            break;
        default:
            HDF_LOGI("%s: no this cmd: %d", __func__, cmd);
            break;
    }

    if (!HdfSbufWriteInt32(reply, ret)) {
        HDF_LOGE("%s: reply int32 fail", __func__);
    }
    
    return ret;
}

static int32_t UsbnetAdapterBind(struct HdfDeviceObject *device)
{
    HARCH_NET_INFO_PRINT("begin");
    static struct IDeviceIoService UsbnetAdapterService = {
        .Dispatch = UsbnetAdapterDispatch,
    };

    if (device == NULL) {
        HARCH_NET_INFO_PRINT("device is NULL!");
        return HDF_ERR_INVALID_OBJECT;
    }
    device->service = &UsbnetAdapterService;
    return HDF_SUCCESS;
}

static int32_t UsbnetAdapterInit(struct HdfDeviceObject *device)
{
    HARCH_NET_INFO_PRINT("begin");
    if (device == NULL) {
        HARCH_NET_INFO_PRINT("device is null!");
        return HDF_FAILURE;
    }

    HARCH_NET_INFO_PRINT("UsbnetAdapterInit Init success");
    return HDF_SUCCESS;
}

static void UsbnetAdapterRelease(struct HdfDeviceObject *device)
{
    HARCH_NET_INFO_PRINT("begin");
    return;
}

struct HdfDriverEntry g_UsbnetAdapterEntry = {
    .moduleVersion = 1,
    .Bind = UsbnetAdapterBind,
    .Init = UsbnetAdapterInit,
    .Release = UsbnetAdapterRelease,
    .moduleName = "HDF_USB_NET",
};

HDF_INIT(g_UsbnetAdapterEntry);
