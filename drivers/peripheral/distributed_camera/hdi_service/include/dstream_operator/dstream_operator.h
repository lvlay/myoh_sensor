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

#ifndef DISTRIBUTED_CAMERA_STREAM_OPERATOR_H
#define DISTRIBUTED_CAMERA_STREAM_OPERATOR_H

#include <map>
#include <set>
#include <vector>

#include "constants.h"
#include "dcamera.h"
#include "dcamera_stream.h"
#include "dmetadata_processor.h"

#include "cJSON.h"
#include "v1_0/istream_operator.h"
#include "v1_1/istream_operator.h"
#include "v1_2/istream_operator.h"
#include "v1_0/types.h"
#include "v1_1/types.h"

namespace OHOS {
namespace DistributedHardware {
using namespace std;
using namespace OHOS::HDI::Camera::V1_0;
using HDI::Camera::V1_1::OperationMode_V1_1;
using HDI::Camera::V1_1::StreamInfo_V1_1;
class DCameraProvider;
class DStreamOperator : public HDI::Camera::V1_2::IStreamOperator {
public:
    explicit DStreamOperator(std::shared_ptr<DMetadataProcessor> &dMetadataProcessor);
    DStreamOperator() = default;
    ~DStreamOperator() override = default;
    DStreamOperator(const DStreamOperator &other) = delete;
    DStreamOperator(DStreamOperator &&other) = delete;
    DStreamOperator& operator=(const DStreamOperator &other) = delete;
    DStreamOperator& operator=(DStreamOperator &&other) = delete;

public:
    int32_t UpdateStreams(const std::vector<StreamInfo_V1_1> &streamInfos) override;
    int32_t ConfirmCapture(int32_t cId) override;

    int32_t IsStreamsSupported_V1_1(OperationMode_V1_1 mode, const std::vector<uint8_t> &modeSetting,
        const std::vector<StreamInfo_V1_1> &infos, StreamSupportType &type) override;
    int32_t CreateStreams_V1_1(const std::vector<StreamInfo_V1_1> &streamInfos) override;
    int32_t CommitStreams_V1_1(OperationMode_V1_1 mode, const std::vector<uint8_t> &modeSetting) override;

    int32_t IsStreamsSupported(OperationMode mode, const std::vector<uint8_t> &modeSetting,
        const std::vector<StreamInfo> &infos, StreamSupportType &type) override;
    int32_t CreateStreams(const std::vector<StreamInfo> &streamInfos) override;
    int32_t ReleaseStreams(const std::vector<int32_t> &streamIds) override;
    int32_t CommitStreams(OperationMode mode, const std::vector<uint8_t> &modeSetting) override;
    int32_t GetStreamAttributes(std::vector<StreamAttribute> &attributes) override;
    int32_t AttachBufferQueue(int32_t streamId, const sptr<BufferProducerSequenceable> &bufferProducer) override;
    int32_t DetachBufferQueue(int32_t streamId) override;
    int32_t Capture(int32_t captureId, const CaptureInfo &info, bool isStreaming) override;
    int32_t CancelCapture(int32_t captureId) override;
    int32_t ChangeToOfflineStream(const std::vector<int32_t> &streamIds,
        const sptr<IStreamOperatorCallback> &callbackObj, sptr<IOfflineStreamOperator> &offlineOperator) override;

    DCamRetCode InitOutputConfigurations(const DHBase &dhBase, const std::string &sinkAbilityInfo,
        const std::string &sourceCodecInfo);
    std::vector<DCEncodeType> ParseEncoderTypes(cJSON* rootValue);
    DCamRetCode ParsePhotoFormats(cJSON* rootValue);
    DCamRetCode ParsePreviewFormats(cJSON* rootValue);
    DCamRetCode ParseVideoFormats(cJSON* rootValue);
    DCamRetCode AcquireBuffer(int streamId, DCameraBuffer &buffer);
    DCamRetCode ShutterBuffer(int streamId, const DCameraBuffer &buffer);
    DCamRetCode SetCallBack(OHOS::sptr<IStreamOperatorCallback> const &callback);
    DCamRetCode SetDeviceCallback(function<void(ErrorType, int)> &errorCbk,
        function<void(uint64_t, std::shared_ptr<OHOS::Camera::CameraMetadata>)> &resultCbk);
    void Release();
    std::vector<int> GetStreamIds();
    DCamRetCode SetOutputVal(const DHBase &dhBase, const std::string &sinkAbilityInfo,
        const std::string &sourceCodecInfo);

private:
    bool IsCapturing();
    void SetCapturing(bool isCapturing);
    DCamRetCode NegotiateSuitableCaptureInfo(const CaptureInfo& srcCaptureInfo, bool isStreaming);
    void ChooseSuitableFormat(std::vector<std::shared_ptr<DCStreamInfo>> &streamInfo,
        std::shared_ptr<DCCaptureInfo> &captureInfo);
    void ChooseSuitableResolution(std::vector<std::shared_ptr<DCStreamInfo>> &streamInfo,
        std::shared_ptr<DCCaptureInfo> &captureInfo);
    void ChooseSuitableDataSpace(std::vector<std::shared_ptr<DCStreamInfo>> &streamInfo,
        std::shared_ptr<DCCaptureInfo> &captureInfo);
    void ChooseSuitableEncodeType(std::vector<std::shared_ptr<DCStreamInfo>> &streamInfo,
        std::shared_ptr<DCCaptureInfo> &captureInfo);
    void ChooseSuitableStreamId(std::shared_ptr<DCCaptureInfo> &captureInfo);
    void ConvertStreamInfo(const StreamInfo &srcInfo, std::shared_ptr<DCStreamInfo> &dstInfo);
    DCEncodeType ConvertDCEncodeType(std::string &srcEncodeType);
    std::shared_ptr<DCCaptureInfo> BuildSuitableCaptureInfo(const CaptureInfo& srcCaptureInfo,
        std::vector<std::shared_ptr<DCStreamInfo>> &srcStreamInfo);
    void SnapShotStreamOnCaptureEnded(int32_t captureId, int streamId);
    bool HasContinuousCaptureInfo(int captureId);
    int32_t ExtractStreamInfo(std::vector<DCStreamInfo>& dCameraStreams);
    void ExtractCaptureInfo(std::vector<DCCaptureInfo> &captureInfos);
    void ExtractCameraAttr(cJSON* rootValue, std::vector<int>& formats, const std::string rootNode);
    void GetCameraAttr(cJSON* rootValue, std::string formatStr, const std::string rootNode, int format);
    cJSON* GetFormatObj(const std::string rootNode, cJSON* rootValue, std::string& formatStr);
    DCamRetCode GetInputCaptureInfo(const CaptureInfo& srcCaptureInfo, bool isStreaming,
        std::shared_ptr<DCCaptureInfo>& inputCaptureInfo);
    void AppendCaptureInfo(std::shared_ptr<DCCaptureInfo> &appendCaptureInfo, bool isStreaming,
        std::shared_ptr<DCCaptureInfo> &inputCaptureInfo, const CaptureInfo& srcCaptureInfo);
    int32_t HalStreamCommit(const DCStreamInfo &streamInfo);

    std::shared_ptr<DCameraStream> FindHalStreamById(int32_t streamId);
    void InsertHalStream(int32_t streamId, std::shared_ptr<DCameraStream>& dcStream);
    void EraseHalStream(int32_t streamId);

    std::shared_ptr<CaptureInfo> FindCaptureInfoById(int32_t captureId);
    void InsertCaptureInfo(int captureId, std::shared_ptr<CaptureInfo>& captureInfo);
    void EraseCaptureInfo(int32_t captureId);
    int32_t FindCaptureIdByStreamId(int32_t streamId);

    std::shared_ptr<DCStreamInfo> FindDCStreamById(int32_t streamId);
    void InsertDCStream(int32_t streamId, std::shared_ptr<DCStreamInfo>& dcStreamInfo);
    void EraseDCStream(int32_t streamId);
    void ExtractNotCaptureStream(bool isStreaming, std::vector<std::shared_ptr<DCStreamInfo>>& appendStreamInfo);

    bool FindEnableShutter(int32_t streamId);
    void InsertEnableShutter(int32_t streamId, bool enableShutterCallback);
    void EraseEnableShutter(int32_t streamId);

    int32_t FindStreamCaptureBufferNum(const pair<int, int>& streamPair);
    void AddStreamCaptureBufferNum(const pair<int, int>& streamPair);
    void EraseStreamCaptureBufferNum(const pair<int, int>& streamPair);

    bool IsStreamInfosInvalid(const std::vector<StreamInfo> &infos);
    bool IsCaptureInfoInvalid(const CaptureInfo &info);

    int32_t DoCapture(int32_t captureId, const CaptureInfo &info, bool isStreaming);
    void InsertNotifyCaptureMap(int32_t captureId);
    void EraseNotifyCaptureMap(int32_t captureId);
    bool CheckInputInfo();

private:
    constexpr static uint32_t JSON_ARRAY_MAX_SIZE = 1000;
    std::shared_ptr<DMetadataProcessor> dMetadataProcessor_;
    OHOS::sptr<IStreamOperatorCallback> dcStreamOperatorCallback_;
    function<void(ErrorType, int)> errorCallback_;

    DHBase dhBase_;
    std::string sinkAbilityInfo_;
    std::string sourceCodecInfo_;
    std::vector<DCEncodeType> dcSupportedCodecType_;
    std::vector<DCEncodeType> sourceEncodeTypes_;

    std::map<DCSceneType, std::vector<int>> dcSupportedFormatMap_;
    std::map<int, std::vector<DCResolution>> dcSupportedPhotoResolutionMap_;
    std::map<int, std::vector<DCResolution>> dcSupportedPreviewResolutionMap_;
    std::map<int, std::vector<DCResolution>> dcSupportedVideoResolutionMap_;

    std::map<int, std::shared_ptr<DCameraStream>> halStreamMap_;
    std::map<int, std::shared_ptr<DCStreamInfo>> dcStreamInfoMap_;
    std::map<int, std::shared_ptr<CaptureInfo>> halCaptureInfoMap_;
    std::vector<std::shared_ptr<DCCaptureInfo>> cachedDCaptureInfoList_;
    std::map<int, bool> enableShutterCbkMap_;
    std::map<pair<int, int>, int> acceptedBufferNum_;

    std::mutex streamAttrLock_;
    std::mutex halStreamLock_;
    bool isCapturing_ = false;
    std::mutex isCapturingLock_;
    OperationMode_V1_1 currentOperMode_ = OperationMode_V1_1::NORMAL;
    std::shared_ptr<OHOS::Camera::CameraMetadata> latestStreamSetting_;
    std::map<int, bool> notifyCaptureStartedMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // DISTRIBUTED_CAMERA_STREAM_OPERATOR_H
