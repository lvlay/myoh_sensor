/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef WIFI_LEGACY_HAL_H
#define WIFI_LEGACY_HAL_H

#include <condition_variable>
#include <functional>
#include <map>
#include <thread>
#include <vector>

#include "wifi_hal.h"
#include "iface_tool.h"
#include "hdi_sync_util.h"
#include "v1_0/chip_types.h"
#include "callback_handler.h"

namespace OHOS {
namespace HDI {
namespace Wlan {
namespace Chip {
namespace V1_0 {

using OnVendorHalRestartCallback = std::function<void(const std::string&)>;
class WifiVendorHal {
public:
    WifiVendorHal(const std::weak_ptr<IfaceTool> ifaceTool,
        const WifiHalFn& fn, bool isPrimary);
    virtual ~WifiVendorHal() = default;
    virtual WifiError Initialize();
    WifiError Start();
    virtual WifiError Stop(std::unique_lock<std::recursive_mutex>* lock,
        const std::function<void()>& onCompleteCallback);
    WifiError GetSupportedFeatureSet(const std::string& ifaceName, uint32_t& capabilities);
    WifiError CreateVirtualInterface(const std::string& ifname, HalIfaceType iftype);
    WifiError DeleteVirtualInterface(const std::string& ifname);
    std::pair<WifiError, std::vector<uint32_t>> GetValidFrequenciesForBand(
        const std::string& ifaceName, int band);
    WifiError SetCountryCode(const std::string& ifaceName, const std::string& code);
    WifiError GetChipCaps(const std::string& ifaceName, uint32_t& capabilities);
    WifiError RegisterRestartCallback(
        const OnVendorHalRestartCallback& onRestartCallback);
    std::pair<WifiError, int> GetPowerMode(const std::string& ifaceName);
    WifiError SetPowerMode(const std::string& ifaceName, int mode);
    WifiError IsSupportCoex(bool& isCoex);
    WifiError StartScan(const std::string& ifaceName, const ScanParams& params);
    WifiError StartPnoScan(const std::string& ifaceName, const PnoScanParams& pnoParams);
    WifiError StopPnoScan(const std::string& ifaceName);
    WifiError GetScanInfos(const std::string& ifaceName,
        std::vector<ScanResultsInfo>& scanResultsInfo);
    WifiError EnablePowerMode(const std::string& ifaceName, int mode);
    WifiError GetSignalPollInfo(const std::string& ifaceName,
        SignalPollResult& signalPollResult);
    WifiError SetDpiMarkRule(int32_t uid, int32_t protocol, int32_t enable);
    WifiError RegisterIfaceCallBack(const std::string& ifaceName, const sptr<IChipIfaceCallback>& chipIfaceCallback);
    WifiError UnRegisterIfaceCallBack(const std::string& ifaceName, const sptr<IChipIfaceCallback>& chipIfaceCallback);
    static void OnAsyncGscanFullResult(int event);
    static void OnAsyncRssiReport(int32_t index, int32_t c0Rssi, int32_t c1Rssi);
    WifiError SetTxPower(const std::string& ifaceName, int mode);

private:
    WifiError RetrieveIfaceHandles();
    wifiInterfaceHandle GetIfaceHandle(const std::string& ifaceName);
    void RunEventLoop();
    void Invalidate();
    WifiError HandleIfaceChangeStatus(const std::string& ifname, WifiError status);
    WifiHalFn globalFuncTable_;
    wifiHandle globalHandle_;
    std::map<std::string, wifiInterfaceHandle> ifaceNameHandle_;
    std::atomic<bool> awaitingEventLoopTermination_;
    std::condition_variable_any stopWaitCv_;
    bool isInited_;
    std::weak_ptr<IfaceTool> ifaceTool_;
    bool isPrimary_;
    static CallbackHandler<IChipIfaceCallback> vendorHalCbHandler_;
};
    
} // namespace v1_0
} // namespace Chip
} // namespace Wlan
} // namespace HDI
} // namespace OHOS

#endif
