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
#include "medical_js.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <pthread.h>
#include <string>
#include <thread>
#include <unistd.h>

#include "hilog/log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "medical_native_impl.h"
#include "medical_napi_utils.h"

#include "refbase.h"
#include "securec.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;
static constexpr HiLogLabel LABEL = {LOG_CORE, 0xD002786, "AfeJsAPI"};

static std::map<int32_t, struct AsyncCallbackInfo*> g_onCallbackInfos;

static void DataCallbackImpl(SensorEvent *event)
{
    HiLog::Info(LABEL, "%{public}s in", __func__);
    if (event == nullptr) {
        HiLog::Error(LABEL, "%{public}s event is null!", __func__);
        return;
    }
    int32_t sensorTypeId = event->sensorTypeId;
    uint32_t *data = (uint32_t *)(event->data);
    uint32_t maxDataLen = MAX_DATA_LEN * sizeof(uint32_t);
    uint32_t dataLen = (event->dataLen > maxDataLen) ? maxDataLen : event->dataLen;
    // check sensorType
    if (g_onCallbackInfos.find(sensorTypeId) == g_onCallbackInfos.end()) {
        HiLog::Debug(LABEL, "%{public}s no subscribe to the sensor data on", __func__);
        return;
    }
    AsyncCallbackInfo *onCallbackInfo = g_onCallbackInfos[sensorTypeId];
    onCallbackInfo->sensorTypeId = sensorTypeId;
    onCallbackInfo->sensorDataLength = dataLen / sizeof(uint32_t);
    if (memcpy_s(onCallbackInfo->sensorData, dataLen, data, dataLen) != EOK) {
        HiLog::Error(LABEL, "%{public}s copy data failed", __func__);
        return;
    }

    EmitUvEventLoop(static_cast <AsyncCallbackInfo *> (onCallbackInfo));
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

static const MedicalSensorUser user = {
    .callback = DataCallbackImpl
};

static int32_t UnsubscribeSensor(int32_t sensorTypeId)
{
    HiLog::Info(LABEL, "%{public}s in", __func__);
    int32_t ret = DeactivateSensor(sensorTypeId, &user);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s  DeactivateSensor failed", __func__);
        return ret;
    }
    ret = UnsubscribeSensor(sensorTypeId, &user);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s  UnsubscribeSensor failed", __func__);
        return ret;
    }
    HiLog::Info(LABEL, "%{public}s left", __func__);
    return 0;
}

static int32_t SubscribeSensor(int32_t sensorTypeId, int64_t interval, const MedicalSensorUser *user)
{
    HiLog::Info(LABEL, "%{public}s in, sensorTypeId: %{public}d", __func__, sensorTypeId);
    int32_t ret = SubscribeSensor(sensorTypeId, user);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s subscribeSensor failed", __func__);
        return ret;
    }
    ret = SetBatch(sensorTypeId, user, interval, 0);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s set batch failed", __func__);
        return ret;
    }
    ret = ActivateSensor(sensorTypeId, user);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s activateSensor failed", __func__);
        return ret;
    }
    return 0;
}

static napi_value On(napi_env env, napi_callback_info info)
{
    HiLog::Info(LABEL, "%{public}s in", __func__);
    size_t requiredMaxArgc = 3;
    size_t requiredMinArgc = 2;
    size_t argc = 3;
    napi_value args[3];
    napi_value thisVar;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &thisVar, NULL));
    NAPI_ASSERT(env, ((argc >= requiredMinArgc) && (argc <= requiredMaxArgc)), "requires 2 or 3 parameteres");

    napi_valuetype eventId = napi_undefined;
    napi_typeof(env, args[0], &eventId);
    NAPI_ASSERT(env, eventId == napi_number, "type mismatch for parameter 1");
    int32_t sensorTypeId = GetCppInt32(args[0], env);

    napi_valuetype handler = napi_undefined;
    napi_typeof(env, args[1], &handler);
    NAPI_ASSERT(env, handler == napi_function, "type mismatch for parameter 2");
    
    int64_t interval = 200000000;
    if (argc == requiredMaxArgc) {
        HiLog::Info(LABEL, "%{public}s argc = 3!", __func__);
        napi_value value = NapiGetNamedProperty(args[2], "interval", env);
        napi_valuetype intervalType = napi_undefined;
        napi_typeof(env, value, &intervalType);
        NAPI_ASSERT(env, intervalType == napi_number, "type mismatch for parameter 3");
        interval = GetCppInt64(value, env);
    }
    AsyncCallbackInfo *asyncCallbackInfo = new AsyncCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };
    napi_create_reference(env, args[1], 1, &asyncCallbackInfo->callback[0]);
    g_onCallbackInfos[sensorTypeId] = asyncCallbackInfo;
    int32_t ret = SubscribeSensor(sensorTypeId, interval, &user);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s subscribeSensor failed", __func__);
        EmitAsyncCallbackWork(asyncCallbackInfo);
        g_onCallbackInfos.erase(sensorTypeId);
        return nullptr;
    }
    HiLog::Info(LABEL, "%{public}s out", __func__);
    return nullptr;
}
 
static napi_value Off(napi_env env, napi_callback_info info)
{
    HiLog::Info(LABEL, "%{public}s in", __func__);
    size_t argc = 2;
    size_t requiredMaxArgc = 2;
    size_t requiredMinArgc = 1;
    napi_value args[2];
    napi_value thisVar;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &thisVar, NULL));
    NAPI_ASSERT(env, ((argc >= requiredMinArgc) && (argc <= requiredMaxArgc)), "requires 1 or 2 parameteres");

    napi_valuetype eventId = napi_undefined;
    napi_typeof(env, args[0], &eventId);
    NAPI_ASSERT(env, eventId == napi_number, "type mismatch for parameter 1");
    int32_t sensorTypeId = GetCppInt32(args[0], env);

    AsyncCallbackInfo *asyncCallbackInfo = new AsyncCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };

    if (argc == requiredMaxArgc) {
        napi_valuetype handler = napi_undefined;
        napi_typeof(env, args[1], &handler);
        NAPI_ASSERT(env, handler == napi_function, "type mismatch for parameter 2");
    } else {
        NAPI_ASSERT(env, g_onCallbackInfos.find(sensorTypeId) != g_onCallbackInfos.end(), "no callback registered");
        napi_get_reference_value(env, g_onCallbackInfos[sensorTypeId]->callback[0], &args[1]);
    }
    napi_create_reference(env, args[1], 1, &asyncCallbackInfo->callback[0]);

    int32_t ret = UnsubscribeSensor(sensorTypeId);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s UnsubscribeSensor failed", __func__);
    } else {
        HiLog::Error(LABEL, "%{public}s UnsubscribeSensor success", __func__);
        if (g_onCallbackInfos.find(sensorTypeId) != g_onCallbackInfos.end()) {
            napi_delete_reference(env, g_onCallbackInfos[sensorTypeId]->callback[0]);
            delete g_onCallbackInfos[sensorTypeId];
            g_onCallbackInfos[sensorTypeId] = nullptr;
            g_onCallbackInfos.erase(sensorTypeId);
        }
    }
    EmitAsyncCallbackWork(asyncCallbackInfo);
    HiLog::Info(LABEL, "%{public}s left", __func__);
    return nullptr;
}

static napi_value SetOpt(napi_env env, napi_callback_info info)
{
    HiLog::Info(LABEL, "%{public}s in", __func__);
    size_t argc = 2;
    napi_value args[2];
    napi_value thisVar;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &thisVar, NULL));

    if (argc < 1) {
        HiLog::Error(LABEL, "%{public}s Invalid input.", __func__);
        return nullptr;
    }
    if (!IsMatchType(args[0], napi_number, env)) {
        HiLog::Error(LABEL, "%{public}s argument should be number type!", __func__);
        return nullptr;
    }
    int32_t sensorTypeId = GetCppInt32(args[0], env);

    if (!IsMatchType(args[1], napi_number, env)) {
        HiLog::Error(LABEL, "%{public}s argument should be function type!", __func__);
        return nullptr;
    }
    int32_t opt = GetCppInt32(args[1], env);

    int32_t ret = SetOption(sensorTypeId, &user, opt);
    if (ret < 0) {
        HiLog::Error(LABEL, "%{public}s SetOption failed", __func__);
    } else {
        HiLog::Error(LABEL, "%{public}s SetOption success", __func__);
    }
    HiLog::Info(LABEL, "%{public}s left", __func__);
    return nullptr;
}

static napi_value EnumClassConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value args[1] = {0};
    napi_value res = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &res, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    return res;
}

static napi_value CreateEnumMdicalSensorType(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("TYPE_ID_PHOTOPLETHYSMOGRAPH", GetNapiInt32(TYPE_ID_PHOTOPLETHYSMOGRAPH, env)),
    };
    napi_value result = nullptr;
    napi_define_class(env, "MedicalSensorType", NAPI_AUTO_LENGTH, EnumClassConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "MedicalSensorType", result);
    return exports;
}

EXTERN_C_START

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("setOpt", SetOpt),
        DECLARE_NAPI_FUNCTION("off", Off)
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));
    CreateEnumMdicalSensorType(env, exports);
    return exports;
}
EXTERN_C_END

static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = NULL,
    .nm_register_func = Init,
    .nm_modname = "medical",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
} // namespace Sensors
} // namespace OHOS
