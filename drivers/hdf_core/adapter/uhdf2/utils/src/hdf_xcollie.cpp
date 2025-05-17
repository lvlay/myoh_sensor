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
#include <unistd.h>
#include "hdf_xcollie.h"
#include "hdf_log.h"

namespace OHOS {
#ifdef HDFHICOLLIE_ENABLE
#else
#define HDFXCOLLIE_SUCCESS 0
HdfXCollie::HdfXCollie()
{
}
HdfXCollie::~HdfXCollie()
{
}
// NULL impl
int HdfXCollie::SetTimer(const std::string &name, unsigned int timeout,
    std::function<void (void *)> func, void *arg, unsigned int flag)
{
    return HDFXCOLLIE_SUCCESS;
}

// NULL impl
void HdfXCollie::CancelTimer(int id)
{
}

// NULL impl
int HdfXCollie::SetTimerCount(const std::string &name, unsigned int timeLimit, int countLimit)
{
    return HDFXCOLLIE_SUCCESS;
}

// NULL impl
void HdfXCollie::TriggerTimerCount(const std::string &name, bool bTrigger, const std::string &message)
{
}
#endif
}