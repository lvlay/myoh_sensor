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
#include "light_hdi_connection.h"

#include <list>

#include "compatible_light_connection.h"
#ifdef HDF_DRIVERS_INTERFACE_LIGHT
#include "hdi_light_connection.h"
#endif // HDF_DRIVERS_INTERFACE_LIGHT
#include "hitrace_meter.h"
#include "sensors_errors.h"

#undef LOG_TAG
#define LOG_TAG "LightHdiConnection"

namespace OHOS {
namespace Sensors {

int32_t LightHdiConnection::ConnectHdi()
{
#ifdef HDF_DRIVERS_INTERFACE_LIGHT
    iLightHdiConnection_ = std::make_unique<HdiLightConnection>();
    int32_t ret = ConnectHdiService();
    if (ret == ERR_OK) {
        MISC_HILOGI("Connect light hdi success");
        return ERR_OK;
    }
#endif // HDF_DRIVERS_INTERFACE_LIGHT
    iLightHdiConnection_ = std::make_unique<CompatibleLightConnection>();
    return ConnectHdiService();
}

int32_t LightHdiConnection::ConnectHdiService()
{
    CHKPR(iLightHdiConnection_, ERROR);
    int32_t ret = iLightHdiConnection_->ConnectHdi();
    if (ret != ERR_OK) {
        MISC_HILOGE("Connect hdi service failed");
        return LIGHT_HDF_CONNECT_ERR;
    }
    return iLightHdiConnection_->GetLightList(lightInfoList_);
}

int32_t LightHdiConnection::GetLightList(std::vector<LightInfoIPC> &lightList) const
{
    lightList.assign(lightInfoList_.begin(), lightInfoList_.end());
    return ERR_OK;
}

int32_t LightHdiConnection::TurnOn(int32_t lightId, const LightColor &color, const LightAnimationIPC &animation)
{
    CHKPR(iLightHdiConnection_, ERROR);
    int32_t ret = iLightHdiConnection_->TurnOn(lightId, color, animation);
    if (ret != ERR_OK) {
        MISC_HILOGE("TurnOn failed");
        return LIGHT_ID_NOT_SUPPORT;
    }
    return ERR_OK;
}

int32_t LightHdiConnection::TurnOff(int32_t lightId)
{
    CHKPR(iLightHdiConnection_, ERROR);
    int32_t ret = iLightHdiConnection_->TurnOff(lightId);
    if (ret != ERR_OK) {
        MISC_HILOGE("TurnOff failed");
        return LIGHT_ERR;
    }
    return ERR_OK;
}

int32_t LightHdiConnection::DestroyHdiConnection()
{
    CHKPR(iLightHdiConnection_, ERROR);
    int32_t ret = iLightHdiConnection_->DestroyHdiConnection();
    if (ret != ERR_OK) {
        MISC_HILOGE("DestroyHdiConnection failed");
        return LIGHT_HDF_CONNECT_ERR;
    }
    return ERR_OK;
}
}  // namespace Sensors
}  // namespace OHOS
