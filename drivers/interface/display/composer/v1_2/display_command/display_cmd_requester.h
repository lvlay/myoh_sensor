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

#ifndef OHOS_HDI_DISPLAY_V1_2_DISPLAY_CMD_REQUESTER_H
#define OHOS_HDI_DISPLAY_V1_2_DISPLAY_CMD_REQUESTER_H

#include "v1_1/display_command/display_cmd_requester.h"
#include "v1_2/display_command/display_cmd_utils.h"
#include "v1_2/display_composer_type.h"
#include "v1_2/idisplay_composer.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {
namespace V1_2 {
using namespace OHOS::HDI::Base;

template <typename Transfer, typename CompHdi>
class DisplayCmdRequester : public V1_1::DisplayCmdRequester<Transfer, CompHdi> {
public:
    DisplayCmdRequester(sptr<CompHdi> hdi) : BaseType1_1(hdi), hdi_1_2_(hdi) {}

    static std::unique_ptr<DisplayCmdRequester> Create(sptr<CompHdi> hdi)
    {
        DISPLAY_CHK_RETURN(hdi == nullptr, nullptr, HDF_LOGE("%{public}s: hdi is nullptr", __func__));
        auto requester = std::make_unique<DisplayCmdRequester>(hdi);
        DISPLAY_CHK_RETURN(requester == nullptr, nullptr,
            HDF_LOGE("%{public}s: CmdRequester is nullptr", __func__));
        auto ret = requester->Init(CmdUtils::INIT_ELEMENT_COUNT);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("DisplayCmdRequester init failed");
            return nullptr;
        }
        return requester;
    }

    int32_t CommitAndGetReleaseFence(uint32_t devId, int32_t& fence, bool isSupportSkipValidate, int32_t& skipState,
        bool& needFlush, std::vector<uint32_t>& layers, std::vector<int32_t>& fences, bool isValidated)
    {
        uint32_t replyEleCnt = 0;
        std::vector<HdifdInfo> outFds;
        std::shared_ptr<char> replyData;

        int32_t ret = CmdUtils::StartSection(REQUEST_CMD_COMMIT_AND_GET_RELEASE_FENCE, requestPacker_);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = requestPacker_.WriteUint32(devId) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = requestPacker_.WriteBool(isSupportSkipValidate) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = requestPacker_.WriteBool(isValidated) ? HDF_SUCCESS : HDF_FAILURE;
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = CmdUtils::EndSection(requestPacker_);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = CmdUtils::EndPack(requestPacker_);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = DoRequest(replyEleCnt, outFds, replyData);
        DISPLAY_CHECK(ret != HDF_SUCCESS, goto EXIT);

        ret = DoReplyResults(replyEleCnt, outFds, replyData, [&](void *data) -> int32_t {
            FenceData *fenceData = reinterpret_cast<struct FenceData *>(data);
            if (fenceData == nullptr) {
                fence = -1;
                skipState = -1;
                needFlush = false;
                layers.clear();
                fences.clear();
                return HDF_FAILURE;
            }
            fence = fenceData->fence_;
            skipState = fenceData->skipValidateState_;
            needFlush = fenceData->needFlush_;
            layers = fenceData->layers;
            fences = fenceData->fences;
            return HDF_SUCCESS;
        });
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("DoReplyResults failure, ret=%{public}d", ret);
        }

EXIT:
        return PeriodDataReset() == HDF_SUCCESS ? ret : HDF_FAILURE;
    }

    int32_t OnReplySkipStateFailed(CommandDataUnpacker& replyUnpacker, bool &needFlush)
    {
        uint32_t devId = 0;
        int32_t retBool = replyUnpacker.ReadUint32(devId);
        DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE, HDF_LOGE("%{public}s: read devId failed", __func__));

        retBool = replyUnpacker.ReadBool(needFlush);
        DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE, HDF_LOGE("%{public}s: read needFlush failed", __func__));

        // unpack layers vector
        uint32_t vectSize = 0;
        retBool = replyUnpacker.ReadUint32(vectSize);
        DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE,
            HDF_LOGE("%{public}s: HDI 1.2 read vect size failed", __func__));

        compChangeLayers_[devId].resize(vectSize);
        for (uint32_t i = 0; i < vectSize; i++) {
            DISPLAY_CHK_RETURN(replyUnpacker.ReadUint32(compChangeLayers_[devId][i]) == false, HDF_FAILURE,
                HDF_LOGE("%{public}s: HDI 1.2 read layer vector failed", __func__));
        }
        // unpack types vector
        vectSize = 0;
        retBool = replyUnpacker.ReadUint32(vectSize);
        DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE,
            HDF_LOGE("%{public}s: HDI 1.2 read vect size failed", __func__));

        compChangeTypes_[devId].resize(vectSize);
        for (uint32_t i = 0; i < vectSize; i++) {
            DISPLAY_CHK_RETURN(replyUnpacker.ReadInt32(compChangeTypes_[devId][i]) == false, HDF_FAILURE,
                HDF_LOGE("%{public}s: HDI 1.2 read composition type vector failed", __func__));
        }
        return HDF_SUCCESS;
    }

    int32_t OnReplyCommitAndGetReleaseFence(CommandDataUnpacker& replyUnpacker,
        std::vector<HdifdInfo>& replyFds, int32_t& fenceFd, int32_t& skipState,
        bool& needFlush, std::vector<uint32_t>& layers, std::vector<int32_t>& fences)
    {
        int32_t ret = CmdUtils::FileDescriptorUnpack(replyUnpacker, replyFds, fenceFd);
        DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, ret,
            HDF_LOGE("%{public}s: FileDescriptorUnpack failed", __func__));

        int32_t retBool = replyUnpacker.ReadInt32(skipState);
        DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE,
            HDF_LOGE("%{public}s: read skipValidateState failed", __func__));

        if (skipState != HDF_SUCCESS) {
            return OnReplySkipStateFailed(replyUnpacker, needFlush);
        } else {
            // unpack layers vector
            uint32_t vectSize = 0;
            DISPLAY_CHK_RETURN(true != replyUnpacker.ReadUint32(vectSize), HDF_FAILURE,
                HDF_LOGE("%{public}s: HDI 1.2 read vect size failed", __func__));

            layers.resize(vectSize);
            for (uint32_t i = 0; i < vectSize; i++) {
                DISPLAY_CHK_RETURN(replyUnpacker.ReadUint32(layers[i]) == false, HDF_FAILURE,
                    HDF_LOGE("%{public}s: HDI 1.2 read layer vector failed", __func__));
            }

            // unpack fences vector
            vectSize = 0;
            DISPLAY_CHK_RETURN(true != replyUnpacker.ReadUint32(vectSize), HDF_FAILURE,
                HDF_LOGE("%{public}s: HDI 1.2 read vect size failed", __func__));

            fences.resize(vectSize);
            for (uint32_t i = 0; i < vectSize; i++) {
                ret = CmdUtils::FileDescriptorUnpack(replyUnpacker, replyFds, fences[i]);
                DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, ret,
                    HDF_LOGE("%{public}s: HDI 1.2 FileDescriptorUnpack failed", __func__));
            }
        }

        return HDF_SUCCESS;
    }

    int32_t ProcessUnpackCmd(CommandDataUnpacker& replyUnpacker, int32_t unpackCmd,
        std::vector<HdifdInfo>& replyFds, std::function<int32_t(void *)> fn)
    {
        int32_t ret = HDF_SUCCESS;
        while (replyUnpacker.NextSection()) {
            bool retBool = replyUnpacker.BeginSection(unpackCmd);
            DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE,
                HDF_LOGE("%{public}s: BeginSection failed", __func__));

            FenceData fenceData;

            std::unordered_map<int32_t, int32_t> errMaps;
            switch (unpackCmd) {
                case REPLY_CMD_COMMIT_AND_GET_RELEASE_FENCE:
                    ret = OnReplyCommitAndGetReleaseFence(replyUnpacker, replyFds, fenceData.fence_,
                        fenceData.skipValidateState_, fenceData.needFlush_, fenceData.layers, fenceData.fences);
                    DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, ret,
                        HDF_LOGE("%{public}s: OnReplyCommit failed unpackCmd=%{public}s",
                        __func__, CmdUtils::CommandToString(unpackCmd)));

                    ret = fn(&fenceData);
                    DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, ret,
                        HDF_LOGE("%{public}s: return fence fd error, unpackCmd=%{public}s",
                        __func__, CmdUtils::CommandToString(unpackCmd)));
                    break;
                default:
                    HDF_LOGE("Unpack command failure");
                    return HDF_FAILURE;
            }
        }
        return HDF_SUCCESS;
    }

    int32_t DoReplyResults(uint32_t replyEleCnt, std::vector<HdifdInfo>& replyFds, std::shared_ptr<char> replyData,
        std::function<int32_t(void *)> fn)
    {
        CommandDataUnpacker replyUnpacker;
        replyUnpacker.Init(replyData.get(), replyEleCnt << CmdUtils::MOVE_SIZE);
#ifdef DEBUG_DISPLAY_CMD_RAW_DATA
        replyUnpacker.Dump();
#endif // DEBUG_DISPLAY_CMD_RAW_DATA
        int32_t unpackCmd = -1;
        bool retBool = replyUnpacker.PackBegin(unpackCmd);
        DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE,
            HDF_LOGE("%{public}s: PackBegin failed", __func__));
        DISPLAY_CHK_RETURN(unpackCmd != CONTROL_CMD_REPLY_BEGIN, HDF_FAILURE,
            HDF_LOGE("%{public}s: PackBegin cmd not match, unpackCmd=%{public}d", __func__, unpackCmd));
        if (ProcessUnpackCmd(replyUnpacker, unpackCmd, replyFds, fn) != HDF_SUCCESS) {
            return HDF_FAILURE;
        }

        retBool = replyUnpacker.PackEnd(unpackCmd);
        DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE,
            HDF_LOGE("%{public}s: PackEnd failed", __func__));

        DISPLAY_CHK_RETURN(unpackCmd != CONTROL_CMD_REPLY_END, HDF_FAILURE,
            HDF_LOGE("%{public}s: PackEnd failed, endCmd = %{public}s",
            __func__, CmdUtils::CommandToString(unpackCmd)));

        return HDF_SUCCESS;
    }

    // LTPO新增接口
    int32_t SetDisplayConstraint(uint32_t devId, uint64_t frameID, uint64_t ns, uint32_t type)
    {
        int32_t ret = CmdUtils::StartSection(REQUEST_CMD_SET_DISPLAY_CONSTRAINT, requestPacker_);
        DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, ret,
            HDF_LOGE("%{public}s: StartSection failed", __func__));

        bool retBool = requestPacker_.WriteUint32(devId);
        DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE,
            HDF_LOGE("%{public}s: write devId failed", __func__));

        retBool = requestPacker_.WriteUint64(frameID);
        DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE,
            HDF_LOGE("%{public}s: write frameID failed", __func__));

        retBool = requestPacker_.WriteUint64(ns);
        DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE,
            HDF_LOGE("%{public}s: write ns failed", __func__));

        retBool = requestPacker_.WriteUint32(type);
        DISPLAY_CHK_RETURN(retBool == false, HDF_FAILURE,
            HDF_LOGE("%{public}s: write type failed", __func__));

        ret = CmdUtils::EndSection(requestPacker_);
        DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, ret,
            HDF_LOGE("%{public}s: EndSection failed", __func__));

        return HDF_SUCCESS;
    }

protected:
    sptr<CompHdi> hdi_1_2_;
private:
    using BaseType1_1 = V1_1::DisplayCmdRequester<Transfer, CompHdi>;
    using BaseType1_1::requestPacker_;
    using BaseType1_1::DoRequest;
    using BaseType1_1::PeriodDataReset;
    using BaseType1_1::DoReplyResults;

    // Composition layers/types changed
    using BaseType1_1::compChangeLayers_;
    using BaseType1_1::compChangeTypes_;
    // CommitAndGetReleaseFence
    struct FenceData {
        int32_t fence_ = -1;
        int32_t skipValidateState_ = -1;
        bool needFlush_ = false;
        std::vector<uint32_t> layers;
        std::vector<int32_t> fences;
    };
};
using HdiDisplayCmdRequester = V1_2::DisplayCmdRequester<SharedMemQueue<int32_t>, V1_2::IDisplayComposer>;
} // namespace V1_2
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_2_DISPLAY_CMD_REQUESTER_H
