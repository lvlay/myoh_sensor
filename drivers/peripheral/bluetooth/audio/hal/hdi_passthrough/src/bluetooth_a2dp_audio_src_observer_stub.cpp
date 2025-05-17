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
#include <hdf_log.h>
#include "raw_address.h"
#include "bluetooth_a2dp_audio_src_observer_stub.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
BluetoothA2dpAudioSrcObserverStub::BluetoothA2dpAudioSrcObserverStub()
{
    HDF_LOGI("%{public}s start.", __func__);
    funcMap_[static_cast<uint32_t>(IBluetoothA2dpSourceObserver::Code::BT_A2DP_SRC_OBSERVER_CONNECTION_STATE_CHANGED)] =
        &BluetoothA2dpAudioSrcObserverStub::OnConnectionStateChangedInner;
    funcMap_[static_cast<uint32_t>(IBluetoothA2dpSourceObserver::Code::BT_A2DP_SRC_OBSERVER_PLAYING_STATUS_CHANGED)] =
        &BluetoothA2dpAudioSrcObserverStub::OnPlayingStatusChangedInner;
    funcMap_[static_cast<uint32_t>(IBluetoothA2dpSourceObserver::Code::BT_A2DP_SRC_OBSERVER_CONFIGURATION_CHANGED)] =
        &BluetoothA2dpAudioSrcObserverStub::OnConfigurationChangedInner;
    funcMap_[static_cast<uint32_t>(IBluetoothA2dpSourceObserver::Code::BT_A2DP_SRC_OBSERVER_MEDIASTACK_CHANGED)] =
        &BluetoothA2dpAudioSrcObserverStub::OnMediaStackChangedInner;
}

BluetoothA2dpAudioSrcObserverStub::~BluetoothA2dpAudioSrcObserverStub()
{
    HDF_LOGI("%{public}s start.", __func__);
    funcMap_.clear();
}

int BluetoothA2dpAudioSrcObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HDF_LOGI("BluetoothA2dpAudioSrcObserverStub::OnRemoteRequest, cmd=%{public}d, flags= %d", code, option.GetFlags());
    std::u16string descriptor = BluetoothA2dpAudioSrcObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HDF_LOGE("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }
    auto itFunc = funcMap_.find(code);
    if (itFunc != funcMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    HDF_LOGI("BluetoothA2dpAudioSrcObserverStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

ErrCode BluetoothA2dpAudioSrcObserverStub::OnConnectionStateChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int state = data.ReadInt32();
    int cause = data.ReadInt32();
    HDF_LOGI("BluetoothA2dpAudioSrcObserverStub::OnConnectionStateChangedInner");
    OnConnectionStateChanged(RawAddress(addr), state, cause);

    return NO_ERROR;
}

ErrCode BluetoothA2dpAudioSrcObserverStub::OnPlayingStatusChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int playingState = data.ReadInt32();
    int error = data.ReadInt32();
    HDF_LOGI("BluetoothA2dpAudioSrcObserverStub::OnPlayingStatusChangedInner");
    OnPlayingStatusChanged(RawAddress(addr), playingState, error);

    return NO_ERROR;
}

ErrCode BluetoothA2dpAudioSrcObserverStub::OnConfigurationChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    std::shared_ptr<BluetoothA2dpCodecInfo> info(data.ReadParcelable<BluetoothA2dpCodecInfo>());
    if (info == nullptr) {
        HDF_LOGE("Read a2dp code info failed");
        return TRANSACTION_ERR;
    }
    int error = data.ReadInt32();
    HDF_LOGI("BluetoothA2dpAudioSrcObserverStub::OnConfigurationChangedInner");
    OnConfigurationChanged(RawAddress(addr), *info, error);

    return NO_ERROR;
}

int32_t BluetoothA2dpAudioSrcObserverStub::OnMediaStackChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int action = data.ReadInt32();
    HDF_LOGI("BluetoothA2dpAudioSrcObserverStub::OnMediaStackChangedInner");
    OnMediaStackChanged(RawAddress(addr), action);

    return NO_ERROR;
}
}
}