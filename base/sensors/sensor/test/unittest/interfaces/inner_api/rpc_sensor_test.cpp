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

#include <atomic>
#include <thread>

#include <gtest/gtest.h>

#include "sensor_agent.h"
#include "sensor_agent_type.h"
#include "sensor_errors.h"

#undef LOG_TAG
#define LOG_TAG "RPCSensorTest"

namespace OHOS {
namespace Sensors {
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
std::atomic_bool g_existRPC = false;
} // namespace

class RPCSensorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void RPCSensorTest::SetUpTestCase()
{
    SensorInfo *sensorInfo = nullptr;
    int32_t count = 0;
    int32_t ret = GetAllSensors(&sensorInfo, &count);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    if (sensorInfo == nullptr || count == 0) {
        SEN_HILOGE("sensorInfo is nullptr or count is 0");
        return;
    }
    for (int32_t i = 0; i < count; ++i) {
        if (sensorInfo[i].sensorId == SENSOR_TYPE_ID_RPC) {
            g_existRPC = true;
            SEN_HILOGD("Exist rpc sensor");
            break;
        }
    }
    SEN_HILOGD("Not exist rpc sensor");
}

void RPCSensorTest::TearDownTestCase() {}

void RPCSensorTest::SetUp() {}

void RPCSensorTest::TearDown() {}

void RPCDataCallbackImpl(SensorEvent *event)
{
    if (event == nullptr) {
        SEN_HILOGE("event is nullptr");
        return;
    }
    if (event[0].data == nullptr) {
        SEN_HILOGE("event[0].data is nullptr");
        return;
    }
    if (event[0].dataLen < sizeof(RPCData)) {
        SEN_HILOGE("Event dataLen less than rpc data size, event.dataLen:%{public}u", event[0].dataLen);
        return;
    }
    RPCData *rpcData = reinterpret_cast<struct RPCData *>(event[0].data);
    if (rpcData == nullptr) {
        SEN_HILOGE("rpcData is nullptr");
        return;
    }
    SEN_HILOGD("sensorId:%{public}d, version:%{public}d, dataLen:%{public}u,"
        "absorptionRatio:%{public}f, threshold:%{public}f, offset:%{public}f", event[0].sensorTypeId, event[0].version,
        event[0].dataLen, rpcData->absorptionRatio, rpcData->threshold, rpcData->offset);
}

void RPCDataCallbackImpl2(SensorEvent *event)
{
    if (event == nullptr) {
        SEN_HILOGE("event is nullptr");
        return;
    }
    if (event[0].data == nullptr) {
        SEN_HILOGE("event[0].data is nullptr");
        return;
    }
    if (event[0].dataLen < sizeof(RPCData)) {
        SEN_HILOGE("Event dataLen less than rpc data size, event.dataLen:%{public}u", event[0].dataLen);
        return;
    }
    RPCData *rpcData = reinterpret_cast<struct RPCData *>(event[0].data);
    if (rpcData == nullptr) {
        SEN_HILOGE("rpcData is nullptr");
        return;
    }
    SEN_HILOGD("sensorId:%{public}d, version:%{public}d, dataLen:%{public}u,"
        "absorptionRatio:%{public}f, threshold:%{public}f, offset:%{public}f", event[0].sensorTypeId, event[0].version,
        event[0].dataLen, rpcData->absorptionRatio, rpcData->threshold, rpcData->offset);
}

HWTEST_F(RPCSensorTest, RPCSensorTest_001, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_001 enter");
    if (g_existRPC) {
        ASSERT_NE(ActivateSensor(SENSOR_TYPE_ID_RPC, nullptr), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_002, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_002 enter");
    if (g_existRPC) {
        SensorUser user;
        user.callback = RPCDataCallbackImpl;
        ASSERT_NE(ActivateSensor(-1, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_003, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_003 enter");
    if (g_existRPC) {
        SensorUser user;
        user.callback = nullptr;
        ASSERT_NE(ActivateSensor(SENSOR_TYPE_ID_RPC, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_004, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_004 enter");
    if (g_existRPC) {
        ASSERT_NE(DeactivateSensor(SENSOR_TYPE_ID_RPC, nullptr), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_005, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_005 enter");
    if (g_existRPC) {
        SensorUser user;
        user.callback = RPCDataCallbackImpl;
        ASSERT_NE(DeactivateSensor(-1, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_006, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_006 enter");
    if (g_existRPC) {
        SensorUser user;
        user.callback = nullptr;
        ASSERT_NE(DeactivateSensor(SENSOR_TYPE_ID_RPC, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_007, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_007 enter");
    if (g_existRPC) {
        ASSERT_NE(SetBatch(SENSOR_TYPE_ID_RPC, nullptr, 100000000, 100000000), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_008, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_008 enter");
    if (g_existRPC) {
        SensorUser user;
        user.callback = RPCDataCallbackImpl;
        ASSERT_NE(SetBatch(-1, &user, 100000000, 100000000), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_009, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_009 enter");
    if (g_existRPC) {
        SensorUser user;
        user.callback = nullptr;
        ASSERT_NE(SetBatch(SENSOR_TYPE_ID_RPC, &user, 100000000, 100000000), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_010, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_010 enter");
    if (g_existRPC) {
        SensorUser user;
        user.callback = RPCDataCallbackImpl;
        ASSERT_NE(SetBatch(SENSOR_TYPE_ID_RPC, &user, -1, -1), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_011, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_011 enter");
    if (g_existRPC) {
        ASSERT_NE(SubscribeSensor(SENSOR_TYPE_ID_RPC, nullptr), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_012, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_012 enter");
    if (g_existRPC) {
        SensorUser user;
        user.callback = RPCDataCallbackImpl;
        ASSERT_NE(SubscribeSensor(-1, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_013, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_013 enter");
    if (g_existRPC) {
        SensorUser user;
        user.callback = nullptr;
        ASSERT_NE(SubscribeSensor(SENSOR_TYPE_ID_RPC, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_014, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_014 enter");
    if (g_existRPC) {
        ASSERT_NE(UnsubscribeSensor(SENSOR_TYPE_ID_RPC, nullptr), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_015, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_015 enter");
    if (g_existRPC) {
        SensorUser user;
        user.callback = RPCDataCallbackImpl;
        ASSERT_NE(UnsubscribeSensor(-1, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_016, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_016 enter");
    if (g_existRPC) {
        SensorUser user;
        user.callback = nullptr;
        ASSERT_NE(UnsubscribeSensor(SENSOR_TYPE_ID_RPC, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_017, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_017 enter");
    if (g_existRPC) {
        SensorUser user;
        user.callback = RPCDataCallbackImpl;
        ASSERT_EQ(SubscribeSensor(SENSOR_TYPE_ID_RPC, &user), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(SetBatch(SENSOR_TYPE_ID_RPC, &user, 100000000, 100000000), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(ActivateSensor(SENSOR_TYPE_ID_RPC, &user), OHOS::Sensors::SUCCESS);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(DeactivateSensor(SENSOR_TYPE_ID_RPC, &user), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(UnsubscribeSensor(SENSOR_TYPE_ID_RPC, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(RPCSensorTest, RPCSensorTest_018, TestSize.Level1)
{
    SEN_HILOGI("RPCSensorTest_018 enter");
    if (g_existRPC) {
        SensorUser user;
        user.callback = RPCDataCallbackImpl;
        ASSERT_EQ(SubscribeSensor(SENSOR_TYPE_ID_RPC, &user), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(SetBatch(SENSOR_TYPE_ID_RPC, &user, 100000000, 100000000), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(ActivateSensor(SENSOR_TYPE_ID_RPC, &user), OHOS::Sensors::SUCCESS);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(DeactivateSensor(SENSOR_TYPE_ID_RPC, &user), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(UnsubscribeSensor(SENSOR_TYPE_ID_RPC, &user), OHOS::Sensors::SUCCESS);

        SensorUser user2;
        user2.callback = RPCDataCallbackImpl2;
        ASSERT_EQ(SubscribeSensor(SENSOR_TYPE_ID_RPC, &user2), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(SetBatch(SENSOR_TYPE_ID_RPC, &user2, 200000000, 200000000), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(ActivateSensor(SENSOR_TYPE_ID_RPC, &user2), OHOS::Sensors::SUCCESS);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(DeactivateSensor(SENSOR_TYPE_ID_RPC, &user2), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(UnsubscribeSensor(SENSOR_TYPE_ID_RPC, &user2), OHOS::Sensors::SUCCESS);
    }
}
} // namespace Sensors
} // namespace OHOS
