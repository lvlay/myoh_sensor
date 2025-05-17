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

#include <cerrno>
#include <ashmem.h>
#include <unordered_map>
#include <mutex>
#include "ddk_api.h"
#include "ddk_types.h"
#include "hilog_wrapper.h"

#define PORT_MAX 7

using namespace OHOS::ExternalDeviceManager;
namespace {
static std::unordered_map<int32_t, OHOS::sptr<OHOS::Ashmem>> g_shareMemoryMap;
std::mutex g_mutex;
}

DDK_RetCode OH_DDK_CreateAshmem(const uint8_t *name, uint32_t size, DDK_Ashmem **ashmem)
{
    if (name == nullptr) {
        EDM_LOGE(MODULE_BASE_DDK, "invalid buffer name!");
        return DDK_INVALID_PARAMETER;
    }

    if (size == 0) {
        EDM_LOGE(MODULE_BASE_DDK, "invalid buffer size!, size = %{public}d", size);
        return DDK_INVALID_PARAMETER;
    }

    if (ashmem == nullptr) {
        EDM_LOGE(MODULE_BASE_DDK, "invalid pointer of ashmem!");
        return DDK_INVALID_PARAMETER;
    }

    OHOS::sptr<OHOS::Ashmem> shareMemory = OHOS::Ashmem::CreateAshmem(reinterpret_cast<const char*>(name), size);
    if (shareMemory == nullptr) {
        EDM_LOGE(MODULE_BASE_DDK, "create ashmem failed! errno = %{public}d", errno);
        return DDK_FAILURE;
    }

    int32_t fd = shareMemory->GetAshmemFd();
    DDK_Ashmem *ddkAshmem = new DDK_Ashmem({fd, nullptr, size, 0, size, 0});
    if (ddkAshmem == nullptr) {
        EDM_LOGE(MODULE_BASE_DDK, "alloc ddk ashmem failed! errno = %{public}d", errno);
        return DDK_FAILURE;
    }
    *ashmem = ddkAshmem;

    std::lock_guard<std::mutex> lock(g_mutex);
    g_shareMemoryMap[ddkAshmem->ashmemFd] = shareMemory;
    return DDK_SUCCESS;
}

static DDK_RetCode AshmemValidityCheck(DDK_Ashmem *ashmem)
{
    if (ashmem == nullptr) {
        EDM_LOGE(MODULE_BASE_DDK, "ashmem is nullptr!");
        return DDK_NULL_PTR;
    }

    if (g_shareMemoryMap.find(ashmem->ashmemFd) == g_shareMemoryMap.end()) {
        EDM_LOGE(MODULE_BASE_DDK, "ashmemFd dose not exist! error fd = %{public}d", ashmem->ashmemFd);
        return DDK_FAILURE;
    }

    if (g_shareMemoryMap[ashmem->ashmemFd] == nullptr) {
        EDM_LOGE(MODULE_BASE_DDK, "share memory dose not create!");
        return DDK_FAILURE;
    }

    return DDK_SUCCESS;
}

DDK_RetCode OH_DDK_MapAshmem(DDK_Ashmem *ashmem, const uint8_t ashmemMapType)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    DDK_RetCode ret = AshmemValidityCheck(ashmem);
    if (ret != DDK_SUCCESS) {
        EDM_LOGE(MODULE_BASE_DDK, "%{public}s: check the validity of ashmem fail!", __func__);
        return ret;
    }

    if (ashmemMapType > PORT_MAX) {
        EDM_LOGE(MODULE_BASE_DDK, "%{public}s: the ashmemMapType is illegal ,ashmemMapType = %{public}u",
            __func__, ashmemMapType);
        return DDK_INVALID_OPERATION;
    }

    if (!g_shareMemoryMap[ashmem->ashmemFd]->MapAshmem(ashmemMapType)) {
        EDM_LOGE(MODULE_BASE_DDK, "MapAshmem fail! errno = %{public}d", errno);
        return DDK_INVALID_OPERATION;
    }

    ashmem->address =
        reinterpret_cast<const uint8_t *>(g_shareMemoryMap[ashmem->ashmemFd]->ReadFromAshmem(ashmem->size, 0));
    return DDK_SUCCESS;
}

DDK_RetCode OH_DDK_UnmapAshmem(DDK_Ashmem *ashmem)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    DDK_RetCode ret = AshmemValidityCheck(ashmem);
    if (ret != DDK_SUCCESS) {
        EDM_LOGE(MODULE_BASE_DDK, "%{public}s: check the validity of ashmem fail!", __func__);
        return ret;
    }

    g_shareMemoryMap[ashmem->ashmemFd]->UnmapAshmem();
    ashmem->address = nullptr;
    return DDK_SUCCESS;
}

DDK_RetCode OH_DDK_DestroyAshmem(DDK_Ashmem *ashmem)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    DDK_RetCode ret = AshmemValidityCheck(ashmem);
    if (ret != DDK_SUCCESS) {
        EDM_LOGE(MODULE_BASE_DDK, "%{public}s: check the validity of ashmem fail!", __func__);
        return ret;
    }

    g_shareMemoryMap.erase(ashmem->ashmemFd);
    ashmem->address = nullptr;
    delete ashmem;
    ashmem = nullptr;
    return DDK_SUCCESS;
}
