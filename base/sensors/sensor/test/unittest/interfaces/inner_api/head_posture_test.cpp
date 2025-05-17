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

#include <atomic>
#include <thread>

#include <gtest/gtest.h>

#include "sensor_agent.h"
#include "sensor_agent_type.h"
#include "sensor_errors.h"

#undef LOG_TAG
#define LOG_TAG "HeadPostureTest"

namespace OHOS {
namespace Sensors {
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
std::atomic_bool g_existHeadPosture = false;
} // namespace

class HeadPostureTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HeadPostureTest::SetUpTestCase()
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
        if (sensorInfo[i].sensorId == SENSOR_TYPE_ID_HEADPOSTURE) {
            g_existHeadPosture = true;
            SEN_HILOGD("Exist head posture sensor");
            break;
        }
    }
    SEN_HILOGD("Not exist head posture sensor");
}

void HeadPostureTest::TearDownTestCase() {}

void HeadPostureTest::SetUp() {}

void HeadPostureTest::TearDown() {}

void HeadPostureDataCallbackImpl(SensorEvent *event)
{
    if (event == nullptr) {
        SEN_HILOGE("event is nullptr");
        return;
    }
    if (event[0].data == nullptr) {
        SEN_HILOGE("event[0].data is nullptr");
        return;
    }
    if (event[0].dataLen < sizeof(HeadPostureData)) {
        SEN_HILOGE("Event dataLen less than head posture data size, event.dataLen:%{public}u", event[0].dataLen);
        return;
    }
    HeadPostureData *headPostureData = reinterpret_cast<HeadPostureData *>(event[0].data);
    if (headPostureData == nullptr) {
        SEN_HILOGE("headPostureData is nullptr");
        return;
    }
    SEN_HILOGD("sensorId:%{public}d, version:%{public}d, dataLen:%{public}u, "
        "w:%{public}f, x:%{public}f, y:%{public}f, z:%{public}f", event[0].sensorTypeId, event[0].version,
        event[0].dataLen, headPostureData->w, headPostureData->x, headPostureData->y, headPostureData->z);
}

void HeadPostureDataCallbackImpl2(SensorEvent *event)
{
    if (event == nullptr) {
        SEN_HILOGE("event is nullptr");
        return;
    }
    if (event[0].data == nullptr) {
        SEN_HILOGE("event[0].data is nullptr");
        return;
    }
    if (event[0].dataLen < sizeof(HeadPostureData)) {
        SEN_HILOGE("Event dataLen less than head posture data size, event.dataLen:%{public}u", event[0].dataLen);
        return;
    }
    HeadPostureData *headPostureData = reinterpret_cast<HeadPostureData *>(event[0].data);
    if (headPostureData == nullptr) {
        SEN_HILOGE("headPostureData is nullptr");
        return;
    }
    SEN_HILOGD("sensorId:%{public}d, version:%{public}d, dataLen:%{public}u, "
        "w:%{public}f, x:%{public}f, y:%{public}f, z:%{public}f", event[0].sensorTypeId, event[0].version,
        event[0].dataLen, headPostureData->w, headPostureData->x, headPostureData->y, headPostureData->z);
}

HWTEST_F(HeadPostureTest, HeadPostureTest_001, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_001 enter");
    if (g_existHeadPosture) {
        ASSERT_NE(ActivateSensor(SENSOR_TYPE_ID_HEADPOSTURE, nullptr), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_002, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_002 enter");
    if (g_existHeadPosture) {
        SensorUser user;
        user.callback = HeadPostureDataCallbackImpl;
        ASSERT_NE(ActivateSensor(-1, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_003, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_003 enter");
    if (g_existHeadPosture) {
        SensorUser user;
        user.callback = nullptr;
        ASSERT_NE(ActivateSensor(SENSOR_TYPE_ID_HEADPOSTURE, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_004, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_004 enter");
    if (g_existHeadPosture) {
        ASSERT_NE(DeactivateSensor(SENSOR_TYPE_ID_HEADPOSTURE, nullptr), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_005, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_005 enter");
    if (g_existHeadPosture) {
        SensorUser user;
        user.callback = HeadPostureDataCallbackImpl;
        ASSERT_NE(DeactivateSensor(-1, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_006, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_006 enter");
    if (g_existHeadPosture) {
        SensorUser user;
        user.callback = nullptr;
        ASSERT_NE(DeactivateSensor(SENSOR_TYPE_ID_HEADPOSTURE, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_007, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_007 enter");
    if (g_existHeadPosture) {
        ASSERT_NE(SetBatch(SENSOR_TYPE_ID_HEADPOSTURE, nullptr, 10000000, 10000000), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_008, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_008 enter");
    if (g_existHeadPosture) {
        SensorUser user;
        user.callback = HeadPostureDataCallbackImpl;
        ASSERT_NE(SetBatch(-1, &user, 10000000, 10000000), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_009, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_009 enter");
    if (g_existHeadPosture) {
        SensorUser user;
        user.callback = nullptr;
        ASSERT_NE(SetBatch(SENSOR_TYPE_ID_HEADPOSTURE, &user, 10000000, 10000000), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_010, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_010 enter");
    if (g_existHeadPosture) {
        SensorUser user;
        user.callback = HeadPostureDataCallbackImpl;
        ASSERT_NE(SetBatch(SENSOR_TYPE_ID_HEADPOSTURE, &user, -1, -1), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_011, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_011 enter");
    if (g_existHeadPosture) {
        ASSERT_NE(SubscribeSensor(SENSOR_TYPE_ID_HEADPOSTURE, nullptr), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_012, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_012 enter");
    if (g_existHeadPosture) {
        SensorUser user;
        user.callback = HeadPostureDataCallbackImpl;
        ASSERT_NE(SubscribeSensor(-1, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_013, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_013 enter");
    if (g_existHeadPosture) {
        SensorUser user;
        user.callback = nullptr;
        ASSERT_NE(SubscribeSensor(SENSOR_TYPE_ID_HEADPOSTURE, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_014, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_014 enter");
    if (g_existHeadPosture) {
        ASSERT_NE(UnsubscribeSensor(SENSOR_TYPE_ID_HEADPOSTURE, nullptr), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_015, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_015 enter");
    if (g_existHeadPosture) {
        SensorUser user;
        user.callback = HeadPostureDataCallbackImpl;
        ASSERT_NE(UnsubscribeSensor(-1, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_016, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_016 enter");
    if (g_existHeadPosture) {
        SensorUser user;
        user.callback = nullptr;
        ASSERT_NE(UnsubscribeSensor(SENSOR_TYPE_ID_HEADPOSTURE, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_017, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_017 enter");
    if (g_existHeadPosture) {
        SensorUser user;
        user.callback = HeadPostureDataCallbackImpl;
        ASSERT_EQ(SubscribeSensor(SENSOR_TYPE_ID_HEADPOSTURE, &user), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(SetBatch(SENSOR_TYPE_ID_HEADPOSTURE, &user, 10000000, 10000000), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(ActivateSensor(SENSOR_TYPE_ID_HEADPOSTURE, &user), OHOS::Sensors::SUCCESS);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(DeactivateSensor(SENSOR_TYPE_ID_HEADPOSTURE, &user), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(UnsubscribeSensor(SENSOR_TYPE_ID_HEADPOSTURE, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(HeadPostureTest, HeadPostureTest_018, TestSize.Level1)
{
    SEN_HILOGI("HeadPostureTest_018 enter");
    if (g_existHeadPosture) {
        SensorUser user;
        user.callback = HeadPostureDataCallbackImpl;
        ASSERT_EQ(SubscribeSensor(SENSOR_TYPE_ID_HEADPOSTURE, &user), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(SetBatch(SENSOR_TYPE_ID_HEADPOSTURE, &user, 10000000, 10000000), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(ActivateSensor(SENSOR_TYPE_ID_HEADPOSTURE, &user), OHOS::Sensors::SUCCESS);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(DeactivateSensor(SENSOR_TYPE_ID_HEADPOSTURE, &user), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(UnsubscribeSensor(SENSOR_TYPE_ID_HEADPOSTURE, &user), OHOS::Sensors::SUCCESS);

        SensorUser user2;
        user2.callback = HeadPostureDataCallbackImpl2;
        ASSERT_EQ(SubscribeSensor(SENSOR_TYPE_ID_HEADPOSTURE, &user2), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(SetBatch(SENSOR_TYPE_ID_HEADPOSTURE, &user2, 20000000, 20000000), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(ActivateSensor(SENSOR_TYPE_ID_HEADPOSTURE, &user2), OHOS::Sensors::SUCCESS);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(DeactivateSensor(SENSOR_TYPE_ID_HEADPOSTURE, &user2), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(UnsubscribeSensor(SENSOR_TYPE_ID_HEADPOSTURE, &user2), OHOS::Sensors::SUCCESS);
    }
}
} // namespace Sensors
} // namespace OHOS
