/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "osal_thread.h"
#include "hdf_log.h"
#include "prt_sys_external.h"
#include "prt_task.h"
#include "osal_mem.h"
#include "securec.h"

#define HDF_LOG_TAG osal_thread

#define OS_PRIORITY_WIN 3
#define OS_TASK_PRIORITY_LOWEST 31
#define OSAL_THREAD_NAME "hdf_thread"
#define OSAL_INVALID_THREAD_ID UINT32_MAX
#define OSAL_INVALID_CPU_ID UINT32_MAX

struct ThreadWrapper {
    OsalThreadEntry threadEntry;
    void *entryPara;
    uint32_t cpuID;
    TskHandle tid;
};

enum {
    OSAL_PRIORITY_LOW = 15,
    OSAL_PRIORITY_MIDDLE = 16,
    OSAL_PRIORITY_HIGH = 24,
    OSAL_PRIORITY_HIGHEST = 31,
};
int32_t OsalThreadCreate(struct OsalThread *thread, OsalThreadEntry threadEntry, void *entryPara)
{
    struct ThreadWrapper *para = NULL;

    if (thread == NULL || threadEntry == NULL) {
        HDF_LOGE("%s invalid param", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    thread->realThread = NULL;
    para = (struct ThreadWrapper *)OsalMemCalloc(sizeof(*para));
    if (para == NULL) {
        HDF_LOGE("%s malloc fail", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    para->entryPara = entryPara;
    para->threadEntry = threadEntry;
    para->tid = OSAL_INVALID_THREAD_ID;
    para->cpuID = OSAL_INVALID_CPU_ID;
    thread->realThread = para;

    return HDF_SUCCESS;
}

int32_t OsalThreadBind(struct OsalThread *thread, unsigned int cpuID)
{
    struct ThreadWrapper *para = NULL;

    if (thread == NULL || thread->realThread == NULL) {
        HDF_LOGE("%s invalid parameter %d", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }
    para = (struct ThreadWrapper *)thread->realThread;
    para->cpuID = cpuID;
    return HDF_SUCCESS;
}

int32_t OsalThreadStart(struct OsalThread *thread, const struct OsalThreadParam *param)
{
    uint32_t ret;
    struct TskInitParam stTskInitParam;
    uint16_t priority;
    struct ThreadWrapper *para = NULL;

    if (OS_INT_ACTIVE) {
        return HDF_FAILURE;
    }

    if (thread == NULL || thread->realThread == NULL || param == NULL || param->priority > OSAL_THREAD_PRI_HIGHEST) {
        HDF_LOGE("%s invalid parameter %d", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    para = (struct ThreadWrapper *)thread->realThread;
    (void)memset_s(&stTskInitParam, sizeof(struct TskInitParam), 0, sizeof(struct TskInitParam));
    stTskInitParam.taskEntry = (TskEntryFunc)para->threadEntry;
    if (param->stackSize != 0) {
        stTskInitParam.stackSize = param->stackSize;
    }
    stTskInitParam.name = param->name;
    if (param->priority == OSAL_THREAD_PRI_HIGHEST) {
        priority = OSAL_PRIORITY_HIGHEST;
    } else if (param->priority == OSAL_THREAD_PRI_HIGH) {
        priority = OSAL_PRIORITY_HIGH;
    } else if (param->priority == OSAL_THREAD_PRI_DEFAULT) {
        priority = OSAL_PRIORITY_MIDDLE;
    } else {
        priority = OSAL_PRIORITY_LOW;
    }

    stTskInitParam.taskPrio = OS_TASK_PRIORITY_LOWEST - (priority - OS_PRIORITY_WIN);
    stTskInitParam.args[0] = (uintptr_t)para->entryPara;
    ret = PRT_TaskCreate(&para->tid, &stTskInitParam);
    if (ret != OS_OK) {
        para->tid = OSAL_INVALID_THREAD_ID;
        HDF_LOGE("%s PRT_TaskCreate fail %u %u", __func__, ret, priority);
        return HDF_FAILURE;
    }

    ret = PRT_TaskResume(para->tid);
    if (ret != OS_OK) {
        (void)PRT_TaskDelete(para->tid);
        para->tid = OSAL_INVALID_THREAD_ID;
        HDF_LOGE("%s PRT_TaskCreate fail %u %u", __func__, ret, priority);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static bool OsalCheckPara(struct OsalThread *thread)
{
    struct ThreadWrapper *para = NULL;

    if (OS_INT_ACTIVE) {
        return false;
    }

    if (thread == NULL || thread->realThread == NULL) {
        HDF_LOGE("%s invalid parameter %d", __func__, __LINE__);
        return false;
    }

    para = (struct ThreadWrapper *)thread->realThread;
    if (para->tid == OSAL_INVALID_THREAD_ID) {
        HDF_LOGE("%s invalid parameter %d", __func__, __LINE__);
        return false;
    }

    return true;
}

int32_t OsalThreadSuspend(struct OsalThread *thread)
{
    uint32_t ret;
    bool flag = false;

    flag = OsalCheckPara(thread);
    if (!flag) {
        HDF_LOGE("%s invalid parameter %d", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    ret = PRT_TaskSuspend(((struct ThreadWrapper *)thread->realThread)->tid);
    if (ret != OS_OK) {
        HDF_LOGE("%s PRT_TaskSuspend failed %u", __func__, ret);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t OsalThreadDestroy(struct OsalThread *thread)
{
    bool flag = false;

    flag = OsalCheckPara(thread);
    if (!flag) {
        HDF_LOGE("%s invalid parameter %d", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    OsalMemFree(thread->realThread);
    thread->realThread = NULL;

    return HDF_SUCCESS;
}

int32_t OsalThreadResume(struct OsalThread *thread)
{
    uint32_t ret;
    bool flag = false;

    flag = OsalCheckPara(thread);
    if (!flag) {
        HDF_LOGE("%s invalid parameter %d", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    ret = PRT_TaskResume(((struct ThreadWrapper *)thread->realThread)->tid);
    if (ret != OS_OK) {
        HDF_LOGE("%s failed %u %d", __func__, ret, __LINE__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}
