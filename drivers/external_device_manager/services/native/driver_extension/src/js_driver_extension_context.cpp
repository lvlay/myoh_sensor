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

#include "js_driver_extension_context.h"

#include <chrono>
#include <cstdint>

#include "ability_manager_client.h"
#include "ability_runtime/js_caller_complex.h"
#include "hilog_wrapper.h"
#include "js_extension_context.h"
#include "js_error_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_ability.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "napi_remote_object.h"
#include "napi_common_start_options.h"
#include "start_options.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr size_t ARGC_ZERO = 0;

class JsDriverExtensionContext final {
public:
    explicit JsDriverExtensionContext(const std::shared_ptr<DriverExtensionContext>& context) : context_(context) {}
    ~JsDriverExtensionContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        HILOG_INFO("JsAbilityContext::Finalizer is called");
        std::unique_ptr<JsDriverExtensionContext>(static_cast<JsDriverExtensionContext*>(data));
    }

    static napi_value UpdateDriverState(napi_env env, napi_callback_info info)
    {
        JsDriverExtensionContext* me = CheckParamsAndGetThis<JsDriverExtensionContext>(env, info);
        return (me != nullptr) ? me->OnUpdateDriverState(env, info) : nullptr;
    }

private:
    std::weak_ptr<DriverExtensionContext> context_;
    sptr<JsFreeInstallObserver> freeInstallObserver_ = nullptr;

    napi_value OnUpdateDriverState(napi_env env, napi_callback_info info)
    {
        HILOG_INFO("OnUpdateDriverState is called");
        
        NapiAsyncTask::CompleteCallback complete =
            [weak = context_](napi_env env, NapiAsyncTask& task, int32_t status) {
                HILOG_INFO("UpdateDriverState begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                    return;
                }

                ErrCode innerErrorCode = context->UpdateDriverState();
                if (innerErrorCode == 0) {
                    napi_value result = nullptr;
                    napi_get_undefined(env, &result);
                    task.Resolve(env, result);
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
                }
            };
        size_t argc = 1;
        napi_value argv[1] = {nullptr};
        napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
        napi_value lastParam = (argc > ARGC_ZERO) ? argv[INDEX_ZERO] : nullptr;

        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSDriverExtensionContext::OnUpdateDriverState",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};
} // namespace

napi_value CreateJsDriverExtensionContext(napi_env env, std::shared_ptr<DriverExtensionContext> context)
{
    HILOG_INFO("CreateJsDriverExtensionContext begin");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }
    napi_value objValue = CreateJsExtensionContext(env, context, abilityInfo);

    std::unique_ptr<JsDriverExtensionContext> jsContext = std::make_unique<JsDriverExtensionContext>(context);
    napi_wrap(env, objValue, jsContext.release(), JsDriverExtensionContext::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsDriverExtensionContext";
    BindNativeFunction(env, objValue, "updateDriverState", moduleName, JsDriverExtensionContext::UpdateDriverState);
    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
