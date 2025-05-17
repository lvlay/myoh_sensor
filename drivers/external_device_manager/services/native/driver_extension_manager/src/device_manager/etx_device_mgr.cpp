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

#include "etx_device_mgr.h"
#include "cinttypes"
#include "common_timer_errors.h"
#include "driver_extension_controller.h"
#include "driver_pkg_manager.h"
#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bundle_update_callback.h"

namespace OHOS {
namespace ExternalDeviceManager {
constexpr uint32_t UNLOAD_SA_TIMER_INTERVAL = 30 * 1000;
std::string Device::stiching_ = "-";
IMPLEMENT_SINGLE_INSTANCE(ExtDeviceManager);

ExtDeviceManager::~ExtDeviceManager()
{
    unloadSelftimer_.Unregister(unloadSelftimerId_);
    unloadSelftimer_.Shutdown();
}

void ExtDeviceManager::PrintMatchDriverMap()
{
    if (!bundleMatchMap_.empty()) {
        EDM_LOGD(MODULE_DEV_MGR, " bundleInfo map size[%{public}zu]", bundleMatchMap_.size());
        for (auto iter : bundleMatchMap_) {
            for (auto iterId = iter.second.begin(); iterId != iter.second.end(); ++iterId) {
                uint64_t deviceId = *iterId;
                EDM_LOGD(MODULE_DEV_MGR, "print match map info[%{public}s], deviceId %{public}016" PRIX64 "",
                    iter.first.c_str(), deviceId);
            }
        }
        EDM_LOGD(MODULE_DEV_MGR, "ExtDeviceManager print driver match map success");
    }
}

int32_t ExtDeviceManager::Init()
{
    EDM_LOGD(MODULE_DEV_MGR, "ExtDeviceManager Init success");
    auto callback = make_shared<BundleUpdateCallback>();
    int32_t ret = DriverPkgManager::GetInstance().RegisterBundleCallback(callback);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "Register bundle update callback fail");
        return EDM_NOK;
    }
    return EDM_OK;
}

int32_t ExtDeviceManager::AddDevIdOfBundleInfoMap(shared_ptr<Device> device, string &bundleInfo)
{
    if (bundleInfo.empty() || device == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "bundleInfo or device is null");
        return EDM_ERR_INVALID_PARAM;
    }

    // update bundle info
    lock_guard<mutex> lock(bundleMatchMapMutex_);
    auto pos = bundleMatchMap_.find(bundleInfo);
    uint64_t deviceId = device->GetDeviceInfo()->GetDeviceId();
    if (pos == bundleMatchMap_.end()) {
        unordered_set<uint64_t> tmpSet;
        tmpSet.emplace(deviceId);
        bundleMatchMap_.emplace(bundleInfo, tmpSet);
        EDM_LOGI(MODULE_DEV_MGR, "bundleMap emplace New driver, add deviceId %{public}016" PRIX64 "", deviceId);
    } else {
        auto pairRet = pos->second.emplace(deviceId);
        // Check whether the deviceId matches the driver
        if (!pairRet.second || pos->second.size() > 1) {
            EDM_LOGI(MODULE_DEV_MGR, "bundleMap had existed driver, add deviceId %{public}016" PRIX64 "", deviceId);
            PrintMatchDriverMap();
        }
    }

    // start ability
    int32_t ret = device->Connect();
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR,
            "deviceId[%{public}016" PRIX64 "] connect driver extension ability[%{public}s] failed[%{public}d]",
            deviceId, Device::GetAbilityName(bundleInfo).c_str(), ret);
        return EDM_NOK;
    }
    PrintMatchDriverMap();
    return EDM_OK;
}

int32_t ExtDeviceManager::RemoveDevIdOfBundleInfoMap(shared_ptr<Device> device, string &bundleInfo)
{
    if (bundleInfo.empty() || device == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "bundleInfo or device is null");
        return EDM_ERR_INVALID_PARAM;
    }

    // update bundle info
    lock_guard<mutex> lock(bundleMatchMapMutex_);
    auto pos = bundleMatchMap_.find(bundleInfo);
    if (pos == bundleMatchMap_.end()) {
        EDM_LOGI(MODULE_DEV_MGR, "not find bundleInfo from map");
        return EDM_OK;
    }

    // If the number of devices is greater than one, only the device erase
    uint64_t deviceId = device->GetDeviceInfo()->GetDeviceId();
    if (pos->second.size() > 1) {
        pos->second.erase(deviceId);
        EDM_LOGD(MODULE_DEV_MGR, "bundleMap existed driver, remove deviceId %{public}016" PRIX64 "", deviceId);
        PrintMatchDriverMap();
        return EDM_OK;
    }

    EDM_LOGD(MODULE_DEV_MGR, "bundleMap remove bundleInfo[%{public}s]", bundleInfo.c_str());
    bundleMatchMap_.erase(pos);

    // stop ability and destory sa
    int32_t ret = device->Disconnect();
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR,
            "deviceId[%{public}016" PRIX64 "] disconnect driver extension ability[%{public}s] failed[%{public}d]",
            deviceId, Device::GetAbilityName(bundleInfo).c_str(), ret);
        return ret;
    }
    PrintMatchDriverMap();
    return EDM_OK;
}

void ExtDeviceManager::RemoveDeviceOfDeviceMap(shared_ptr<Device> device)
{
    EDM_LOGI(MODULE_DEV_MGR, "RemoveDeviceOfDeviceMap enter");
    shared_ptr<DeviceInfo> deviceInfo = device->GetDeviceInfo();
    if (deviceInfo == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "device info is null");
        return;
    }
    BusType type = deviceInfo->GetBusType();
    uint64_t deviceId = deviceInfo->GetDeviceId();

    lock_guard<mutex> lock(deviceMapMutex_);
    if (deviceMap_.find(type) != deviceMap_.end()) {
        unordered_map<uint64_t, shared_ptr<Device>> &map = deviceMap_[type];
        map.erase(deviceId);
        EDM_LOGI(MODULE_DEV_MGR, "success RemoveDeviceOfDeviceMap, deviceId:%{public}016" PRIx64 "", deviceId);
    }
}

std::unordered_set<uint64_t> ExtDeviceManager::DeleteBundlesOfBundleInfoMap(const std::string &bundleName)
{
    EDM_LOGD(MODULE_DEV_MGR, "DeleteBundlesOfBundleInfoMap enter");
    std::unordered_set<uint64_t> deviceIds;
    lock_guard<mutex> lock(bundleMatchMapMutex_);
    if (bundleName.empty()) {
        bundleMatchMap_.clear();
    } else {
        std::string startStr = bundleName + Device::GetStiching();
        for (auto iter = bundleMatchMap_.begin(); iter != bundleMatchMap_.end();) {
            if (startStr.compare(iter->first.substr(0, startStr.size())) == 0) {
                deviceIds.insert(iter->second.begin(), iter->second.end());
                iter = bundleMatchMap_.erase(iter);
            } else {
                iter++;
            }
        }
    }
    return deviceIds;
}

void ExtDeviceManager::MatchDriverInfos(std::unordered_set<uint64_t> deviceIds, bool isDriversReset)
{
    EDM_LOGI(MODULE_DEV_MGR, "MatchDriverInfos enter");
    lock_guard<mutex> lock(deviceMapMutex_);
    for (auto &m : deviceMap_) {
        for (auto &[deviceId, device] : m.second) {
            if (isDriversReset) {
                device->RemoveBundleInfo();
                device->ClearDrvExtRemote();
            }
            if (deviceIds.find(deviceId) != deviceIds.end()) {
                device->RemoveBundleInfo();
            }
            if (device->IsUnRegisted() || device->GetDrvExtRemote() != nullptr) {
                continue;
            }
            auto bundleInfoNames = DriverPkgManager::GetInstance().QueryMatchDriver(device->GetDeviceInfo());
            if (bundleInfoNames == nullptr) {
                EDM_LOGD(MODULE_DEV_MGR, "deviceId[%{public}016" PRIX64 "], not find driver", deviceId);
                continue;
            }
            std::string bundleInfo = bundleInfoNames->bundleName + Device::GetStiching() + bundleInfoNames->abilityName;
            EDM_LOGI(MODULE_DEV_MGR, "MatchDriverInfo success, bundleInfo: %{public}s", bundleInfo.c_str());
            device->AddBundleInfo(bundleInfo, bundleInfoNames->driverUid);
            int32_t ret = AddDevIdOfBundleInfoMap(device, bundleInfo);
            if (ret != EDM_OK) {
                EDM_LOGD(MODULE_DEV_MGR,
                    "deviceId[%{public}016" PRIX64 "] AddDevIdOfBundleInfoMap failed, ret=%{public}d", deviceId, ret);
            }
        }
    }
}

void ExtDeviceManager::ClearMatchedDrivers(const int32_t userId)
{
    EDM_LOGI(MODULE_DEV_MGR, "ClearMatchedDrivers start");
    lock_guard<mutex> lock(bundleMatchMapMutex_);
    for (auto iter : bundleMatchMap_) {
        std::string bundleInfo = iter.first;
        (void)DriverExtensionController::GetInstance().StopDriverExtension(Device::GetBundleName(bundleInfo),
            Device::GetAbilityName(bundleInfo), userId);
    }
    bundleMatchMap_.clear();
}

int32_t ExtDeviceManager::RegisterDevice(shared_ptr<DeviceInfo> devInfo)
{
    BusType type = devInfo->GetBusType();
    uint64_t deviceId = devInfo->GetDeviceId();
    shared_ptr<Device> device;
    lock_guard<mutex> lock(deviceMapMutex_);
    if (deviceMap_.find(type) != deviceMap_.end()) {
        unordered_map<uint64_t, shared_ptr<Device>> &map = deviceMap_[type];
        if (map.find(deviceId) != map.end() && map[deviceId] != nullptr) {
            device = map.find(deviceId)->second;
            // device has been registered and do not need to connect again
            if (device->GetDrvExtRemote() != nullptr) {
                EDM_LOGI(MODULE_DEV_MGR, "device has been registered, deviceId is %{public}016" PRIx64 "", deviceId);
                return EDM_OK;
            }
            // device has been registered and need to connect
            EDM_LOGI(MODULE_DEV_MGR, "device has been registered, deviceId is %{public}016" PRIx64 "", deviceId);
        }
    } else {
        EDM_LOGI(MODULE_DEV_MGR, "emplace Type of deviceMap_");
        deviceMap_.emplace(type, unordered_map<uint64_t, shared_ptr<Device>>());
    }
    EDM_LOGD(MODULE_DEV_MGR, "begin to register device, deviceId is %{public}016" PRIx64 "", deviceId);
    // device need to register
    if (device == nullptr) {
        device = make_shared<Device>(devInfo);
        deviceMap_[type].emplace(deviceId, device);
        EDM_LOGI(MODULE_DEV_MGR, "successfully registered device, deviceId = %{public}016" PRIx64 "", deviceId);
    }
    // driver match
    std::string bundleInfo = device->GetBundleInfo();
    // if device does not have a matching driver, match driver here
    if (bundleInfo.empty()) {
        auto bundleInfoNames = DriverPkgManager::GetInstance().QueryMatchDriver(devInfo);
        if (bundleInfoNames != nullptr) {
            bundleInfo = bundleInfoNames->bundleName + Device::GetStiching() + bundleInfoNames->abilityName;
            device->AddBundleInfo(bundleInfo, bundleInfoNames->driverUid);
        }
    }
    unloadSelftimer_.Unregister(unloadSelftimerId_);

    // match driver failed, waitting to install driver package
    if (bundleInfo.empty()) {
        EDM_LOGD(MODULE_DEV_MGR,
            "deviceId %{public}016" PRIX64 "match driver failed, waitting to install ext driver package", deviceId);
        return EDM_OK;
    }

    int32_t ret = AddDevIdOfBundleInfoMap(device, bundleInfo);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "deviceId[%{public}016" PRIX64 "] update bundle info map failed[%{public}d]", deviceId,
            ret);
        return EDM_NOK;
    }
    EDM_LOGI(MODULE_DEV_MGR, "successfully match driver[%{public}s], deviceId is %{public}016" PRIx64 "",
        bundleInfo.c_str(), deviceId);

    return ret;
}

int32_t ExtDeviceManager::UnRegisterDevice(const shared_ptr<DeviceInfo> devInfo)
{
    BusType type = devInfo->GetBusType();
    uint64_t deviceId = devInfo->GetDeviceId();
    shared_ptr<Device> device;
    string bundleInfo;

    lock_guard<mutex> lock(deviceMapMutex_);
    if (deviceMap_.find(type) != deviceMap_.end()) {
        unordered_map<uint64_t, shared_ptr<Device>> &map = deviceMap_[type];
        if (map.find(deviceId) != map.end()) {
            device = map[deviceId];
            bundleInfo = map[deviceId]->GetBundleInfo();
            if (device != nullptr && device->GetDrvExtRemote() != nullptr) {
                device->UnRegist();
            } else {
                map.erase(deviceId);
            }
            EDM_LOGI(MODULE_DEV_MGR, "successfully unregistered device, deviceId is %{public}016" PRIx64 "", deviceId);
            UnLoadSelf();
        }
    }

    if (bundleInfo.empty()) {
        EDM_LOGI(MODULE_DEV_MGR, "deviceId %{public}016" PRIX64 " bundleInfo is empty", deviceId);
        return EDM_OK;
    }

    int32_t ret = RemoveDevIdOfBundleInfoMap(device, bundleInfo);
    if (ret != EDM_OK) {
        EDM_LOGE(
            MODULE_DEV_MGR, "deviceId[%{public}016" PRIX64 "] remove bundleInfo map failed[%{public}d]", deviceId, ret);
        return ret;
    }

    EDM_LOGI(MODULE_DEV_MGR, "successfully remove bundleInfo, deviceId %{public}016" PRIx64 ", bundleInfo[%{public}s]",
        deviceId, bundleInfo.c_str());

    return EDM_OK;
}

vector<shared_ptr<DeviceInfo>> ExtDeviceManager::QueryDevice(const BusType busType)
{
    vector<shared_ptr<DeviceInfo>> devInfoVec;

    lock_guard<mutex> lock(deviceMapMutex_);
    if (deviceMap_.find(busType) == deviceMap_.end()) {
        EDM_LOGE(MODULE_DEV_MGR, "no device is found and busType %{public}d is invalid", busType);
        return devInfoVec;
    }

    unordered_map<uint64_t, shared_ptr<Device>> map = deviceMap_[busType];
    for (auto &[_, device] : map) {
        if (device != nullptr && !device->IsUnRegisted()) {
            devInfoVec.emplace_back(device->GetDeviceInfo());
        }
    }
    EDM_LOGD(MODULE_DEV_MGR, "find %{public}zu device of busType %{public}d", devInfoVec.size(), busType);

    return devInfoVec;
}

vector<shared_ptr<Device>> ExtDeviceManager::QueryAllDevices()
{
    vector<shared_ptr<Device>> devices;
    lock_guard<mutex> lock(deviceMapMutex_);

    for (auto &m : deviceMap_) {
        for (auto &[_, device] : m.second) {
            if (device != nullptr && !device->IsUnRegisted()) {
                devices.emplace_back(device);
            }
        }
    }

    return devices;
}

vector<shared_ptr<Device>> ExtDeviceManager::QueryDevicesById(const uint64_t deviceId)
{
    vector<shared_ptr<Device>> devices;
    lock_guard<mutex> lock(deviceMapMutex_);

    for (auto &m : deviceMap_) {
        for (auto &[id, device] : m.second) {
            if (deviceId == id && device != nullptr && !device->IsUnRegisted()) {
                devices.emplace_back(device);
            }
        }
    }

    return devices;
}


size_t ExtDeviceManager::GetTotalDeviceNum(void) const
{
    // Please do not add lock. This will be called in the UnRegisterDevice.
    size_t totalNum = 0;
    for (auto &m : deviceMap_) {
        for (auto &[_, device] : m.second) {
            if (!device->IsUnRegisted()) {
                totalNum++;
            }
        }
    }
    EDM_LOGD(MODULE_DEV_MGR, "total device num is %{public}zu", totalNum);
    return totalNum;
}

void ExtDeviceManager::UnLoadSA(void)
{
    EDM_LOGI(MODULE_DEV_MGR, "UnLoadSA enter");
    if (GetTotalDeviceNum() != 0) {
        EDM_LOGI(MODULE_DEV_MGR, "not need unload");
        return;
    }
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "get samgr failed");
        return;
    }

    auto saObj = samgrProxy->CheckSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
    if (saObj == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "sa has unloaded");
        return;
    }

    auto ret = samgrProxy->UnloadSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "unload failed");
    }
}

void ExtDeviceManager::UnLoadSelf(void)
{
    unloadSelftimer_.Unregister(unloadSelftimerId_);
    unloadSelftimer_.Shutdown();
    if (GetTotalDeviceNum() != 0) {
        EDM_LOGI(MODULE_DEV_MGR, "not need unload");
        return;
    }

    if (auto ret = unloadSelftimer_.Setup(); ret != Utils::TIMER_ERR_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "set up timer failed %{public}u", ret);
        return;
    }

    auto task = []() {
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy == nullptr) {
            EDM_LOGE(MODULE_DEV_MGR, "get samgr failed");
            return;
        }

        auto saObj = samgrProxy->CheckSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
        if (saObj == nullptr) {
            EDM_LOGE(MODULE_DEV_MGR, "sa has unloaded");
            return;
        }

        auto ret = samgrProxy->UnloadSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
        if (ret != EDM_OK) {
            EDM_LOGE(MODULE_DEV_MGR, "unload failed");
        }
    };
    unloadSelftimerId_ = unloadSelftimer_.Register(task, UNLOAD_SA_TIMER_INTERVAL, true);
}

std::shared_ptr<Device> ExtDeviceManager::QueryDeviceByDeviceID(uint64_t deviceId)
{
    BusType busType = *reinterpret_cast<BusType *>(&deviceId);
    EDM_LOGI(MODULE_DEV_MGR, "the busType: %{public}d", static_cast<uint32_t>(busType));
    auto deviceMapIter = deviceMap_.find(busType);
    if (deviceMapIter == deviceMap_.end()) {
        EDM_LOGE(MODULE_DEV_MGR, "can not find device by %{public}d busType", static_cast<uint32_t>(busType));
        return nullptr;
    }

    const auto &devices = deviceMapIter->second;
    auto deviceIter = devices.find(deviceId);
    if (deviceIter == devices.end()) {
        EDM_LOGE(MODULE_DEV_MGR, "can not find device by %{public}016" PRIX64 " deviceId", deviceId);
        return nullptr;
    }

    EDM_LOGI(MODULE_DEV_MGR, "find device by %{public}016" PRIX64 " deviceId sucessfully", deviceId);
    return deviceIter->second;
}

int32_t ExtDeviceManager::ConnectDevice(uint64_t deviceId, const sptr<IDriverExtMgrCallback> &connectCallback)
{
    // find device by deviceId
    lock_guard<mutex> lock(deviceMapMutex_);
    std::shared_ptr<Device> device = QueryDeviceByDeviceID(deviceId);
    if (device == nullptr) {
        EDM_LOGI(MODULE_DEV_MGR, "failed to find device with %{public}016" PRIX64 " deviceId", deviceId);
        return EDM_NOK;
    }
    return device->Connect(connectCallback);
}

int32_t ExtDeviceManager::DisConnectDevice(uint64_t deviceId)
{
    lock_guard<mutex> lock(deviceMapMutex_);
    std::shared_ptr<Device> device = QueryDeviceByDeviceID(deviceId);
    if (device == nullptr) {
        EDM_LOGI(MODULE_DEV_MGR, "failed to find device with %{public}016" PRIX64 " deviceId", deviceId);
        return EDM_NOK;
    }

    return device->Disconnect();
}
} // namespace ExternalDeviceManager
} // namespace OHOS