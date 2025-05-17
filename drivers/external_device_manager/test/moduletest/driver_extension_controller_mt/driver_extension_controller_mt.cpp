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

#include <unistd.h>
#include "iostream"
#include "hilog_wrapper.h"
#include "string_ex.h"
#include "iservice_registry.h"
#include "accesstoken_kit.h"
#include "token_setproc.h"
#include "nativetoken_kit.h"
#include "driver_extension_controller.h"
namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace OHOS::Security::AccessToken;
constexpr uint32_t PARAM_COUNT_NEED = 4;
constexpr uint32_t PARAM_INDEX_OF_ACTION      = 1;
constexpr uint32_t PARAM_INDEX_OF_BUNDLENAME  = 2;
constexpr uint32_t PARAM_INDEX_OF_ABILITYNAME = 3;
constexpr int32_t MAX_WAIT_TIME_SECOND = 10;
TokenInfoParams g_sysInfoInstance = {
    .dcapsNum = 0,
    .permsNum = 0,
    .aclsNum = 0,
    .dcaps = nullptr,
    .perms = nullptr,
    .acls = nullptr,
    .processName = "usb_manager",
    .aplStr = "system_basic",
};
static void SetTestCaseNative(TokenInfoParams *infoInstance)
{
    uint64_t tokenId = GetAccessTokenId(infoInstance);
    int ret = SetSelfTokenID(tokenId);
    if (ret == 0) {
        cout << "SetSelfTokenID success" << endl;
    } else {
        cout << "SetSelfTokenID fail" << endl;
    }
    AccessTokenKit::ReloadNativeTokenInfo();
}

class DriverExtensionControllerMt {
public:
    int TestEntry(int argc, char **argv)
    {
        int ret = 0;
        cout << "[info]driver_extension_controller_mt" << endl;
        cout << "[usage]: start/stop/connect bundleName ExtensionAbilityName" << endl;
        if (argc != PARAM_COUNT_NEED) {
            cout << "error! wrong param count" << endl;
            return 0;
        }
        auto &drvExtCtrl = DriverExtensionController::GetInstance();
        string action(argv[PARAM_INDEX_OF_ACTION]);
        string bundleName(argv[PARAM_INDEX_OF_BUNDLENAME]);
        string abilityName(argv[PARAM_INDEX_OF_ABILITYNAME]);
        cout << "args: "<< action << ", " << bundleName << ", " << abilityName << endl;
        SetTestCaseNative(&g_sysInfoInstance);
        if (action == "start") {
            cout << "begin to connect extension ability" << endl;
            ret = drvExtCtrl.StartDriverExtension(bundleName, abilityName);
        } else if (action == "stop") {
            cout << "begin to stop extension ability" << endl;
            ret = drvExtCtrl.StopDriverExtension(bundleName, abilityName);
        } else if (action == "connect") {
            ConnectAbilityTest(bundleName, abilityName);
        } else {
            cout << "wrong param! please check!" << endl;
            return 0;
        }
        if (ret == 0) {
            cout << "driver_extension_controller_mt sucess" << endl;
        } else {
            cout << "driver_extension_controller_mt fail , ret = " << ret << endl;
        }
        cout << "delay 10s to exit" << endl;
        for (int i = 0; i < MAX_WAIT_TIME_SECOND; i++) {
            sleep(1);
        }
        cout << "exit 0" << endl;
        return 0;
        }
private:
    void ConnectAbilityTest(const string bundleName, const string abilityName)
    {
        auto &drvExtCtrl = DriverExtensionController::GetInstance();
        shared_ptr<ConCb> conCb = make_shared<ConCb>();
        cout << "begin to Connect extension ability" << endl;
        auto ret = drvExtCtrl.ConnectDriverExtension(bundleName, abilityName, conCb);
        if (ret == 0) {
            cout << "connect suncess, wait 5 second to disconnect" << endl;
            for (int i = 0; i < MAX_WAIT_TIME_SECOND; i++) {
                sleep(1);
                if (conCb->IsConnectDone()) {
                    cout << "connectDone suncess, remoteObj = " << conCb->GetRemoteObj() << endl;
                    break;
                }
            }
            ret = drvExtCtrl.DisconnectDriverExtension(bundleName, abilityName, conCb);
            for (int i = 0; i < MAX_WAIT_TIME_SECOND; i++) {
                sleep(1);
                if (!conCb->IsConnectDone()) {
                    cout << "DisconnectDone sucess" << endl;
                    break;
                }
            }
        }
    };
    class ConCb : public IDriverExtensionConnectCallback {
    public:
        ConCb() { };
        int32_t OnConnectDone(const sptr<IRemoteObject> &remote, int resultCode) override
        {
            cout << "ConCb OnConnectDone, "<<  remote.GetRefPtr() << ", " << resultCode << endl;
            return 0;
        };
        int32_t OnDisconnectDone(int resultCode) override
        {
            cout << "ConCb OnDisconnectDone, " << resultCode << endl;
            return 0;
        }
    };
};
}
}
using namespace OHOS::ExternalDeviceManager;
int main(int argc, char **argv)
{
    return DriverExtensionControllerMt().TestEntry(argc, argv);
}
