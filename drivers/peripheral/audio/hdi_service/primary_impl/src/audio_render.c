/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <math.h>
#include <sys/mman.h>
#include "hdf_base.h"
#include "hdf_types.h"
#include "osal_mem.h"
#include "audio_adapter_info_common.h"
#include "audio_common.h"
#include "audio_interface_lib_render.h"
#include "audio_internal.h"
#include "audio_uhdf_log.h"
#include "securec.h"

#define HDF_LOG_TAG HDF_AUDIO_PRIMARY_IMPL

#define CONFIG_OUT_LATENCY_MS 100 // unit: ms

/* 1 buffer: 8000(8kHz sample rate) * 2(bytes, PCM_16_BIT) * 1(channel) */
/* 1 frame: 1024(sample) * 2(bytes, PCM_16_BIT) * 1(channel) */
#define CONFIG_FRAME_SIZE  (1024 * 2 * 1)
#define FRAME_SIZE         1024
#define CONFIG_FRAME_COUNT ((8000 * 2 * 1 + ((CONFIG_FRAME_SIZE) - 1)) / (CONFIG_FRAME_SIZE))

#define DEEP_BUFFER_PLATFORM_DELAY (29 * 1000LL)
#define LOW_LATENCY_PLATFORM_DELAY (13 * 1000LL)
#define BITS_TO_FROMAT 3
#define VOLUME_AVERAGE 2
#define SEC_TO_MILLSEC 1000
#define INTEGER_TO_DEC 10
#define DECIMAL_PART   5
#define RENDER_2_CHS   2
#define PCM_8_BIT      8
#define PCM_16_BIT     16

int32_t PcmBytesToFrames(const struct AudioFrameRenderMode *frameRenderMode,
    uint64_t bytes, uint32_t *frameCount)
{
    if (frameRenderMode == NULL || frameCount == NULL) {
        AUDIO_FUNC_LOGE("Parameter error!");
        return AUDIO_ERR_INVALID_PARAM;
    }

    uint32_t formatBits = 0;

    int32_t ret = FormatToBits(frameRenderMode->attrs.format, &formatBits);
    if (ret != HDF_SUCCESS) {
        AUDIO_FUNC_LOGE("FormatToBits error!");
        return ret;
    }

    /* channelCount Not greater than 4 , formatBits Not greater than 64 */
    if (frameRenderMode->attrs.channelCount > 4 || formatBits > 64) {
        AUDIO_FUNC_LOGE("channelCount or formatBits error!");
        return AUDIO_ERR_INTERNAL;
    }

    uint32_t frameSize = frameRenderMode->attrs.channelCount * (formatBits >> 3); // Bit to byte >> 3
    if (frameSize == 0) {
        AUDIO_FUNC_LOGE("frameSize error!");
        return AUDIO_ERR_INTERNAL;
    }
    *frameCount = (uint32_t)bytes / frameSize;
    return AUDIO_SUCCESS;
}

int32_t AudioRenderStart(struct IAudioRender *handle)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL) {
        AUDIO_FUNC_LOGE("The pointer is null");
        return AUDIO_ERR_INVALID_PARAM;
    }

    InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
        return AUDIO_ERR_INTERNAL;
    }
    pthread_mutex_lock(&hwRender->renderParam.frameRenderMode.mutex);
    if (hwRender->renderParam.frameRenderMode.buffer != NULL) {
        pthread_mutex_unlock(&hwRender->renderParam.frameRenderMode.mutex);
        AUDIO_FUNC_LOGE("IAudioRender already start!");
        return AUDIO_ERR_AO_BUSY; // render is busy now
    }
    if (hwRender->devDataHandle == NULL) {
        pthread_mutex_unlock(&hwRender->renderParam.frameRenderMode.mutex);
        AUDIO_FUNC_LOGE("devDataHandle is null!");
        return AUDIO_ERR_INTERNAL;
    }

    int32_t ret =
        (*pInterfaceLibModeRender)(hwRender->devDataHandle, &hwRender->renderParam, AUDIO_DRV_PCM_IOCTRL_START);
    if (ret < 0) {
        pthread_mutex_unlock(&hwRender->renderParam.frameRenderMode.mutex);
        AUDIO_FUNC_LOGE("AudioRenderStart SetParams FAIL");
        return AUDIO_ERR_INTERNAL;
    }

    char *buffer = (char *)OsalMemCalloc(FRAME_DATA);
    if (buffer == NULL) {
        pthread_mutex_unlock(&hwRender->renderParam.frameRenderMode.mutex);
        AUDIO_FUNC_LOGE("Calloc Render buffer Fail!");
        return AUDIO_ERR_MALLOC_FAIL;
    }

    hwRender->renderParam.frameRenderMode.buffer = buffer;
    pthread_mutex_unlock(&hwRender->renderParam.frameRenderMode.mutex);

    AudioLogRecord(AUDIO_INFO, "[%s]-[%s]-[%d] :> [%s]", __FILE__, __func__, __LINE__, "Audio Render Start");
    return AUDIO_SUCCESS;
}

int32_t AudioRenderStop(struct IAudioRender *handle)
{
    AUDIO_FUNC_LOGD("Enter.");
    int32_t ret = AUDIO_SUCCESS;
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL) {
        AUDIO_FUNC_LOGE("hwRender is invalid");
        return AUDIO_ERR_INVALID_PARAM;
    }
    do {
        if (hwRender->devDataHandle == NULL) {
            AUDIO_FUNC_LOGE("RenderStart Bind Fail!");
            ret = AUDIO_ERR_INTERNAL;
            break;
        }

        InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
        if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
            AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
            ret = AUDIO_ERR_INTERNAL;
            break;
        }
        ret =
            (*pInterfaceLibModeRender)(hwRender->devDataHandle, &hwRender->renderParam, AUDIO_DRV_PCM_IOCTRL_STOP);
        hwRender->renderParam.renderMode.ctlParam.turnStandbyStatus = AUDIO_TURN_STANDBY_LATER;
        if (ret < 0) {
            AUDIO_FUNC_LOGE("AudioRenderStop SetParams FAIL");
            ret = AUDIO_ERR_INTERNAL;
            break;
        }  
    } while (0);
    
    pthread_mutex_lock(&hwRender->renderParam.frameRenderMode.mutex);
    if (hwRender->renderParam.frameRenderMode.buffer != NULL) {
        AudioMemFree((void **)&hwRender->renderParam.frameRenderMode.buffer);
    } else {
        AUDIO_FUNC_LOGE("Repeat invalid stop operation!");
        ret = AUDIO_ERR_NOT_SUPPORT;
    }
    pthread_mutex_unlock(&hwRender->renderParam.frameRenderMode.mutex);

    AudioLogRecord(AUDIO_INFO, "[%s]-[%s]-[%d] :> [%s]", __FILE__, __func__, __LINE__, "Audio Render Stop");
    return ret;
}

int32_t AudioRenderPause(struct IAudioRender *handle)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL) {
        AUDIO_FUNC_LOGE("hwRender is null");
        return AUDIO_ERR_INVALID_PARAM;
    }
    pthread_mutex_lock(&hwRender->renderParam.frameRenderMode.mutex);
    if (hwRender->renderParam.frameRenderMode.buffer == NULL) {
        pthread_mutex_unlock(&hwRender->renderParam.frameRenderMode.mutex);
        AUDIO_FUNC_LOGE("IAudioRender already stop!");
        return AUDIO_ERR_INTERNAL;
    }
    pthread_mutex_unlock(&hwRender->renderParam.frameRenderMode.mutex);
    if (hwRender->renderParam.renderMode.ctlParam.pause) {
        AUDIO_FUNC_LOGE("Audio is already pause!");
        return AUDIO_ERR_NOT_SUPPORT;
    }
    if (hwRender->devDataHandle == NULL) {
        AUDIO_FUNC_LOGE("RenderPause Bind Fail!");
        return AUDIO_ERR_INTERNAL;
    }

    InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
        return AUDIO_ERR_INTERNAL;
    }

    bool pauseStatus = hwRender->renderParam.renderMode.ctlParam.pause;

    hwRender->renderParam.renderMode.ctlParam.pause = true;
    int32_t ret =
        (*pInterfaceLibModeRender)(hwRender->devDataHandle, &hwRender->renderParam, AUDIODRV_CTL_IOCTL_PAUSE_WRITE);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("RenderPause FAIL!");
        hwRender->renderParam.renderMode.ctlParam.pause = pauseStatus;
        return AUDIO_ERR_INTERNAL;
    }

    AudioLogRecord(AUDIO_INFO, "[%s]-[%s]-[%d] :> [%s]", __FILE__, __func__, __LINE__, "Audio Render Pause");
    return AUDIO_SUCCESS;
}

int32_t AudioRenderResume(struct IAudioRender *handle)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL) {
        AUDIO_FUNC_LOGE("Param is error");
        return AUDIO_ERR_INVALID_PARAM;
    }
    if (!hwRender->renderParam.renderMode.ctlParam.pause) {
        AUDIO_FUNC_LOGE("Audio is already Resume !");
        return AUDIO_ERR_NOT_SUPPORT;
    }
    if (hwRender->devDataHandle == NULL) {
        AUDIO_FUNC_LOGE("RenderResume Bind Fail!");
        return AUDIO_ERR_INTERNAL;
    }

    InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
        return AUDIO_ERR_INTERNAL;
    }

    bool resumeStatus = hwRender->renderParam.renderMode.ctlParam.pause;

    hwRender->renderParam.renderMode.ctlParam.pause = false;
    int32_t ret =
        (*pInterfaceLibModeRender)(hwRender->devDataHandle, &hwRender->renderParam, AUDIODRV_CTL_IOCTL_PAUSE_WRITE);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("RenderResume FAIL!");
        hwRender->renderParam.renderMode.ctlParam.pause = resumeStatus;
        return AUDIO_ERR_INTERNAL;
    }

    AudioLogRecord(AUDIO_INFO, "[%s]-[%s]-[%d] :> [%s]", __FILE__, __func__, __LINE__, "Audio Render Resume");
    return AUDIO_SUCCESS;
}

int32_t AudioRenderFlush(struct IAudioRender *handle)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL) {
        AUDIO_FUNC_LOGE("Param is error");
        return AUDIO_ERR_INVALID_PARAM;
    }

    bool callBackStatus = hwRender->renderParam.renderMode.hwInfo.callBackEnable;
    if (callBackStatus) {
        return CallbackProcessing(hwRender, AUDIO_FLUSH_COMPLETED);
    }
    return AUDIO_ERR_NOT_SUPPORT;
}

int32_t AudioRenderGetFrameSize(struct IAudioRender *handle, uint64_t *size)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL || size == NULL) {
        AUDIO_FUNC_LOGE("Param is error");
        return AUDIO_ERR_INVALID_PARAM;
    }

    uint32_t channelCount = hwRender->renderParam.frameRenderMode.attrs.channelCount;
    enum AudioFormat format = hwRender->renderParam.frameRenderMode.attrs.format;

    uint32_t formatBits = 0;
    int32_t ret = FormatToBits(format, &formatBits);
    if (ret != AUDIO_SUCCESS) {
        return ret;
    }
    *size = FRAME_SIZE * channelCount * (formatBits >> BITS_TO_FROMAT);
    return AUDIO_SUCCESS;
}

int32_t AudioRenderGetFrameCount(struct IAudioRender *handle, uint64_t *count)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL || count == NULL) {
        AUDIO_FUNC_LOGE("Param is error");
        return AUDIO_ERR_INVALID_PARAM;
    }
    *count = hwRender->renderParam.frameRenderMode.frames;
    return AUDIO_SUCCESS;
}

int32_t AudioRenderSetSampleAttributes(struct IAudioRender *handle, const struct AudioSampleAttributes *attrs)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL || attrs == NULL || hwRender->devDataHandle == NULL) {
        AUDIO_FUNC_LOGE("Param is error");
        return AUDIO_ERR_INVALID_PARAM;
    }

    int32_t ret = AudioCheckParaAttr(attrs);
    if (ret != AUDIO_SUCCESS) {
        return ret;
    }

    /* attrs temp */
    struct AudioSampleAttributes tempAttrs = hwRender->renderParam.frameRenderMode.attrs;
    hwRender->renderParam.frameRenderMode.attrs = *attrs;

    InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL || hwRender->devDataHandle == NULL) {
        hwRender->renderParam.frameRenderMode.attrs = tempAttrs;
        AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
        return AUDIO_ERR_INTERNAL;
    }

    ret = (*pInterfaceLibModeRender)(hwRender->devDataHandle, &hwRender->renderParam, AUDIO_DRV_PCM_IOCTL_HW_PARAMS);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("SetSampleAttributes FAIL");
        hwRender->renderParam.frameRenderMode.attrs = tempAttrs;
        return AUDIO_ERR_INTERNAL;
    }
    return AUDIO_SUCCESS;
}

int32_t AudioRenderGetSampleAttributes(struct IAudioRender *handle, struct AudioSampleAttributes *attrs)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL || attrs == NULL) {
        AUDIO_FUNC_LOGE("Param is error");
        return AUDIO_ERR_INVALID_PARAM;
    }

    attrs->format = hwRender->renderParam.frameRenderMode.attrs.format;
    attrs->sampleRate = hwRender->renderParam.frameRenderMode.attrs.sampleRate;
    attrs->channelCount = hwRender->renderParam.frameRenderMode.attrs.channelCount;
    attrs->type = hwRender->renderParam.frameRenderMode.attrs.type;
    attrs->interleaved = hwRender->renderParam.frameRenderMode.attrs.interleaved;
    attrs->period = hwRender->renderParam.frameRenderMode.attrs.period;
    attrs->frameSize = hwRender->renderParam.frameRenderMode.attrs.frameSize;
    attrs->isBigEndian = hwRender->renderParam.frameRenderMode.attrs.isBigEndian;
    attrs->isSignedData = hwRender->renderParam.frameRenderMode.attrs.isSignedData;
    attrs->startThreshold = hwRender->renderParam.frameRenderMode.attrs.startThreshold;
    attrs->stopThreshold = hwRender->renderParam.frameRenderMode.attrs.stopThreshold;
    attrs->silenceThreshold = hwRender->renderParam.frameRenderMode.attrs.silenceThreshold;
    return AUDIO_SUCCESS;
}

int32_t AudioRenderGetCurrentChannelId(struct IAudioRender *handle, uint32_t *channelId)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL || channelId == NULL) {
        AUDIO_FUNC_LOGE("Param is error");
        return AUDIO_ERR_INVALID_PARAM;
    }
    *channelId = hwRender->renderParam.frameRenderMode.attrs.channelCount;
    return AUDIO_SUCCESS;
}

int32_t AudioRenderCheckSceneCapability(
    struct IAudioRender *handle, const struct AudioSceneDescriptor *scene, bool *supported)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL || scene == NULL || supported == NULL) {
        AUDIO_FUNC_LOGE("Param is error");
        return AUDIO_ERR_INVALID_PARAM;
    }
#ifndef AUDIO_HAL_NOTSUPPORT_PATHSELECT
    *supported = false;
    /* Temporary storage does not save the structure */
    struct AudioHwRenderParam renderParam = hwRender->renderParam;
    renderParam.frameRenderMode.attrs.type = (enum AudioCategory)scene->scene.id;
    renderParam.renderMode.hwInfo.deviceDescript.pins = scene->desc.pins;

    PathSelAnalysisJson *pPathSelAnalysisJson = AudioPassthroughGetPathSelAnalysisJson();
    if (pPathSelAnalysisJson == NULL) {
        AUDIO_FUNC_LOGE("pPathSelAnalysisJson Is NULL!");
        return AUDIO_ERR_NOT_SUPPORT;
    }

    int32_t ret = (*pPathSelAnalysisJson)((void *)&renderParam, CHECKSCENE_PATH_SELECT);
    if (ret < 0) {
        if (ret == AUDIO_ERR_NOT_SUPPORT) {
            AUDIO_FUNC_LOGE("AudioRenderCheckSceneCapability not Support!");
            return AUDIO_ERR_NOT_SUPPORT;
        } else {
            AUDIO_FUNC_LOGE("AudioRenderCheckSceneCapability fail!");
            return AUDIO_ERR_INTERNAL;
        }
    }
    *supported = true;
    return AUDIO_SUCCESS;
#else
    return AUDIO_ERR_NOT_SUPPORT;
#endif
}

int32_t AudioRenderSelectScene(struct IAudioRender *handle, const struct AudioSceneDescriptor *scene)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL || scene == NULL) {
        AUDIO_FUNC_LOGE("Param is error");
        return AUDIO_ERR_INVALID_PARAM;
    }
    if (hwRender->devCtlHandle == NULL) {
        AUDIO_FUNC_LOGE("RenderSelectScene Bind Fail!");
        return AUDIO_ERR_INTERNAL;
    }
#ifndef AUDIO_HAL_NOTSUPPORT_PATHSELECT
    PathSelAnalysisJson *pPathSelAnalysisJson = AudioPassthroughGetPathSelAnalysisJson();
    if (pPathSelAnalysisJson == NULL) {
        AUDIO_FUNC_LOGE("pPathSelAnalysisJson Is NULL!");
        return AUDIO_ERR_NOT_SUPPORT;
    }
    enum AudioCategory sceneId = hwRender->renderParam.frameRenderMode.attrs.type;
    enum AudioPortPin descPins = hwRender->renderParam.renderMode.hwInfo.deviceDescript.pins;

    hwRender->renderParam.frameRenderMode.attrs.type = (enum AudioCategory)(scene->scene.id);
    hwRender->renderParam.renderMode.hwInfo.deviceDescript.pins = scene->desc.pins;
    if ((*pPathSelAnalysisJson)((void *)&hwRender->renderParam, RENDER_PATH_SELECT) < 0) {
        AUDIO_FUNC_LOGE("AudioRenderSelectScene Fail!");
        hwRender->renderParam.frameRenderMode.attrs.type = sceneId;
        hwRender->renderParam.renderMode.hwInfo.deviceDescript.pins = descPins;
        return AUDIO_ERR_INTERNAL;
    }

    InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
        hwRender->renderParam.frameRenderMode.attrs.type = sceneId;
        hwRender->renderParam.renderMode.hwInfo.deviceDescript.pins = descPins;
        return AUDIO_ERR_INTERNAL;
    }

    int32_t ret = (*pInterfaceLibModeRender)(
        hwRender->devCtlHandle, &hwRender->renderParam, AUDIODRV_CTL_IOCTL_SCENESELECT_WRITE);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("SetParams FAIL!");
        hwRender->renderParam.frameRenderMode.attrs.type = sceneId;
        hwRender->renderParam.renderMode.hwInfo.deviceDescript.pins = descPins;
        return AUDIO_ERR_INTERNAL;
    }
    return AUDIO_SUCCESS;
#else
    return AUDIO_ERR_NOT_SUPPORT;
#endif
}

int32_t AudioRenderSetMute(struct IAudioRender *handle, bool mute)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *impl = (struct AudioHwRender *)handle;
    if (impl == NULL) {
        AUDIO_FUNC_LOGE("impl is error");
        return AUDIO_ERR_INVALID_PARAM;
    }
    if (impl->devCtlHandle == NULL) {
        AUDIO_FUNC_LOGE("RenderSetMute Bind Fail!");
        return AUDIO_ERR_INTERNAL;
    }

    InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
        return AUDIO_ERR_INTERNAL;
    }

    bool muteStatus = impl->renderParam.renderMode.ctlParam.mute;
    impl->renderParam.renderMode.ctlParam.mute = mute;

    int32_t ret = (*pInterfaceLibModeRender)(impl->devCtlHandle, &impl->renderParam, AUDIODRV_CTL_IOCTL_MUTE_WRITE);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("SetMute SetParams FAIL");
        impl->renderParam.renderMode.ctlParam.mute = muteStatus;
        return AUDIO_ERR_INTERNAL;
    }
    AudioLogRecord(AUDIO_INFO, "[%s]-[%s]-[%d] :> [Setmute = %d]", __FILE__, __func__, __LINE__, mute);
    return AUDIO_SUCCESS;
}

int32_t AudioRenderGetMute(struct IAudioRender *handle, bool *mute)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *impl = (struct AudioHwRender *)handle;
    if (impl == NULL || mute == NULL) {
        AUDIO_FUNC_LOGE("RenderGetMute Bind Fail!");
        return AUDIO_ERR_INVALID_PARAM;
    }

    if (impl->devCtlHandle == NULL) {
        AUDIO_FUNC_LOGE("RenderGetMute Bind Fail!");
        return AUDIO_ERR_INTERNAL;
    }

    InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
        return AUDIO_ERR_INTERNAL;
    }

    int32_t ret =
        (*pInterfaceLibModeRender)(impl->devCtlHandle, &impl->renderParam, AUDIODRV_CTL_IOCTL_MUTE_READ);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("Get Mute FAIL!");
        return AUDIO_ERR_INTERNAL;
    }
    *mute = impl->renderParam.renderMode.ctlParam.mute;
    AUDIO_FUNC_LOGI("GetMute SUCCESS!");
    return AUDIO_SUCCESS;
}

int32_t AudioRenderSetVolume(struct IAudioRender *handle, float volume)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL) {
        AUDIO_FUNC_LOGE("hwRender is error");
        return AUDIO_ERR_INVALID_PARAM;
    }

    float volumeTemp = hwRender->renderParam.renderMode.ctlParam.volume;
    float volMax = (float)hwRender->renderParam.renderMode.ctlParam.volThreshold.volMax;
    float volMin = (float)hwRender->renderParam.renderMode.ctlParam.volThreshold.volMin;
    if (volume < 0 || volume > 1) {
        AUDIO_FUNC_LOGE("volume param Is error!");
        return AUDIO_ERR_INVALID_PARAM;
    }
    if (hwRender->devCtlHandle == NULL) {
        AUDIO_FUNC_LOGE("RenderSetVolume Bind Fail!");
        return AUDIO_ERR_INTERNAL;
    }

    InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
        return AUDIO_ERR_INTERNAL;
    }

    volume = (volume == 0) ? 1 : (volume * VOLUME_CHANGE);
    /* change volume to db */
    float volTemp = ((volMax - volMin) / 2) * log10(volume) + volMin;
    if (volTemp < volMin || volTemp > volMax) {
        AUDIO_FUNC_LOGE("volTemp fail");
        return AUDIO_ERR_INTERNAL;
    }
    hwRender->renderParam.renderMode.ctlParam.volume = volTemp;

    int32_t ret =
        (*pInterfaceLibModeRender)(hwRender->devCtlHandle, &hwRender->renderParam, AUDIODRV_CTL_IOCTL_ELEM_WRITE);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("RenderSetVolume FAIL!");
        hwRender->renderParam.renderMode.ctlParam.volume = volumeTemp;
        return AUDIO_ERR_INTERNAL;
    }
    return AUDIO_SUCCESS;
}

int32_t AudioRenderGetVolume(struct IAudioRender *handle, float *volume)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL || volume == NULL) {
        AUDIO_FUNC_LOGE("Param is error");
        return AUDIO_ERR_INVALID_PARAM;
    }
    if (hwRender->devCtlHandle == NULL) {
        AUDIO_FUNC_LOGE("RenderGetVolume Bind Fail!");
        return AUDIO_ERR_INTERNAL;
    }

    InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
        return AUDIO_ERR_INTERNAL;
    }

    int32_t ret =
        (*pInterfaceLibModeRender)(hwRender->devCtlHandle, &hwRender->renderParam, AUDIODRV_CTL_IOCTL_ELEM_READ);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("RenderGetVolume FAIL!");
        return AUDIO_ERR_INTERNAL;
    }

    float volumeTemp = hwRender->renderParam.renderMode.ctlParam.volume;
    float volMax = (float)hwRender->renderParam.renderMode.ctlParam.volThreshold.volMax;
    float volMin = (float)hwRender->renderParam.renderMode.ctlParam.volThreshold.volMin;
    if ((volMax - volMin) == 0) {
        AUDIO_FUNC_LOGE("Divisor cannot be zero!");
        return AUDIO_ERR_INTERNAL;
    }

    volumeTemp = (volumeTemp - volMin) / ((volMax - volMin) / VOLUME_AVERAGE);

    int volumeT = (int)((pow(INTEGER_TO_DEC, volumeTemp) + DECIMAL_PART) / INTEGER_TO_DEC); // delet 0.X num

    *volume = (float)volumeT / INTEGER_TO_DEC;                                               // get volume (0-1)
    return AUDIO_SUCCESS;
}

int32_t AudioRenderGetGainThreshold(struct IAudioRender *handle, float *min, float *max)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL || min == NULL || max == NULL) {
        return AUDIO_ERR_INVALID_PARAM;
    }
    if (hwRender->devCtlHandle == NULL) {
        AUDIO_FUNC_LOGE("RenderGetGainThreshold Bind Fail!");
        return AUDIO_ERR_INTERNAL;
    }

    InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
        return AUDIO_ERR_INTERNAL;
    }

    int32_t ret = (*pInterfaceLibModeRender)(
        hwRender->devCtlHandle, &hwRender->renderParam, AUDIODRV_CTL_IOCTL_GAINTHRESHOLD_READ);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("GetGainThreshold FAIL!");
        return AUDIO_ERR_INTERNAL;
    }

    *max = hwRender->renderParam.renderMode.ctlParam.audioGain.gainMax;
    *min = hwRender->renderParam.renderMode.ctlParam.audioGain.gainMin;
    return AUDIO_SUCCESS;
}

int32_t AudioRenderGetGain(struct IAudioRender *handle, float *gain)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *impl = (struct AudioHwRender *)handle;
    if (impl == NULL || gain == NULL) {
        AUDIO_FUNC_LOGE("Param is error");
        return AUDIO_ERR_INVALID_PARAM;
    }
    if (impl->devCtlHandle == NULL) {
        AUDIO_FUNC_LOGE("RenderGetGain Bind Fail!");
        return AUDIO_ERR_INTERNAL;
    }

    InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
        return AUDIO_ERR_INTERNAL;
    }

    int32_t ret =
        (*pInterfaceLibModeRender)(impl->devCtlHandle, &impl->renderParam, AUDIODRV_CTL_IOCTL_GAIN_READ);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("RenderGetGain FAIL");
        return AUDIO_ERR_INTERNAL;
    }

    *gain = impl->renderParam.renderMode.ctlParam.audioGain.gain;
    return AUDIO_SUCCESS;
}

int32_t AudioRenderSetGain(struct IAudioRender *handle, float gain)
{
    AUDIO_FUNC_LOGD("Enter.");
    int32_t ret = 0;
    struct AudioHwRender *impl = (struct AudioHwRender *)handle;
    if (impl == NULL || gain < 0) {
        AUDIO_FUNC_LOGE("Param is error");
        return AUDIO_ERR_INVALID_PARAM;
    }

    float gainTemp = impl->renderParam.renderMode.ctlParam.audioGain.gain;
    impl->renderParam.renderMode.ctlParam.audioGain.gain = gain;
    if (impl->devCtlHandle == NULL) {
        AUDIO_FUNC_LOGE("RenderSetGain Bind Fail!");
        impl->renderParam.renderMode.ctlParam.audioGain.gain = gainTemp;
        return AUDIO_ERR_INTERNAL;
    }

    InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
        impl->renderParam.renderMode.ctlParam.audioGain.gain = gainTemp;
        return AUDIO_ERR_INTERNAL;
    }

    ret = (*pInterfaceLibModeRender)(impl->devCtlHandle, &impl->renderParam, AUDIODRV_CTL_IOCTL_GAIN_WRITE);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("RenderSetGain FAIL");
        impl->renderParam.renderMode.ctlParam.audioGain.gain = gainTemp;
        return AUDIO_ERR_INTERNAL;
    }
    return AUDIO_SUCCESS;
}

int32_t AudioRenderGetLatency(struct IAudioRender *render, uint32_t *ms)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *impl = (struct AudioHwRender *)render;
    if (impl == NULL || ms == NULL) {
        AUDIO_FUNC_LOGE("impl or ms is null!");
        return AUDIO_ERR_INVALID_PARAM;
    }

    uint32_t byteRate = impl->renderParam.frameRenderMode.byteRate;
    uint32_t periodSize = impl->renderParam.frameRenderMode.periodSize;

    if (byteRate == 0) {
        AUDIO_FUNC_LOGE("byteRate error!");
        return AUDIO_ERR_INTERNAL;
    }

    *ms = (periodSize * SEC_TO_MILLSEC) / (byteRate * RENDER_2_CHS * PCM_16_BIT / PCM_8_BIT);
    return AUDIO_SUCCESS;
}

static int32_t LogErrorGetRensonAndTime(struct AudioHwRender *hwRender, int errorReason)
{
    if (hwRender == NULL) {
        AUDIO_FUNC_LOGE("hwRender is null!");
        return AUDIO_ERR_INVALID_PARAM;
    }

    if (hwRender->errorLog.iter >= ERROR_LOG_MAX_NUM) {
        AUDIO_FUNC_LOGE("hwRender->errorLog.iter >= ERROR_LOG_MAX_NUM error!");
        return AUDIO_ERR_INVALID_PARAM;
    }

    if (hwRender->errorLog.errorDump[hwRender->errorLog.iter].reason == NULL) {
        hwRender->errorLog.errorDump[hwRender->errorLog.iter].reason = (char *)OsalMemCalloc(ERROR_REASON_DESC_LEN);
        if (hwRender->errorLog.errorDump[hwRender->errorLog.iter].reason == NULL) {
            AUDIO_FUNC_LOGE("Calloc reasonDesc Fail!");
            return AUDIO_ERR_MALLOC_FAIL;
        }
    }

    if (hwRender->errorLog.errorDump[hwRender->errorLog.iter].currentTime == NULL) {
        hwRender->errorLog.errorDump[hwRender->errorLog.iter].currentTime =
            (char *)OsalMemCalloc(ERROR_REASON_DESC_LEN);
        if (hwRender->errorLog.errorDump[hwRender->errorLog.iter].currentTime == NULL) {
            AUDIO_FUNC_LOGE("Calloc time Fail!");
            return AUDIO_ERR_MALLOC_FAIL;
        }
    }

    memset_s(
        hwRender->errorLog.errorDump[hwRender->errorLog.iter].reason, ERROR_REASON_DESC_LEN, 0, ERROR_REASON_DESC_LEN);
    memset_s(hwRender->errorLog.errorDump[hwRender->errorLog.iter].currentTime, ERROR_REASON_DESC_LEN, 0,
        ERROR_REASON_DESC_LEN);

    int32_t ret = GetErrorReason(errorReason, hwRender->errorLog.errorDump[hwRender->errorLog.iter].reason);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("GetErrorReason failed!");
        return AUDIO_ERR_INTERNAL;
    }

    ret = GetCurrentTime(hwRender->errorLog.errorDump[hwRender->errorLog.iter].currentTime);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("GetCurrentTime Fail");
        return AUDIO_ERR_INTERNAL;
    }
    return AUDIO_SUCCESS;
}

static void LogError(AudioHandle handle, int32_t errorCode, int reason)
{
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL) {
        AUDIO_FUNC_LOGE("hwRender is null!");
        return;
    }

    hwRender->errorLog.totalErrors++;
    if (hwRender->errorLog.iter >= ERROR_LOG_MAX_NUM) {
        hwRender->errorLog.iter = 0;
    }

    int32_t ret = LogErrorGetRensonAndTime(hwRender, reason);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("LogErrorGetRensonAndTime error!");
        return;
    }

    if (errorCode == WRITE_FRAME_ERROR_CODE) {
        hwRender->errorLog.errorDump[hwRender->errorLog.iter].errorCode = errorCode;
        hwRender->errorLog.errorDump[hwRender->errorLog.iter].count = hwRender->errorLog.iter;
        hwRender->errorLog.errorDump[hwRender->errorLog.iter].frames = hwRender->renderParam.frameRenderMode.frames;
        hwRender->errorLog.iter++;
    }
}

static int32_t AudioRenderRenderFramSplit(struct AudioHwRender *hwRender)
{
    if (hwRender == NULL) {
        AUDIO_FUNC_LOGE("hwRender is null!");
        return HDF_FAILURE;
    }

    InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
        return HDF_FAILURE;
    }
    if (hwRender->devDataHandle == NULL) {
        AUDIO_FUNC_LOGE("hwRender->devDataHandle is null!");
        return HDF_FAILURE;
    }

    int32_t ret =
        (*pInterfaceLibModeRender)(hwRender->devDataHandle, &hwRender->renderParam, AUDIO_DRV_PCM_IOCTL_WRITE);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("Render Frame FAIL!");
        LogError((AudioHandle)hwRender, WRITE_FRAME_ERROR_CODE, ret);
        return AUDIO_ERR_INTERNAL;
    }
    return HDF_SUCCESS;
}

int32_t AudioRenderRenderFrame(
    struct IAudioRender *render, const int8_t *frame, uint32_t frameLen, uint64_t *replyBytes)
{
    struct AudioHwRender *hwRender = (struct AudioHwRender *)render;
    pthread_mutex_lock(&hwRender->renderParam.frameRenderMode.mutex);
    if (hwRender == NULL || frame == NULL || replyBytes == NULL ||
        hwRender->renderParam.frameRenderMode.buffer == NULL) {
        pthread_mutex_unlock(&hwRender->renderParam.frameRenderMode.mutex);
        AUDIO_FUNC_LOGE("Render Frame Paras is NULL!");
        return AUDIO_ERR_INVALID_PARAM;
    }
    if (frameLen > FRAME_DATA) {
        pthread_mutex_unlock(&hwRender->renderParam.frameRenderMode.mutex);
        AUDIO_FUNC_LOGE("Out of FRAME_DATA size!");
        return AUDIO_ERR_INTERNAL;
    }

    int32_t ret = memcpy_s(hwRender->renderParam.frameRenderMode.buffer, FRAME_DATA, frame, frameLen);
    if (ret != EOK) {
        pthread_mutex_unlock(&hwRender->renderParam.frameRenderMode.mutex);
        AUDIO_FUNC_LOGE("memcpy_s fail");
        return AUDIO_ERR_INTERNAL;
    }

    hwRender->renderParam.frameRenderMode.bufferSize = (uint64_t)frameLen;
    uint32_t frameCount = 0;
    ret = PcmBytesToFrames(&hwRender->renderParam.frameRenderMode, (uint64_t)frameLen, &frameCount);
    if (ret != AUDIO_SUCCESS) {
        pthread_mutex_unlock(&hwRender->renderParam.frameRenderMode.mutex);
        AUDIO_FUNC_LOGE("PcmBytesToFrames error!");
        return ret;
    }

    hwRender->renderParam.frameRenderMode.bufferFrameSize = (uint64_t)frameCount;
    if (AudioRenderRenderFramSplit(hwRender) < 0) {
        pthread_mutex_unlock(&hwRender->renderParam.frameRenderMode.mutex);
        AUDIO_FUNC_LOGE("AudioRenderRenderFramSplit error!");
        return AUDIO_ERR_INTERNAL;
    }

    pthread_mutex_unlock(&hwRender->renderParam.frameRenderMode.mutex);
    
    *replyBytes = (uint64_t)frameLen;

    hwRender->renderParam.frameRenderMode.frames += hwRender->renderParam.frameRenderMode.bufferFrameSize;
    if (hwRender->renderParam.frameRenderMode.attrs.sampleRate == 0) {
        AUDIO_FUNC_LOGE("Divisor cannot be zero!");
        return AUDIO_ERR_INTERNAL;
    }

    if (TimeToAudioTimeStamp(hwRender->renderParam.frameRenderMode.bufferFrameSize,
        &hwRender->renderParam.frameRenderMode.time,
        hwRender->renderParam.frameRenderMode.attrs.sampleRate) == HDF_FAILURE) {
        AUDIO_FUNC_LOGE("Frame is NULL");
        return AUDIO_ERR_INTERNAL;
    }
    return AUDIO_SUCCESS;
}

int32_t AudioRenderGetRenderPosition(struct IAudioRender *render, uint64_t *frames, struct AudioTimeStamp *time)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *impl = (struct AudioHwRender *)render;
    if (impl == NULL || frames == NULL || time == NULL) {
        AUDIO_FUNC_LOGE("Paras is NULL!");
        return AUDIO_ERR_INVALID_PARAM;
    }
    *frames = impl->renderParam.frameRenderMode.frames;
    *time = impl->renderParam.frameRenderMode.time;
    return AUDIO_SUCCESS;
}

int32_t AudioRenderSetRenderSpeed(struct IAudioRender *render, float speed)
{
    AUDIO_FUNC_LOGD("Enter.");
    (void)speed;
    struct AudioHwRender *hwRender = (struct AudioHwRender *)render;
    if (hwRender == NULL) {
        AUDIO_FUNC_LOGE("hwRender is NULL!");
        return AUDIO_ERR_INVALID_PARAM;
    }
    return AUDIO_ERR_NOT_SUPPORT;
}

int32_t AudioRenderGetRenderSpeed(struct IAudioRender *render, float *speed)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *hwRender = (struct AudioHwRender *)render;
    if (hwRender == NULL || speed == NULL) {
        AUDIO_FUNC_LOGE("Param is NULL!");
        return AUDIO_ERR_INVALID_PARAM;
    }
    return AUDIO_ERR_NOT_SUPPORT;
}

int32_t AudioRenderSetChannelMode(struct IAudioRender *render, enum AudioChannelMode mode)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *impl = (struct AudioHwRender *)render;
    if (impl == NULL) {
        AUDIO_FUNC_LOGE("impl is NULL!");
        return AUDIO_ERR_INVALID_PARAM;
    }

    if (impl->devCtlHandle == NULL) {
        AUDIO_FUNC_LOGE("Bind Fail!");
        return AUDIO_ERR_INTERNAL;
    }

    enum AudioChannelMode tempMode = impl->renderParam.frameRenderMode.mode;
    impl->renderParam.frameRenderMode.mode = mode;

    InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
        impl->renderParam.frameRenderMode.mode = tempMode;
        return AUDIO_ERR_INTERNAL;
    }

    int32_t ret =
        (*pInterfaceLibModeRender)(impl->devCtlHandle, &impl->renderParam, AUDIODRV_CTL_IOCTL_CHANNEL_MODE_WRITE);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("Set ChannelMode FAIL!");
        impl->renderParam.frameRenderMode.mode = tempMode;
        return AUDIO_ERR_INTERNAL;
    }
    return AUDIO_SUCCESS;
}

int32_t AudioRenderGetChannelMode(struct IAudioRender *render, enum AudioChannelMode *mode)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *impl = (struct AudioHwRender *)render;
    if (impl == NULL || mode == NULL || impl->devCtlHandle == NULL) {
        AUDIO_FUNC_LOGE("Paras is NULL!");
        return AUDIO_ERR_INVALID_PARAM;
    }

    InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
        return AUDIO_ERR_INTERNAL;
    }

    int32_t ret =
        (*pInterfaceLibModeRender)(impl->devCtlHandle, &impl->renderParam, AUDIODRV_CTL_IOCTL_CHANNEL_MODE_READ);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("Get ChannelMode FAIL!");
        return AUDIO_ERR_INTERNAL;
    }
    *mode = impl->renderParam.frameRenderMode.mode;
    return AUDIO_SUCCESS;
}

static int32_t SetValue(struct ExtraParams mExtraParams, struct AudioHwRender *render)
{
    if (render == NULL) {
        AUDIO_FUNC_LOGE("render is NULL!");
        return HDF_FAILURE;
    }
    if (mExtraParams.route != -1) {
        render->renderParam.renderMode.hwInfo.pathroute = mExtraParams.route;
    }
    if (mExtraParams.format != -1) {
        render->renderParam.frameRenderMode.attrs.format = mExtraParams.format;
    }
    if (mExtraParams.channels != 0) {
        render->renderParam.frameRenderMode.attrs.channelCount = mExtraParams.channels;
    }
    if (mExtraParams.flag) {
        render->renderParam.frameRenderMode.frames = mExtraParams.frames;
    }
    if (mExtraParams.sampleRate != 0) {
        render->renderParam.frameRenderMode.attrs.sampleRate = mExtraParams.sampleRate;
    }
    return HDF_SUCCESS;
}

int32_t AudioRenderSetExtraParams(struct IAudioRender *handle, const char *keyValueList)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *render = (struct AudioHwRender *)handle;
    if (render == NULL || keyValueList == NULL) {
        return AUDIO_ERR_INVALID_PARAM;
    }
    int32_t count = 0;
    int32_t check = 0;
    struct ExtraParams mExtraParams;
    if (AudioSetExtraParams(keyValueList, &count, &mExtraParams, &check) < 0) {
        return AUDIO_ERR_INTERNAL;
    }
    if (count != 0 && check == count) {
        SetValue(mExtraParams, render);
        return AUDIO_SUCCESS;
    } else {
        return AUDIO_ERR_INTERNAL;
    }
}

int32_t AudioRenderGetExtraParams(struct IAudioRender *handle, char *keyValueList, uint32_t listLenth)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *render = (struct AudioHwRender *)handle;
    if (render == NULL || keyValueList == NULL || listLenth == 0) {
        return AUDIO_ERR_INVALID_PARAM;
    }

    int32_t bufferSize = strlen(ROUTE_SAMPLE) + strlen(FORMAT_SAMPLE) + strlen(CHANNELS_SAMPLE) +
        strlen(FRAME_COUNT_SAMPLE) + strlen(SAMPLING_RATE_SAMPLE) + 1;
    if (listLenth < (uint32_t)bufferSize) {
        return AUDIO_ERR_INTERNAL;
    }

    int32_t ret = AddElementToList(
        keyValueList, listLenth, AUDIO_ATTR_PARAM_ROUTE, &render->renderParam.renderMode.hwInfo.pathroute);
    if (ret < 0) {
        return AUDIO_ERR_INTERNAL;
    }

    ret = AddElementToList(
        keyValueList, listLenth, AUDIO_ATTR_PARAM_FORMAT, &render->renderParam.frameRenderMode.attrs.format);
    if (ret < 0) {
        return AUDIO_ERR_INTERNAL;
    }

    ret = AddElementToList(
        keyValueList, listLenth, AUDIO_ATTR_PARAM_CHANNELS, &render->renderParam.frameRenderMode.attrs.channelCount);
    if (ret < 0) {
        return AUDIO_ERR_INTERNAL;
    }

    ret = AddElementToList(
        keyValueList, listLenth, AUDIO_ATTR_PARAM_FRAME_COUNT, &render->renderParam.frameRenderMode.frames);
    if (ret < 0) {
        return AUDIO_ERR_INTERNAL;
    }

    ret = AddElementToList(
        keyValueList, listLenth, AUDIO_ATTR_PARAM_SAMPLING_RATE, &render->renderParam.frameRenderMode.attrs.sampleRate);
    if (ret < 0) {
        return AUDIO_ERR_INTERNAL;
    }
    return AUDIO_SUCCESS;
}

int32_t AudioRenderReqMmapBuffer(
    struct IAudioRender *handle, int32_t reqSize, struct AudioMmapBufferDescriptor *desc)
{
    (void)handle;
    (void)reqSize;
    (void)desc;
    return AUDIO_SUCCESS;
}

int32_t AudioRenderGetMmapPosition(struct IAudioRender *handle, uint64_t *frames, struct AudioTimeStamp *time)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *render = (struct AudioHwRender *)handle;
    if (render == NULL || frames == NULL || time == NULL) {
        return AUDIO_ERR_INVALID_PARAM;
    }

    InterfaceLibModeRenderPassthrough *pInterfaceLibModeRender = AudioPassthroughGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        AUDIO_FUNC_LOGE("pInterfaceLibModeRender Is NULL");
        return AUDIO_ERR_INTERNAL;
    }
    if (render->devDataHandle == NULL) {
        return AUDIO_ERR_INTERNAL;
    }

    int32_t ret =
        (*pInterfaceLibModeRender)(render->devDataHandle, &render->renderParam, AUDIO_DRV_PCM_IOCTL_MMAP_POSITION);
    if (ret < 0) {
        AUDIO_FUNC_LOGE("Get Position FAIL!");
        return AUDIO_ERR_INTERNAL;
    }

    *frames = render->renderParam.frameRenderMode.frames;

    render->renderParam.frameRenderMode.time.tvSec =
        (int64_t)(render->renderParam.frameRenderMode.frames / render->renderParam.frameRenderMode.attrs.sampleRate);

    uint64_t lastBufFrames =
        render->renderParam.frameRenderMode.frames % render->renderParam.frameRenderMode.attrs.sampleRate;

    render->renderParam.frameRenderMode.time.tvNSec =
        (int64_t)((lastBufFrames * SEC_TO_NSEC) / render->renderParam.frameRenderMode.attrs.sampleRate);

    *time = render->renderParam.frameRenderMode.time;
    return AUDIO_SUCCESS;
}

int32_t AudioRenderTurnStandbyMode(struct IAudioRender *handle)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *render = (struct AudioHwRender *)handle;
    if (render == NULL) {
        return AUDIO_ERR_INVALID_PARAM;
    }

    render->renderParam.renderMode.ctlParam.turnStandbyStatus = AUDIO_TURN_STANDBY_NOW;

    int32_t ret = AudioRenderStop((AudioHandle)render);
    if (ret < 0) {
        return AUDIO_ERR_INTERNAL;
    }
    return AUDIO_SUCCESS;
}

int32_t AudioRenderAudioDevDump(struct IAudioRender *handle, int32_t range, int32_t fd)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *render = (struct AudioHwRender *)handle;
    if (render == NULL) {
        return AUDIO_ERR_INVALID_PARAM;
    }

    dprintf(fd, "%s%d\n", "Number of errors: ", render->errorLog.totalErrors);
    if (range < RANGE_MIN - 1 || range > RANGE_MAX) {
        dprintf(fd, "%s\n", "Out of range, invalid output");
        return AUDIO_SUCCESS;
    }

    uint32_t mSize = render->errorLog.iter;
    if (range < RANGE_MIN) {
        dprintf(fd, "%-5s  %-10s  %s\n", "count", "errorCode", "Time");
        for (uint32_t i = 0; i < mSize; i++) {
            dprintf(fd, FORMAT_TWO, render->errorLog.errorDump[i].count + 1, render->errorLog.errorDump[i].errorCode,
                render->errorLog.errorDump[i].currentTime);
        }
    } else {
        dprintf(fd, "%-5s  %-10s  %-20s  %-15s  %s\n", "count", "errorCode", "frames", "fail reason", "Time");
        for (uint32_t i = 0; i < mSize; i++) {
            dprintf(fd, FORMAT_ONE, render->errorLog.errorDump[i].count + 1, render->errorLog.errorDump[i].errorCode,
                render->errorLog.errorDump[i].frames, render->errorLog.errorDump[i].reason,
                render->errorLog.errorDump[i].currentTime);
        }
    }
    return AUDIO_SUCCESS;
}
int32_t CallbackProcessing(AudioHandle handle, enum AudioCallbackType callBackType)
{
    struct AudioHwRender *render = (struct AudioHwRender *)handle;
    if (render == NULL) {
        AUDIO_FUNC_LOGI("Unregistered callback.\n");
        return HDF_FAILURE;
    }
    if (render->renderParam.frameRenderMode.callback.RenderCallback == NULL) {
        return HDF_FAILURE;
    }

    bool isCallBack = false;
    switch (callBackType) {
        case AUDIO_NONBLOCK_WRITE_COMPLETED:
        case AUDIO_DRAIN_COMPLETED:
        case AUDIO_FLUSH_COMPLETED:
        case AUDIO_RENDER_FULL:
        case AUDIO_ERROR_OCCUR:
            isCallBack = true;
            break;
        default:
            break;
    }

    if (!isCallBack) {
        AUDIO_FUNC_LOGI("No callback processing is required.\n");
        return HDF_ERR_NOT_SUPPORT;
    }

    render->renderParam.frameRenderMode.callback.RenderCallback(
        &render->renderParam.frameRenderMode.callback, callBackType, NULL, render->renderParam.frameRenderMode.cookie);
    return HDF_SUCCESS;
}

int32_t AudioRenderRegCallback(struct IAudioRender *render, struct IAudioCallback *audioCallback, int8_t cookie)
{
    AUDIO_FUNC_LOGD("Enter.");
    if (audioCallback == NULL || audioCallback->RenderCallback == NULL) {
        return AUDIO_ERR_INVALID_PARAM;
    }

    struct AudioHwRender *pRender = (struct AudioHwRender *)render;
    if (pRender == NULL) {
        return AUDIO_ERR_INVALID_PARAM;
    }

    pRender->renderParam.frameRenderMode.callback.RenderCallback = audioCallback->RenderCallback;
    pRender->renderParam.frameRenderMode.cookie = (void *)(uintptr_t)cookie;
    pRender->renderParam.renderMode.hwInfo.callBackEnable = true;
    return AUDIO_SUCCESS;
}

int32_t AudioRenderDrainBuffer(struct IAudioRender *render, enum AudioDrainNotifyType *type)
{
    AUDIO_FUNC_LOGD("Enter.");
    struct AudioHwRender *pRender = (struct AudioHwRender *)render;
    if (pRender == NULL || type == NULL) {
        return AUDIO_ERR_INVALID_PARAM;
    }
    return AUDIO_ERR_NOT_SUPPORT;
}

void AudioRenderRelease(struct IAudioRender *instance)
{
    (void)instance;
}
