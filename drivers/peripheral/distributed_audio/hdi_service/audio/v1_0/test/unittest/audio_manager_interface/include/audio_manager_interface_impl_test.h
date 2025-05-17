/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_AUDIO_MANAGER_INTERFACE_IMPL_TEST_H
#define OHOS_AUDIO_MANAGER_INTERFACE_IMPL_TEST_H

#include <gtest/gtest.h>

#include "hdf_device_desc.h"
#include <v1_0/iaudio_manager.h>
#include <v2_0/id_audio_manager.h>

#include "audio_adapter_interface_impl.h"
#include "audio_test_utils.h"

#define private public
#include "audio_manager_interface_impl.h"
#undef private

namespace OHOS {
namespace HDI {
namespace DistributedAudio {
namespace Audio {
namespace V1_0 {
using OHOS::HDI::DistributedAudio::Audioext::V2_0::DAudioEvent;
using OHOS::HDI::DistributedAudio::Audioext::V2_0::IDAudioCallback;

class AudioManagerInterfaceImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AudioManagerInterfaceImpl> audioManagerInterfaceImpl_;
};
} // V1_0
} // Audio
} // Distributedaudio
} // HDI
} // OHOS
#endif // OHOS_AUDIO_MANAGER_INTERFACE_IMPL_TEST_H