/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef DISTRIBUTED_CAMERA_DEVICE_H
#define DISTRIBUTED_CAMERA_DEVICE_H

#include <atomic>
#include <vector>
#include <string>
#include <sstream>
#include "dmetadata_processor.h"
#include "dstream_operator.h"

#include "v1_0/icamera_device.h"
#include "v1_3/icamera_device.h"
#include "v1_0/icamera_device_callback.h"
#include "v1_1/id_camera_provider_callback.h"
#include "v1_0/types.h"
#include "v1_1/types.h"
#include "v1_0/istream_operator_callback.h"
#include "v1_2/istream_operator_callback.h"
#include "v1_3/istream_operator_callback.h"
#include "v1_1/istream_operator.h"

namespace OHOS {
namespace DistributedHardware {
using HDI::Camera::V1_2::IStreamOperator;
using HDI::Camera::V1_0::ICameraDeviceCallback;
class DCameraDevice : public HDI::Camera::V1_3::ICameraDevice {
public:
    DCameraDevice(const DHBase &dhBase, const std::string& sinkAbilityInfo, const std::string& sourceCodecInfo);
    DCameraDevice() = default;
    virtual ~DCameraDevice() = default;
    DCameraDevice(const DCameraDevice &other) = delete;
    DCameraDevice(DCameraDevice &&other) = delete;
    DCameraDevice& operator=(const DCameraDevice &other) = delete;
    DCameraDevice& operator=(DCameraDevice &&other) = delete;

public:
    int32_t GetStreamOperator_V1_3(const sptr<HDI::Camera::V1_3::IStreamOperatorCallback> &callbackObj,
        sptr<IStreamOperator> &streamOperator) override;
    int32_t GetSecureCameraSeq(uint64_t &seqId) override;

    int32_t GetStreamOperator_V1_2(const sptr<HDI::Camera::V1_2::IStreamOperatorCallback> &callbackObj,
        sptr<IStreamOperator> &streamOperator) override;
    int32_t GetStatus(const std::vector<uint8_t> &metaIn, std::vector<uint8_t> &metaOut) override;
    int32_t Reset() override;

    int32_t GetStreamOperator_V1_1(const sptr<HDI::Camera::V1_0::IStreamOperatorCallback> &callbackObj,
        sptr<HDI::Camera::V1_1::IStreamOperator> &streamOperator) override;
    int32_t GetDefaultSettings(std::vector<uint8_t> &settings) override;

    int32_t GetStreamOperator(const sptr<HDI::Camera::V1_0::IStreamOperatorCallback> &callbackObj,
        sptr<HDI::Camera::V1_0::IStreamOperator> &streamOperator) override;
    int32_t UpdateSettings(const std::vector<uint8_t> &settings) override;
    int32_t SetResultMode(ResultCallbackMode mode) override;
    int32_t GetEnabledResults(std::vector<int32_t> &results) override;
    int32_t EnableResult(const std::vector<int32_t> &results) override;
    int32_t DisableResult(const std::vector<int32_t> &results) override;
    int32_t Close() override;

    int32_t GetSettings(std::vector<uint8_t> &settings);
    CamRetCode OpenDCamera(const OHOS::sptr<ICameraDeviceCallback> &callback);
    CamRetCode GetDCameraAbility(std::shared_ptr<CameraAbility> &ability);
    DCamRetCode AcquireBuffer(int streamId, DCameraBuffer &buffer);
    DCamRetCode ShutterBuffer(int streamId, const DCameraBuffer &buffer);
    DCamRetCode OnSettingsResult(const std::shared_ptr<DCameraSettings> &result);
    DCamRetCode Notify(const std::shared_ptr<DCameraHDFEvent> &event);
    void SetProviderCallback(const OHOS::sptr<IDCameraProviderCallback> &callback);
    OHOS::sptr<IDCameraProviderCallback> GetProviderCallback();
    std::string GetDCameraId();
    bool IsOpened();
    void SetDcameraAbility(const std::string& sinkAbilityInfo);

private:
    void Init(const std::string &sinkAbilityInfo);
    DCamRetCode CreateDStreamOperator();
    std::string GenerateCameraId(const DHBase &dhBase);
    void NotifyStartCaptureError();
    void NotifyCameraError(const ErrorType type);
    void IsOpenSessFailedState(bool state);
    CamRetCode TriggerGetFullCaps();
    void SetRefreshFlag(bool flag);
    bool GetRefreshFlag();
private:
    bool isOpened_;
    std::string dCameraId_;
    DHBase dhBase_;
    std::string dCameraAbilityInfo_;
    std::string sourceCodecInfo_;
    OHOS::sptr<ICameraDeviceCallback> dCameraDeviceCallback_;
    OHOS::sptr<IDCameraProviderCallback> dCameraProviderCallback_;
    OHOS::sptr<DStreamOperator> dCameraStreamOperator_;
    std::shared_ptr<DMetadataProcessor> dMetadataProcessor_;

    std::mutex openSesslock_;
    std::condition_variable openSessCV_;
    bool isOpenSessFailed_ = false;
    std::mutex isOpenSessFailedlock_;
    std::mutex getFullLock_;
    std::condition_variable getFullWaitCond_;
    std::atomic<bool> refreshFlag_ = false;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // DISTRIBUTED_CAMERA_DEVICE_H
