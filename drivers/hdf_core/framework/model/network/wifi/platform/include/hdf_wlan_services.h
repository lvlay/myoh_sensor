/*
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef HDF_WLAN_SERVICES_H
#define HDF_WLAN_SERVICES_H

enum PlatformServiceID {
    INTERFACE_SERVICE_ID = 0,
    BASE_SERVICE_ID,
    AP_SERVICE_ID,
    STA_SERVICE_ID,
    P2P_SERVICE_ID,
    AUTO_ALLOC_SERVICE_ID_START = 300
};

enum BaseCommands {
    CMD_BASE_NEW_KEY,
    CMD_BASE_DEL_KEY,
    CMD_BASE_SET_DEFAULT_KEY,
    CMD_BASE_SEND_MLME,
    CMD_BASE_SEND_EAPOL,
    CMD_BASE_RECEIVE_EAPOL = 5,
    CMD_BASE_ENALBE_EAPOL,
    CMD_BASE_DISABLE_EAPOL,
    CMD_BASE_GET_ADDR,
    CMD_BASE_SET_MODE,
    CMD_BASE_GET_HW_FEATURE = 10,
    CMD_BASE_SET_NETDEV,
    CMD_BASE_SEND_ACTION,
    CMD_BASE_SET_CLIENT,
    CMD_BASE_GET_NETWORK_INFO = 15,
    CMD_BASE_IS_SUPPORT_COMBO,
    CMD_BASE_GET_SUPPORT_COMBO,
    CMD_BASE_GET_DEV_MAC_ADDR,
    CMD_BASE_SET_MAC_ADDR,
    CMD_BASE_GET_VALID_FREQ = 20,
    CMD_BASE_SET_TX_POWER,
    CMD_BASE_GET_CHIPID,
    CMD_BASE_GET_IFNAMES,
    CMD_BASE_RESET_DRIVER,
    CMD_BASE_GET_NETDEV_INFO = 25,
    CMD_BASE_DO_RESET_PRIVATE,
    CMD_BASE_GET_POWER_MODE,
    CMD_BASE_SET_POWER_MODE,
    CMD_BASE_START_CHANNEL_MEAS,
    CMD_BASE_SET_PROJECTION_SCREEN_PARAM,
    CMD_BASE_SEND_CMD_IOCTL,
    CMD_BASE_GET_STATION_INFO
};

enum APCommands {
    CMD_AP_START = 0,
    CMD_AP_STOP,
    CMD_AP_CHANGE_BEACON,
    CMD_AP_DEL_STATION,
    CMD_AP_GET_ASSOC_STA,
    CMD_AP_SET_COUNTRY_CODE,
    CMD_AP_GET_BANDWIDTH,
};

enum STACommands {
    CMD_STA_CONNECT = 0,
    CMD_STA_DISCONNECT,
    CMD_STA_SCAN,
    CMD_STA_ABORT_SCAN,
    CMD_STA_SET_SCAN_MAC_ADDR,
    CMD_STA_START_PNO_SCAN,
    CMD_STA_STOP_PNO_SCAN,
    CMD_STA_GET_SIGNAL_INFO
};

enum P2PCommands {
    CMD_P2P_PROBE_REQ_REPORT = 0,
    CMD_P2P_REMAIN_ON_CHANNEL,
    CMD_P2P_CANCEL_REMAIN_ON_CHANNEL,
    CMD_P2P_ADD_IF,
    CMD_P2P_REMOVE_IF,
    CMD_P2P_SET_AP_WPS_P2P_IE,
    CMD_P2P_GET_DRIVER_FLAGS
};

#endif
