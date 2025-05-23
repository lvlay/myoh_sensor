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

#ifndef BLUETOOTH_A2DP_SRC_OBSERVER_H
#define BLUETOOTH_A2DP_SRC_OBSERVER_H

#include "bluetooth_a2dp_audio_src_observer_stub.h"
#include "audio_bluetooth_manager.h"

class BluetoothA2dpSrcObserver : public OHOS::Bluetooth::BluetoothA2dpAudioSrcObserverStub {
public:
    BluetoothA2dpSrcObserver(OHOS::Bluetooth::BtA2dpAudioCallback *callbacks) : callbacks_(callbacks) {};
    ~BluetoothA2dpSrcObserver() {};

    void OnConnectionStateChanged(const OHOS::bluetooth::RawAddress &device, int state, int cause);
    void OnPlayingStatusChanged(const OHOS::bluetooth::RawAddress &device, int playingState, int error);
    void OnConfigurationChanged
        (const OHOS::bluetooth::RawAddress &device, const OHOS::Bluetooth::BluetoothA2dpCodecInfo &info, int error);
    void OnMediaStackChanged(const OHOS::bluetooth::RawAddress &device, int action);

private:
    OHOS::Bluetooth::BtA2dpAudioCallback *callbacks_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothA2dpSrcObserver);
};

#endif