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

#ifndef DEATH_RECIPIENT_TEMPLATE_H
#define DEATH_RECIPIENT_TEMPLATE_H

#include "iremote_object.h"

namespace OHOS {
namespace Sensors {
template<typename T>
class DeathRecipientTemplate : public IRemoteObject::DeathRecipient {
public:
    explicit DeathRecipientTemplate(T &privateData) : privateData_(privateData) {};
    virtual ~DeathRecipientTemplate() = default;
    virtual void OnRemoteDied(const wptr<IRemoteObject> &object)
    {
        privateData_.ProcessDeathObserver(object);
    };

private:
    T &privateData_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // DEATH_RECIPIENT_TEMPLATE_H
