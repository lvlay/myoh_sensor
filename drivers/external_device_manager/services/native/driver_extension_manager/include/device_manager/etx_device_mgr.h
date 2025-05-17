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

#ifndef DEVICE_MANAGER_ETX_DEVICE_MGR_H
#define DEVICE_MANAGER_ETX_DEVICE_MGR_H

#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include "device.h"
#include "ext_object.h"
#include "single_instance.h"
#include "timer.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
class ExtDeviceManager final {
    DECLARE_SINGLE_INSTANCE_BASE(ExtDeviceManager);

public:
    ~ExtDeviceManager();
    int32_t Init();
    int32_t RegisterDevice(shared_ptr<DeviceInfo> devInfo);
    int32_t UnRegisterDevice(const shared_ptr<DeviceInfo> devInfo);
    vector<shared_ptr<DeviceInfo>> QueryDevice(const BusType busType);
    vector<shared_ptr<Device>> QueryAllDevices();
    vector<shared_ptr<Device>> QueryDevicesById(const uint64_t deviceId);
    int32_t ConnectDevice(uint64_t deviceId, const sptr<IDriverExtMgrCallback> &connectCallback);
    int32_t DisConnectDevice(uint64_t deviceId);
    void RemoveDeviceOfDeviceMap(shared_ptr<Device> device);
    void UnLoadSA(void);
    std::unordered_set<uint64_t> DeleteBundlesOfBundleInfoMap(const std::string &bundleName = "");
    void MatchDriverInfos(std::unordered_set<uint64_t> deviceIds, bool isDriversReset = false);
    void ClearMatchedDrivers(const int32_t userId);

private:
    ExtDeviceManager() = default;
    void PrintMatchDriverMap();
    int32_t AddDevIdOfBundleInfoMap(shared_ptr<Device> device, string &bundleInfo);
    int32_t RemoveDevIdOfBundleInfoMap(shared_ptr<Device> device, string &bundleInfo);
    std::shared_ptr<Device> QueryDeviceByDeviceID(uint64_t deviceId);
    void UnLoadSelf(void);
    size_t GetTotalDeviceNum(void) const;
    unordered_map<BusType, unordered_map<uint64_t, shared_ptr<Device>>> deviceMap_;
    unordered_map<string, unordered_set<uint64_t>> bundleMatchMap_; // driver matching table
    mutex deviceMapMutex_;
    mutex bundleMatchMapMutex_;
    Utils::Timer unloadSelftimer_ {"unLoadSelfTimer"};
    uint32_t unloadSelftimerId_ {UINT32_MAX};
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // DEVICE_MANAGER_ETX_DEVICE_MGR_H