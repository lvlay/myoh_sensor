/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *     http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HOS_CAMERA_SCALE_NODE_H
#define HOS_CAMERA_SCALE_NODE_H

#include <vector>
#include <condition_variable>
#include <ctime>
#include <mutex>
#include "device_manager_adapter.h"
#include "utils.h"
#include "camera.h"
#include "source_node.h"
#include "node_utils.h"
namespace OHOS::Camera {
class ScaleNode : public NodeBase {
public:
    ScaleNode(const std::string& name, const std::string& type, const std::string &cameraId);
    ~ScaleNode() override;
    RetCode Start(const int32_t streamId) override;
    RetCode Stop(const int32_t streamId) override;
    void DeliverBuffer(std::shared_ptr<IBuffer>& buffer) override;
    virtual RetCode Capture(const int32_t streamId, const int32_t captureId) override;
    RetCode CancelCapture(const int32_t streamId) override;
    RetCode Flush(const int32_t streamId);
};
} // namespace OHOS::Camera
#endif
