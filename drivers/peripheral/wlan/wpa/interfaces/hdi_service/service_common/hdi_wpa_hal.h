/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef HDI_WPA_HAL_H
#define HDI_WPA_HAL_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include "wpa_hal_define.h"
#include "hdi_wpa_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WIFI_CONF_FILE_PATH_LEN 256

typedef struct AddInterfaceArgv {
    char name[WIFI_IFACE_NAME_MAXLEN];
    char confName[WIFI_CONF_FILE_PATH_LEN];
} AddInterfaceArgv;

typedef struct StWpaIfaceInfo {
    char name[WIFI_IFACE_NAME_MAXLEN];
    int num;
    struct StWpaIfaceInfo *next;
} WpaIfaceInfo;

typedef struct StWifiWpaInterface WifiWpaInterface;
struct StWifiWpaInterface {
    WpaCtrl staCtrl;
    WpaCtrl p2pCtrl;
    WpaCtrl chbaCtrl;
    WpaCtrl commonCtrl;
    pthread_t tid;
    int threadRunFlag;
    WpaIfaceInfo *ifaces;

    int (*wpaCliConnect)(WifiWpaInterface *p);
    void (*wpaCliClose)(WifiWpaInterface *p);
    int (*wpaCliAddIface)(WifiWpaInterface *p, const AddInterfaceArgv *argv, bool isWpaAdd);
    int (*wpaCliRemoveIface)(WifiWpaInterface *p, const char *name);
    int (*wpaCliTerminate)();
};

WifiWpaInterface *GetWifiWpaGlobalInterface(void);
void ReleaseWpaGlobalInterface(void);
WpaCtrl *GetStaCtrl(void);
WpaCtrl *GetP2pCtrl(void);
WpaCtrl *GetChbaCtrl(void);
WpaCtrl *GetCommonCtrl(void);
#ifdef __cplusplus
}
#endif
#endif
