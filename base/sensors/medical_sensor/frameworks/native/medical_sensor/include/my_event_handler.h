/*
 * Copyright (c) 2022 Chipsea Technologies (Shenzhen) Corp., Ltd.
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

#ifndef MY_EVENT_HANDLER_H
#define MY_EVENT_HANDLER_H

#include "event_handler.h"
#include "event_runner.h"

namespace OHOS {
namespace Sensors {
class MyEventHandler : public AppExecFwk::EventHandler {
public:
    explicit MyEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
    ~MyEventHandler() = default;

    /**
     * Function: Process the event. Override the method of base class.
     * @param event The event need to be processed.
     */
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // MY_EVENT_HANDLER_H