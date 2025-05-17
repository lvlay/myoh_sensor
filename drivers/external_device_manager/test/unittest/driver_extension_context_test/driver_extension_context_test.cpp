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

#define private public
#include "ability_manager_client.h"
#include "driver_extension_context.h"
#undef private

#include "ability_connection.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AbilityRuntime {
class DriverExtensionContextTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DriverExtensionContextTest::SetUpTestCase(void)
{}
void DriverExtensionContextTest::TearDownTestCase(void)
{}
void DriverExtensionContextTest::SetUp(void)
{}
void DriverExtensionContextTest::TearDown(void)
{}

/*
 * Feature: DriverExtensionContext
 * Function: UpdateDriverState
 * SubFunction: NA
 * FunctionPoints: DriverExtensionContextTest UpdateDriverState
 * EnvConditions: NA
 * CaseDescription: Verify UpdateDriverState
 */
HWTEST_F(DriverExtensionContextTest, driver_extension_context_updateDriverState_001, TestSize.Level1)
{
    DriverExtensionContext driverExtensionContextTest;
    ErrCode result = driverExtensionContextTest.UpdateDriverState();
    EXPECT_EQ(ERR_OK, result);
}
}
}