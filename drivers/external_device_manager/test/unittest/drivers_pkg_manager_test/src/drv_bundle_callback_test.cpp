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
 
#include <gtest/gtest.h>

#include "drv_bundle_state_callback.h"
namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing::ext;
using namespace OHOS::ExternalDeviceManager;

class DrvBundleStateCallbackTest : public testing::Test {
public:
    DrvBundleStateCallback drvbundleInstance;
    void SetUp() override
    {
        cout << "DrvBundleStateCallbackTest SetUp" << endl;
    }
    void TearDown() override
    {
        cout << "DrvBundleStateCallbackTest TearDown" << endl;
    }
};

HWTEST_F(DrvBundleStateCallbackTest, DrvBundleCallback_CheckPermissio_Test, TestSize.Level1)
{
    bool ret = drvbundleInstance.CheckBundleMgrProxyPermission();
    EXPECT_EQ(true, ret);
    cout << "DrvBundleCallback_CheckPermissio_Test" << endl;
}

HWTEST_F(DrvBundleStateCallbackTest, DrvBundleCallback_GetAllInfos_Test, TestSize.Level1)
{
    bool ret = drvbundleInstance.GetAllDriverInfos();
    EXPECT_EQ(true, ret);
    cout << "Ptr DrvBundleCallback_GetAllInfos_Test" << endl;
}

HWTEST_F(DrvBundleStateCallbackTest, DrvBundleCallback_GetStiching_Test, TestSize.Level1)
{
    string stiching = drvbundleInstance.GetStiching();
    EXPECT_NE(true, stiching.empty());
    cout << "Ptr DrvBundleCallback_GetStiching_Test" << endl;
}

class DrvBundleStateCallbackPtrTest : public testing::Test {
public:
    DrvBundleStateCallback *drvbundleInstance = nullptr;
    void SetUp() override
    {
        drvbundleInstance = new DrvBundleStateCallback();
        cout << "DrvBundleStateCallbackPtrTest SetUp" << endl;
    }
    void TearDown() override
    {
        if (drvbundleInstance != nullptr) {
            delete drvbundleInstance;
            drvbundleInstance = nullptr;
        }
        cout << "DrvBundleStateCallbackPtrTest TearDown" << endl;
    }
};

HWTEST_F(DrvBundleStateCallbackPtrTest, DrvBundleCallback_New_Test, TestSize.Level1)
{
    EXPECT_NE(nullptr, drvbundleInstance);
    cout << "DrvBundleCallback_New_Test" << endl;
}

HWTEST_F(DrvBundleStateCallbackPtrTest, DrvBundleCallback_Delete_Test, TestSize.Level1)
{
    if (drvbundleInstance != nullptr) {
        delete drvbundleInstance;
        drvbundleInstance = nullptr;
        EXPECT_EQ(nullptr, drvbundleInstance);
    }
    cout << "DrvBundleCallback_Delete_Test" << endl;
}
}
}