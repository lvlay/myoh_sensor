/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef CAMERA_STREAM_H
#define CAMERA_STREAM_H

#include "camera_common.h"

namespace OHOS::Camera {
class CameraStreams {
public:
    CameraStreams();
    ~CameraStreams();
    RetCode CameraStreamOn(struct CameraFeature feature);
    RetCode CameraStreamOff(struct CameraFeature feature);

private:
};
} // namespace OHOS::Camera
#endif // CAMERA_STREAM_H
