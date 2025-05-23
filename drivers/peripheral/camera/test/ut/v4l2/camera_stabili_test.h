/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef CAMERA_STABILI_TEST_H
#define CAMERA_STABILI_TEST_H

#include "test_camera_base.h"

class CameraStabiliTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);
    void GetAvalialbleVideoStabilizationModes(std::shared_ptr<CameraAbility> &ability);
    std::shared_ptr<TestCameraBase> cameraBase_ = nullptr;
    std::vector<uint8_t> videoStabilizationAvailableModes_;
};
#endif /* CAMERA_STABILI_TEST_H */