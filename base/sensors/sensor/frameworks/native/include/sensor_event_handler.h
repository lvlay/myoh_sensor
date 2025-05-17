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

#ifndef SENSOR_EVENT_HANDLER_H
#define SENSOR_EVENT_HANDLER_H

#include "event_handler.h"
#include "event_runner.h"

namespace OHOS {
namespace Sensors {
class SensorEventHandler : public AppExecFwk::EventHandler {
public:
    explicit SensorEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
    ~SensorEventHandler() = default;
    /**
     * Function: Process the event. Override the method of base class.
     * @param event The event need to be processed.
     */
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
};
}  // namespace Sensors
}  // namespace OHOS
#endif // SENSOR_EVENT_HANDLER_H