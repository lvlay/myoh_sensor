/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include <string>

#include "vibrator.h"

#include "miscdevice_log.h"
#include "vibrator_agent.h"
#include "vibrator_type.h"

#undef LOG_TAG
#define LOG_TAG "Vibrator"

int32_t OH_Vibrator_PlayVibration(int32_t duration, Vibrator_Attribute vibrateAttribute)
{
    if (duration <= 0) {
        MISC_HILOGE("duration is invalid, duration is %{public}d", duration);
        return PARAMETER_ERROR;
    }
    if ((vibrateAttribute.usage < VIBRATOR_USAGE_UNKNOWN) || (vibrateAttribute.usage >= VIBRATOR_USAGE_MAX)) {
        MISC_HILOGE("vibrate attribute value is is %{public}d", vibrateAttribute.usage);
        return PARAMETER_ERROR;
    }
    if (!OHOS::Sensors::SetUsage(vibrateAttribute.usage)) {
        MISC_HILOGE("SetUsage failed");
        return PARAMETER_ERROR;
    }
    int32_t ret = OHOS::Sensors::StartVibratorOnce(duration);
    if (ret != OHOS::ERR_OK) {
        MISC_HILOGE("start Vibrator failed, ret is %{public}d", ret);
    }
    return ret;
}

int32_t OH_Vibrator_PlayVibrationCustom(Vibrator_FileDescription fileDescription,
    Vibrator_Attribute vibrateAttribute)
{
    if (!OHOS::Sensors::IsSupportVibratorCustom()) {
        MISC_HILOGE("feature is not supported");
        return OHOS::Sensors::IS_NOT_SUPPORTED;
    }
    if ((fileDescription.fd < 0) || (fileDescription.offset < 0) || (fileDescription.length <= 0)) {
        MISC_HILOGE("fileDescription is invalid");
        return PARAMETER_ERROR;
    }
    if ((vibrateAttribute.usage < VIBRATOR_USAGE_UNKNOWN) || (vibrateAttribute.usage >= VIBRATOR_USAGE_MAX)) {
        MISC_HILOGE("vibrate attribute value is invalid");
        return PARAMETER_ERROR;
    }
    if (!OHOS::Sensors::SetUsage(vibrateAttribute.usage)) {
        MISC_HILOGE("SetUsage failed");
        return PARAMETER_ERROR;
    }
    int32_t ret = OHOS::Sensors::PlayVibratorCustom(fileDescription.fd, fileDescription.offset, fileDescription.length);
    if (ret != OHOS::ERR_OK) {
        MISC_HILOGE("play vibrator custom failed, ret is %{public}d", ret);
    }
    return ret;
}

int32_t OH_Vibrator_Cancel()
{
    int32_t ret = OHOS::Sensors::Cancel();
    if (ret != OHOS::ERR_OK) {
        MISC_HILOGE("cancel Vibrator failed, ret is %{public}d", ret);
    }
    return ret;
}