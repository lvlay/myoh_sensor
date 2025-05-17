/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "light_client.h"

#include <securec.h>
#include <thread>

#include "death_recipient_template.h"
#include "hisysevent.h"
#include "iservice_registry.h"
#include "sensors_errors.h"
#include "system_ability_definition.h"

#undef LOG_TAG
#define LOG_TAG "LightClient"

namespace OHOS {
namespace Sensors {

namespace {
constexpr int32_t GET_SERVICE_MAX_COUNT = 30;
constexpr uint32_t MAX_LIGHT_LIST_SIZE = 0X00ff;
constexpr uint32_t WAIT_MS = 200;
}  // namespace

LightClient::~LightClient()
{
    CALL_LOG_ENTER;
    if (miscdeviceProxy_ != nullptr && serviceDeathObserver_ != nullptr) {
        auto remoteObject = miscdeviceProxy_->AsObject();
        if (remoteObject != nullptr) {
            remoteObject->RemoveDeathRecipient(serviceDeathObserver_);
        }
    }
}

int32_t LightClient::InitLightClient()
{
    CALL_LOG_ENTER;
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    if (miscdeviceProxy_ != nullptr) {
        MISC_HILOGD("miscdeviceProxy_ already init");
        return ERR_OK;
    }
    auto systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHKPR(systemManager, MISC_NATIVE_SAM_ERR);
    int32_t retry = 0;
    while (retry < GET_SERVICE_MAX_COUNT) {
        miscdeviceProxy_ = iface_cast<IMiscdeviceService>(systemManager->GetSystemAbility(
            MISCDEVICE_SERVICE_ABILITY_ID));
        if (miscdeviceProxy_ != nullptr) {
            MISC_HILOGD("miscdeviceProxy_ get service success, retry:%{public}d", retry);
            serviceDeathObserver_ = new (std::nothrow) DeathRecipientTemplate(*const_cast<LightClient *>(this));
            CHKPR(serviceDeathObserver_, MISC_NATIVE_GET_SERVICE_ERR);
            auto remoteObject = miscdeviceProxy_->AsObject();
            CHKPR(remoteObject, MISC_NATIVE_GET_SERVICE_ERR);
            remoteObject->AddDeathRecipient(serviceDeathObserver_);
            lightInfoList_ = miscdeviceProxy_->GetLightList();
            return ERR_OK;
        }
        MISC_HILOGW("Get service failed, retry:%{public}d", retry);
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
        retry++;
    }
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::MISCDEVICE, "MISC_SERVICE_EXCEPTION",
        HiviewDFX::HiSysEvent::EventType::FAULT, "PKG_NAME", "InitLightClient", "ERROR_CODE",
        MISC_NATIVE_GET_SERVICE_ERR);
    MISC_HILOGE("Get service failed");
    return MISC_NATIVE_GET_SERVICE_ERR;
}

bool LightClient::IsLightIdValid(int32_t lightId)
{
    CALL_LOG_ENTER;
    int32_t ret = InitLightClient();
    if (ret != ERR_OK) {
        MISC_HILOGE("InitLightClient failed, ret:%{public}d", ret);
        return false;
    }
    for (const auto &item : lightInfoList_) {
        if (lightId == item.GetLightId()) {
            return true;
        }
    }
    return false;
}

int32_t LightClient::GetLightList(LightInfo **lightInfo, int32_t &count)
{
    CALL_LOG_ENTER;
    CHKPR(lightInfo, ERROR);
    int32_t ret = InitLightClient();
    if (ret != ERR_OK) {
        MISC_HILOGE("InitLightClient failed");
        return ERROR;
    }
    std::lock_guard<std::mutex> lightInfosLock(lightInfosMutex_);
    if (lightInfos_ == nullptr) {
        ret = ConvertLightInfos();
        if (ret != ERR_OK) {
            MISC_HILOGE("Convert light lists failed");
            ClearLightInfos();
            return ERROR;
        }
    }
    *lightInfo = lightInfos_;
    count = lightInfoCount_;
    return ERR_OK;
}

bool LightClient::IsLightAnimationValid(const LightAnimation &animation)
{
    CALL_LOG_ENTER;
    if ((animation.mode < 0) || (animation.mode >= LIGHT_MODE_BUTT)) {
        MISC_HILOGE("animation mode is invalid, mode:%{public}d", animation.mode);
        return false;
    }
    if ((animation.onTime < 0) || (animation.offTime < 0)) {
        MISC_HILOGE("animation onTime or offTime is invalid, onTime:%{public}d, offTime:%{public}d",
            animation.onTime, animation.offTime);
        return false;
    }
    return true;
}

int32_t LightClient::TurnOn(int32_t lightId, const LightColor &color, const LightAnimation &animation)
{
    CALL_LOG_ENTER;
    if (!IsLightIdValid(lightId)) {
        MISC_HILOGE("lightId is invalid, lightId:%{public}d", lightId);
        return PARAMETER_ERROR;
    }
    if (!IsLightAnimationValid(animation)) {
        MISC_HILOGE("animation is invalid");
        return PARAMETER_ERROR;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    CHKPR(miscdeviceProxy_, ERROR);
    LightAnimationIPC animationIPC;
    animationIPC.SetMode(animation.mode);
    animationIPC.SetOnTime(animation.onTime);
    animationIPC.SetOffTime(animation.offTime);
    return miscdeviceProxy_->TurnOn(lightId, color, animationIPC);
}

int32_t LightClient::TurnOff(int32_t lightId)
{
    CALL_LOG_ENTER;
    if (!IsLightIdValid(lightId)) {
        MISC_HILOGE("lightId is invalid, lightId:%{public}d", lightId);
        return LIGHT_ID_NOT_SUPPORT;
    }
    std::lock_guard<std::mutex> clientLock(clientMutex_);
    CHKPR(miscdeviceProxy_, ERROR);
    return miscdeviceProxy_->TurnOff(lightId);
}

void LightClient::ProcessDeathObserver(wptr<IRemoteObject> object)
{
    CALL_LOG_ENTER;
    (void)object;
    {
        std::lock_guard<std::mutex> clientLock(clientMutex_);
        miscdeviceProxy_ = nullptr;
    }

    auto ret = InitLightClient();
    if (ret != ERR_OK) {
        MISC_HILOGE("InitLightClient failed, ret:%{public}d", ret);
        return;
    }
}

void LightClient::ClearLightInfos()
{
    CALL_LOG_ENTER;
    CHKPV(lightInfos_);
    free(lightInfos_);
    lightInfos_ = nullptr;
}

int32_t LightClient::ConvertLightInfos()
{
    CALL_LOG_ENTER;
    if (lightInfoList_.empty()) {
        MISC_HILOGE("Get light lists failed");
        return ERROR;
    }
    size_t count = lightInfoList_.size();
    if (count > MAX_LIGHT_LIST_SIZE) {
        MISC_HILOGE("The number of lights exceed the maximum value");
        return ERROR;
    }
    lightInfos_ = (LightInfo *)malloc(sizeof(LightInfo) * count);
    CHKPR(lightInfos_, ERROR);
    for (size_t i = 0; i < count; ++i) {
        LightInfo *lightInfo = lightInfos_ + i;
        errno_t ret = strcpy_s(lightInfo->lightName, NAME_MAX_LEN,
            lightInfoList_[i].GetLightName().c_str());
        CHKCR(ret == EOK, ERROR, "strcpy_s error");
        lightInfo->lightId = lightInfoList_[i].GetLightId();
        lightInfo->lightNumber = lightInfoList_[i].GetLightNumber();
        lightInfo->lightType = lightInfoList_[i].GetLightType();
    }
    lightInfoCount_ = static_cast<int32_t>(count);
    return SUCCESS;
}
}  // namespace Sensors
}  // namespace OHOS
