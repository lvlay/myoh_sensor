/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef HILOG_WRAPPER_H
#define HILOG_WRAPPER_H

#define CONFIG_HILOG
#ifdef CONFIG_HILOG
#include "hilog/log.h"
namespace OHOS {
namespace ExternalDeviceManager {

#ifdef EDM_LOGF
#undef EDM_LOGF
#endif

#ifdef EDM_LOGE
#undef EDM_LOGE
#endif

#ifdef EDM_LOGW
#undef EDM_LOGW
#endif

#ifdef EDM_LOGI
#undef EDM_LOGI
#endif

#ifdef EDM_LOGD
#undef EDM_LOGD
#endif

struct EdmLable {
    uint32_t domainId;
    const char* tag;
};

// param of log interface, such as EDM_LOGF.
enum UsbMgrSubModule {
    MODULE_FRAMEWORK = 0,
    MODULE_SERVICE,
    MODULE_DEV_MGR,
    MODULE_PKG_MGR,
    MODULE_EA_MGR,
    MODULE_BUS_USB,
    MODULE_COMMON,
    MODULE_USB_DDK,
    EDM_MODULE_TEST,
    MODULE_HID_DDK,
    MODULE_BASE_DDK,
    EDM_MODULE_BUTT,
};

// 0xD002550: part:ExternalDeviceManager module:Edm.
constexpr unsigned int BASE_EDM_DOMAIN_ID = 0xD002550;

enum UsbMgrDomainId {
    EDM_FRAMEWORK_DOMAIN = BASE_EDM_DOMAIN_ID + MODULE_FRAMEWORK,
    EDM_SERVICE_DOMAIN,
    EDM_DEV_MGR_DOMAIN,
    EDM_PKG_MGR_DOMAIN,
    EDM_EA_MGR_DOMAIN,
    EDM_BUS_USB_DOMAIN,
    EDM_COMMON_DOMAIN,
    EDM_USB_DDK_DOMAIN,
    EDM_TEST,
    EDM_HID_DDK_DOMAIN,
    EDM_BASE_DDK_DOMAIN,
    EDM_BUTT,
};

enum PkgErrCode {
    PKG_OK = 0,
    PKG_FAILURE = -1,
    PKG_RDB_EXECUTE_FAILTURE = -2,
    PKG_RDB_NO_INIT = -3,
    PKG_RDB_EMPTY = -4,
    PKG_PERMISSION_DENIED = -5,
    PKG_NOP = -6,
    PKG_OVERFLOW = -7,
};

static const EdmLable EDM_MGR_LABEL[EDM_MODULE_BUTT] = {
    {EDM_FRAMEWORK_DOMAIN, "EdmFwk"      },
    {EDM_SERVICE_DOMAIN,   "EdmService"  },
    {EDM_DEV_MGR_DOMAIN,   "EdmDevMgr"   },
    {EDM_PKG_MGR_DOMAIN,   "EdmPkgMgr"   },
    {EDM_EA_MGR_DOMAIN,    "EdmEaMgr"    },
    {EDM_BUS_USB_DOMAIN,   "EdmBusUsbMgr"},
    {EDM_COMMON_DOMAIN,    "EdmCommon"   },
    {EDM_USB_DDK_DOMAIN,   "EdmUsbDdk"   },
    {EDM_TEST,             "EdmTest"     },
    {EDM_HID_DDK_DOMAIN,   "EdmHidDdk"   },
    {EDM_BASE_DDK_DOMAIN,  "EdmBaseDdk"  },
};

// In order to improve performance, do not check the module range, module should less than EDM_MODULE_BUTT.
#define EDM_LOGF(module, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_FATAL, EDM_MGR_LABEL[module].domainId, EDM_MGR_LABEL[module].tag, ##__VA_ARGS__))
#define EDM_LOGE(module, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_ERROR, EDM_MGR_LABEL[module].domainId, EDM_MGR_LABEL[module].tag, ##__VA_ARGS__))
#define EDM_LOGW(module, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_WARN, EDM_MGR_LABEL[module].domainId, EDM_MGR_LABEL[module].tag, ##__VA_ARGS__))
#define EDM_LOGI(module, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_INFO, EDM_MGR_LABEL[module].domainId, EDM_MGR_LABEL[module].tag, ##__VA_ARGS__))
#define EDM_LOGD(module, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_DEBUG, EDM_MGR_LABEL[module].domainId, EDM_MGR_LABEL[module].tag, ##__VA_ARGS__))
} // namespace ExternalDeviceManager
} // namespace OHOS

#else

#define EDM_LOGF(...)
#define EDM_LOGE(...)
#define EDM_LOGW(...)
#define EDM_LOGI(...)
#define EDM_LOGD(...)

#endif // CONFIG_HILOG

#endif // HILOG_WRAPPER_H
