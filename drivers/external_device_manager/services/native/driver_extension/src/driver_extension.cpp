/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "driver_extension.h"

#include "configuration_utils.h"
#include "connection_manager.h"
#include "hilog_wrapper.h"
#include "js_driver_extension.h"
#include "runtime.h"
#include "driver_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;

CreatorDriverFunc DriverExtension::creator_ = nullptr;
void DriverExtension::SetCreator(const CreatorDriverFunc& creator)
{
    creator_ = creator;
}

DriverExtension* DriverExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    if (!runtime) {
        return new DriverExtension();
    }

    if (creator_) {
        return creator_(runtime);
    }

    HILOG_INFO("DriverExtension::Create runtime");
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsDriverExtension::Create(runtime);

        default:
            return new DriverExtension();
    }
}

void DriverExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    ExtensionBase<DriverExtensionContext>::Init(record, application, handler, token);
    HILOG_INFO("DriverExtension begin init context");
}

std::shared_ptr<DriverExtensionContext> DriverExtension::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    std::shared_ptr<DriverExtensionContext> context =
        ExtensionBase<DriverExtensionContext>::CreateAndInitContext(record, application, handler, token);
    if (context == nullptr) {
        HILOG_ERROR("DriverExtension::CreateAndInitContext context is nullptr");
        return context;
    }
    return context;
}
} // namespace AbilityRuntime
} // namespace OHOS
