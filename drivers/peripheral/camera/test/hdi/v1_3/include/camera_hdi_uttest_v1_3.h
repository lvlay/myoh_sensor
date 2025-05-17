/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file expected in compliance with the License.
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

#ifndef CAMERA_HDI_UTTEST_V1_3_H
#define CAMERA_HDI_UTTEST_V1_3_H

#include "hdi_common_v1_3.h"

class CameraHdiUtTestV1_3 : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);
    void TakePhotoWithTags(std::shared_ptr<OHOS::Camera::CameraSetting> meta);
    std::shared_ptr<OHOS::Camera::Test> cameraTest = nullptr;
};
#endif
