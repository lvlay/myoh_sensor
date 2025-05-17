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

#include "audiointerfacelibctlcapturecmdid_fuzzer.h"
#include "audio_adm_fuzzer_common.h"

using namespace OHOS::Audio;
namespace OHOS {
namespace Audio {
    bool AudioInterfacelibctlcaptureCmdidFuzzTest(const uint8_t *data, size_t size)
    {
        bool result = false;
        char resolvedPath[] = HDF_LIBRARY_FULL_PATH("libaudio_capture_adapter");
        void *ctlcapFuzzPtrHandle = dlopen(resolvedPath, RTLD_LAZY);
        if (ctlcapFuzzPtrHandle == nullptr) {
            HDF_LOGE("%{public}s: dlopen failed\n", __func__);
            return false;
        }
        struct DevHandle *(*AudioBindService)(const char *) = nullptr;
        int32_t (*InterfaceLibCtlCapture)(struct DevHandle *, int, struct AudioHwCaptureParam *) = nullptr;
        AudioBindService = reinterpret_cast<struct DevHandle *(*)(const char *)>
           (dlsym(ctlcapFuzzPtrHandle, "AudioBindService"));
        if (AudioBindService == nullptr) {
            HDF_LOGE("%{public}s: dlsym AudioBindService failed \n", __func__);
            dlclose(ctlcapFuzzPtrHandle);
            return false;
        }
        struct DevHandle *handle = AudioBindService(BIND_CONTROL.c_str());
        if (handle == nullptr) {
            HDF_LOGE("%{public}s: AudioBindService failed \n", __func__);
            dlclose(ctlcapFuzzPtrHandle);
            return false;
        }
        InterfaceLibCtlCapture = reinterpret_cast<int32_t (*)(struct DevHandle *, int,
            struct AudioHwCaptureParam *)>(dlsym(ctlcapFuzzPtrHandle, "AudioInterfaceLibCtlCapture"));
        if (InterfaceLibCtlCapture == nullptr) {
            HDF_LOGE("%{public}s: dlsym AudioInterfaceLibCtlCapture failed \n", __func__);
            dlclose(ctlcapFuzzPtrHandle);
            return false;
        }
        struct AudioHwCapture *hwCapture = nullptr;
        (void)BindServiceAndHwCapture(hwCapture);
        int32_t cmdId = *(reinterpret_cast<int32_t *>(const_cast<uint8_t *>(data)));
        int32_t ret = InterfaceLibCtlCapture(handle, cmdId, &hwCapture->captureParam);
        if (ret == HDF_SUCCESS) {
            result = true;
        }
        free(hwCapture);
        void (*CloseService)(const struct DevHandle *) = nullptr;
        CloseService = reinterpret_cast<void (*)(const struct DevHandle *)>(dlsym(ctlcapFuzzPtrHandle,
            "AudioCloseServiceCapture"));
        if (CloseService == nullptr) {
            HDF_LOGE("%{public}s: dlsym AudioCloseServiceCapture failed \n", __func__);
            dlclose(ctlcapFuzzPtrHandle);
            return false;
        }
        CloseService(handle);
        dlclose(ctlcapFuzzPtrHandle);
        return result;
    }
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::Audio::AudioInterfacelibctlcaptureCmdidFuzzTest(data, size);
    return 0;
}
