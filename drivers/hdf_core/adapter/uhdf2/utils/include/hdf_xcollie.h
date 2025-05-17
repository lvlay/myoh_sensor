/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef HDF_UHDF_UTILS_INCLUDE_HDF_XCOLLIE_H
#define HDF_UHDF_UTILS_INCLUDE_HDF_XCOLLIE_H

#include <string>
#ifdef HDFHICOLLIE_ENABLE
#include "xcollie/xcollie.h"
#else
#include "singleton.h"
#endif

namespace OHOS {
#ifdef HDFHICOLLIE_ENABLE
using HdfXCollie = ::OHOS::HiviewDFX::XCollie;
#else
class HdfXCollie : public Singleton<HdfXCollie> {
    DECLARE_SINGLETON(HdfXCollie);
public:
    // NULL impl
    int SetTimer(const std::string &name, unsigned int timeout,
        std::function<void (void *)> func, void *arg, unsigned int flag);

    // NULL impl
    void CancelTimer(int id);

    // NULL impl
    int SetTimerCount(const std::string &name, unsigned int timeLimit, int countLimit);

    // NULL impl
    void TriggerTimerCount(const std::string &name, bool bTrigger, const std::string &message);
};
#endif
}
#endif // HDF_UHDF_UTILS_INCLUDE_HDF_XCOLLIE_H