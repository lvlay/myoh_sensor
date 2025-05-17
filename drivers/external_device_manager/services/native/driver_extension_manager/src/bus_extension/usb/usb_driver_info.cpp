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

#include "iostream"
#include "hilog_wrapper.h"
#include "edm_errors.h"
#include "usb_driver_info.h"
#include "cJSON.h"
namespace OHOS {
namespace ExternalDeviceManager {

static int32_t SetArrayToObj(cJSON *obj, const string &key, const vector<uint16_t> &arr)
{
    cJSON* array = cJSON_CreateArray();
    if (!array) {
        EDM_LOGE(MODULE_BUS_USB,  "Create %{public}s error", key.c_str());
        return false;
    }

    for (const auto item : arr) {
        if (!cJSON_AddItemToArray(array, cJSON_CreateNumber(static_cast<double>(item)))) {
            EDM_LOGE(MODULE_BUS_USB,  "AddItemToArray error, key:%{public}s, item:%{public}hu", key.c_str(), item);
            cJSON_Delete(array);
            return false;
        }
    }

    if (!cJSON_AddItemToObject(obj, key.c_str(), array)) {
        EDM_LOGE(MODULE_BUS_USB,  "Add %{public}s to jsonRoot error", key.c_str());
        cJSON_Delete(array);
        return false;
    }

    return true;
}

static bool GetObjectItem(const cJSON *jsonObj, const string &key, vector<uint16_t> &array)
{
    EDM_LOGD(MODULE_BUS_USB, "Enter GetObjectItem");
    cJSON* item = cJSON_GetObjectItem(jsonObj, key.c_str());
    if (!item) {
        EDM_LOGE(MODULE_BUS_USB,  "json member error, need menbers: %{public}s", key.c_str());
        return false;
    }
    if (item->type != cJSON_Array) {
        EDM_LOGE(MODULE_BUS_USB,  "json member type error, %{public}s type is : %{public}d", \
            key.c_str(), item->type);
        return false;
    }
    EDM_LOGD(MODULE_BUS_USB,  "arraysize:%{public}d", cJSON_GetArraySize(item));
    for (int i = 0; i < cJSON_GetArraySize(item); i++) {
        cJSON* it =  cJSON_GetArrayItem(item, i);
        if (!it) {
            EDM_LOGE(MODULE_BUS_USB,  "GetArrayItem fail");
            return false;
        }
        if (it->type != cJSON_Number) {
            EDM_LOGE(MODULE_BUS_USB,  "json %{public}s type error, error type is: %{public}d", \
                key.c_str(), it->type);
            return false;
        }
        array.push_back(static_cast<uint16_t>(it->valuedouble));
    }
    return true;
}

int32_t UsbDriverInfo::Serialize(string &driverStr)
{
    cJSON* jsonRoot = cJSON_CreateObject();
    if (!jsonRoot) {
        EDM_LOGE(MODULE_BUS_USB,  "Create jsonRoot error");
        return EDM_ERR_JSON_OBJ_ERR;
    }

    if (!SetArrayToObj(jsonRoot, "pids", GetProductIds())
        || !SetArrayToObj(jsonRoot, "vids", GetVendorIds())) {
        cJSON_Delete(jsonRoot);
        return EDM_ERR_JSON_OBJ_ERR;
    }

    driverStr = cJSON_PrintUnformatted(jsonRoot);
    cJSON_Delete(jsonRoot);
    return EDM_OK;
}

int32_t UsbDriverInfo::UnSerialize(const string &driverStr)
{
    EDM_LOGD(MODULE_BUS_USB,  "UsbDrvInfo UnSerialize begin");
    cJSON* jsonObj = cJSON_Parse(driverStr.c_str());
    if (!jsonObj) {
        EDM_LOGE(MODULE_BUS_USB,  "UnSeiralize error, parse json string error, str is : %{public}s",\
            driverStr.c_str());
        EDM_LOGE(MODULE_BUS_USB,  "JsonErr:%{public}s", cJSON_GetErrorPtr());
        return EDM_ERR_JSON_PARSE_FAIL;
    }
    if (cJSON_GetArraySize(jsonObj) == 0) {
        EDM_LOGE(MODULE_BUS_USB,  "Json size error");
        cJSON_Delete(jsonObj);
        return EDM_ERR_JSON_PARSE_FAIL;
    }
    EDM_LOGD(MODULE_BUS_USB,  "parse json sucess");

    vector<uint16_t> vids_;
    vector<uint16_t> pids_;
    if (!GetObjectItem(jsonObj, "pids", pids_) || !GetObjectItem(jsonObj, "vids", vids_)) {
        cJSON_Delete(jsonObj);
        return EDM_ERR_JSON_OBJ_ERR;
    }

    EDM_LOGD(MODULE_BUS_USB,  "member type check sucess");
    this->pids_ = pids_;
    this->vids_ = vids_;
    cJSON_Delete(jsonObj);
    return EDM_OK;
}
}
}