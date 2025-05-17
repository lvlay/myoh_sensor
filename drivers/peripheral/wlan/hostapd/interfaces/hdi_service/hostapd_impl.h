  /*
   * Copyright (c) 2023 Huawei Device Co., Ltd.
   * Licensed under the Apache License, Version 2.0 (the "License");
   * you may not use this file except in compliance with the License.
   * You may obtain a copy of the License at
   *
   * http://www.apache.org/licenses/LICENSE-2.0
   *
   * Unless required by applicable law or agreed to in writing, software
   * distributed under the License is distributed on an "AS IS" BASIS,
   * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   * See the License for the specific language governing permissions and
   * limitations under the License.
   */

#ifndef HOSTAPD_IMPL_H
#define HOSTAPD_IMPL_H

#include <hdf_base.h>
#include <hdf_device_desc.h>
#include <hdf_log.h>
#include <hdf_remote_service.h>
#include <hdf_sbuf.h>
#include <osal_mem.h>
#include "hdf_dlist.h"
#include "osal_mutex.h"
#include "securec.h"
#include "v1_0/hostapd_interface_stub.h"

struct HdfHostapdRemoteNode {
    struct IHostapdCallback *callbackObj;
    struct HdfRemoteService *service;
    struct DListHead node;
};

struct HdfHostapdStubData {
    struct DListHead remoteListHead;
    struct OsalMutex mutex;
};

struct HdfHostapdStubData *HdfHostapdStubDriver(void);

#endif
