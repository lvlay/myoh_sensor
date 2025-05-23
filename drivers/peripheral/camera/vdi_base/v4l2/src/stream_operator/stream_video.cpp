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

#include "stream_video.h"

namespace OHOS::Camera {
StreamVideo::StreamVideo(const int32_t id,
                         const VdiStreamIntent type,
                         std::shared_ptr<IPipelineCore>& p,
                         std::shared_ptr<CaptureMessageOperator>& m)
    : StreamBase(id, type, p, m)
{
    CAMERA_LOGV("enter");
}

StreamVideo::~StreamVideo()
{
    CAMERA_LOGV("enter");
}

RetCode StreamVideo::Capture(const std::shared_ptr<CaptureRequest>& request)
{
    if (state_ == STREAM_STATE_OFFLINE) {
        return RC_OK;
    }
    
    CAMERA_LOGD("start StreamVideo::Capture! ");
    CameraHalTimeSysevent::WriteTimeStatisicEvent(CameraHalTimeSysevent::GetEventName(TIME_OF_VEDIOA_AND_DURATION));
    return StreamBase::Capture(request);
}

REGISTERSTREAM(StreamVideo, {"VIDEO"});
} // namespace OHOS::Camera
