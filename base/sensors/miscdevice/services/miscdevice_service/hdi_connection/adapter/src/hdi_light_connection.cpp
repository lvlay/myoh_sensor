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

#include "hdi_light_connection.h"

#include <memory>
#include <securec.h>
#include <thread>
#include <vector>

#include "hisysevent.h"
#include "v1_0/light_interface_proxy.h"
#include "sensors_errors.h"

#undef LOG_TAG
#define LOG_TAG "HdiLightConnection"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t GET_HDI_SERVICE_COUNT = 10;
constexpr uint32_t WAIT_MS = 100;
} // namespace

int32_t HdiLightConnection::ConnectHdi()
{
    CALL_LOG_ENTER;
    int32_t retry = 0;
    while (retry < GET_HDI_SERVICE_COUNT) {
        lightInterface_ = ILightInterface::Get();
        if (lightInterface_ != nullptr) {
            RegisterHdiDeathRecipient();
            return ERR_OK;
        }
        retry++;
        MISC_HILOGW("Connect hdi service failed, retry:%{public}d", retry);
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
    }
    return ERR_INVALID_VALUE;
}

int32_t HdiLightConnection::GetLightList(std::vector<LightInfoIPC> &lightList) const
{
    CALL_LOG_ENTER;
    std::vector<HDI::Light::V1_0::HdfLightInfo> lightInfos;
    CHKPR(lightInterface_, ERROR);
    int32_t ret = lightInterface_->GetLightInfo(lightInfos);
    if (ret != 0) {
        MISC_HILOGE("Get light info failed");
        return ret;
    }
    for (size_t i = 0; i < lightInfos.size(); ++i) {
        LightInfoIPC light;
        light.SetLightName(lightInfos[i].lightName);
        light.SetLightId(lightInfos[i].lightId);
        light.SetLightNumber(lightInfos[i].lightNumber);
        light.SetLightType(lightInfos[i].lightType);
        lightList.push_back(light);
    }
    return ERR_OK;
}

int32_t HdiLightConnection::TurnOn(int32_t lightId, const LightColor &color, const LightAnimationIPC &animation)
{
    CALL_LOG_ENTER;
    HDI::Light::V1_0::HdfLightColor lightColor;
    lightColor.colorValue.singleColor = color.singleColor;

    HDI::Light::V1_0::HdfLightFlashEffect flashEffect;
    flashEffect.flashMode = animation.GetMode();
    flashEffect.onTime = animation.GetOnTime();
    flashEffect.offTime = animation.GetOffTime();

    HDI::Light::V1_0::HdfLightEffect effect;
    effect.lightColor = lightColor;
    effect.flashEffect = flashEffect;
    CHKPR(lightInterface_, ERROR);
    int32_t ret = lightInterface_->TurnOnLight(lightId, effect);
    if (ret < 0) {
        MISC_HILOGE("TurnOn failed");
        return ret;
    }
    return ERR_OK;
}

int32_t HdiLightConnection::TurnOff(int32_t lightId)
{
    CALL_LOG_ENTER;
    CHKPR(lightInterface_, ERROR);
    int32_t ret = lightInterface_->TurnOffLight(lightId);
    if (ret < 0) {
        MISC_HILOGE("TurnOff failed");
        return ret;
    }
    return ERR_OK;
}
 
int32_t HdiLightConnection::DestroyHdiConnection()
{
    UnregisterHdiDeathRecipient();
    return ERR_OK;
}

void HdiLightConnection::RegisterHdiDeathRecipient()
{
    CALL_LOG_ENTER;
    if (lightInterface_ == nullptr) {
        MISC_HILOGE("Connect v1_0 hdi failed");
        return;
    }
    if (hdiDeathObserver_ == nullptr) {
        hdiDeathObserver_ = new (std::nothrow) DeathRecipientTemplate(*const_cast<HdiLightConnection *>(this));
        CHKPV(hdiDeathObserver_);
    }
    OHOS::HDI::hdi_objcast<ILightInterface>(lightInterface_)->AddDeathRecipient(hdiDeathObserver_);
}

void HdiLightConnection::UnregisterHdiDeathRecipient()
{
    CALL_LOG_ENTER;
    if (lightInterface_ == nullptr || hdiDeathObserver_ == nullptr) {
        MISC_HILOGE("lightInterface_ or hdiDeathObserver_ is nullptr");
        return;
    }
    OHOS::HDI::hdi_objcast<ILightInterface>(lightInterface_)->RemoveDeathRecipient(hdiDeathObserver_);
}

void HdiLightConnection::ProcessDeathObserver(const wptr<IRemoteObject> &object)
{
    CALL_LOG_ENTER;
    sptr<IRemoteObject> hdiService = object.promote();
    if (hdiService == nullptr || hdiDeathObserver_ == nullptr) {
        MISC_HILOGE("Invalid remote object or hdiDeathObserver_ is null");
        return;
    }
    hdiService->RemoveDeathRecipient(hdiDeathObserver_);
    Reconnect();
}

void HdiLightConnection::Reconnect()
{
    if (ConnectHdi() != ERR_OK) {
        MISC_HILOGE("Connect hdi failed");
    }
}
}  // namespace Sensors
}  // namespace OHOS
