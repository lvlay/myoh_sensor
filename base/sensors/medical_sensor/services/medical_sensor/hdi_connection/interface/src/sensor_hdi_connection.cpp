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
#include "sensor_hdi_connection.h"

#include "compatible_connection.h"
#include "hdi_connection.h"
#include "medical_errors.h"
#include "medical_log_domain.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = {
    LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_HDI_INTERFACE, "MedicalSensor_SensorHdiConnection"
};
}

int32_t SensorHdiConnection::ConnectHdi()
{
    iSensorHdiConnection_ = std::make_unique<HdiConnection>();
    int32_t ret = connectHdiService();
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s connect hdi service failed, try to connect compatible connection",
            __func__);
        iSensorHdiConnection_ = std::make_unique<CompatibleConnection>();
        ret = connectHdiService();
    }
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "%{public}s connect hdi failed", __func__);
    }
    return ERR_OK;
}

int32_t SensorHdiConnection::connectHdiService()
{
    int32_t ret = iSensorHdiConnection_->ConnectHdi();
    if (ret != 0) {
        HiLog::Error(LABEL, "%{public}s connect hdi service failed", __func__);
        return CONNECT_SENSOR_HDF_ERR;
    }
    ret = iSensorHdiConnection_->GetSensorList(sensorList_);
    if (ret != 0) {
        HiLog::Error(LABEL, "%{public}s get sensor list failed", __func__);
        return GET_SENSOR_LIST_ERR;
    }
    return ERR_OK;
}

int32_t SensorHdiConnection::GetSensorList(std::vector<MedicalSensor>& sensorList)
{
    sensorList.assign(sensorList_.begin(), sensorList_.end());
    return ERR_OK;
}

int32_t SensorHdiConnection::EnableSensor(int32_t sensorId)
{
    int32_t ret = iSensorHdiConnection_->EnableSensor(sensorId);
    if (ret != 0) {
        HiLog::Info(LABEL, "%{public}s enable sensor failed, sensorId: %{public}d", __func__, sensorId);
        return ENABLE_SENSOR_ERR;
    }
    return ret;
};

int32_t SensorHdiConnection::DisableSensor(int32_t sensorId)
{
    int32_t ret = iSensorHdiConnection_->DisableSensor(sensorId);
    if (ret != 0) {
        HiLog::Info(LABEL, "%{public}s disable sensor failed, sensorId: %{public}d", __func__, sensorId);
        return DISABLE_SENSOR_ERR;
    }
    return ret;
}

int32_t SensorHdiConnection::SetBatch(int32_t sensorId, int64_t samplingInterval, int64_t reportInterval)
{
    int32_t ret = iSensorHdiConnection_->SetBatch(sensorId, samplingInterval, reportInterval);
    if (ret != 0) {
        HiLog::Info(LABEL, "%{public}s set batch failed, sensorId: %{public}d", __func__, sensorId);
        return SET_SENSOR_CONFIG_ERR;
    }
    return ret;
}

int32_t SensorHdiConnection::SetMode(int32_t sensorId, int32_t mode)
{
    int32_t ret = iSensorHdiConnection_->SetMode(sensorId, mode);
    if (ret != 0) {
        HiLog::Info(LABEL, "%{public}s set mode failed, sensorId: %{public}d", __func__, sensorId);
        return SET_SENSOR_MODE_ERR;
    }
    return ret;
}

int32_t SensorHdiConnection::SetOption(int32_t sensorId, int32_t option)
{
    int32_t ret = iSensorHdiConnection_->SetOption(sensorId, option);
    if (ret != 0) {
        HiLog::Info(LABEL, "%{public}s set option failed, sensorId: %{public}d", __func__, sensorId);
        return SET_SENSOR_OPTION_ERR;
    }
    return ret;
}

int32_t SensorHdiConnection::RunCommand(int32_t sensorId, int32_t cmd, int32_t params)
{
    int32_t ret = iSensorHdiConnection_->RunCommand(sensorId, cmd, params);
    if (ret != 0) {
        HiLog::Info(LABEL, "%{public}s run command failed, sensorId: %{public}d", __func__, sensorId);
        return RUN_COMMAND_ERR;
    }
    return ret;
}

int32_t SensorHdiConnection::RegisteDataReport(DataCacheFunc cacheData, sptr<ReportDataCache> reportDataCache)
{
    int32_t ret = iSensorHdiConnection_->RegisteDataReport(cacheData, reportDataCache);
    if (ret != 0) {
        HiLog::Info(LABEL, "%{public}s registe dataReport failed", __func__);
        return REGIST_CALLBACK_ERR;
    }
    return ret;
}

int32_t SensorHdiConnection::DestroyHdiConnection()
{
    int32_t ret = iSensorHdiConnection_->DestroyHdiConnection();
    if (ret != 0) {
        HiLog::Info(LABEL, "%{public}s destroy hdi connectionr failed", __func__);
        return DEVICE_ERR;
    }
    return ret;
}
}  // namespace Sensors
}  // namespace OHOS
