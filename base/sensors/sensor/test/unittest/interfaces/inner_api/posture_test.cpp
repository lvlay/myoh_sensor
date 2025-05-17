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
#define LOG_TAG "PostureTest"

namespace OHOS {
namespace Sensors {
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
constexpr float ANGLE_MAX = 180.0F;
constexpr float ANGLE_MIN = 0.0F;
std::atomic_bool g_existPosture = false;
} // namespace

class PostureTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void PostureTest::SetUpTestCase()
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
        if (sensorInfo[i].sensorId == SENSOR_TYPE_ID_POSTURE) {
            g_existPosture = true;
            SEN_HILOGD("Exist posture sensor");
            break;
        }
    }
    SEN_HILOGD("Not exist posture sensor");
}

void PostureTest::TearDownTestCase() {}

void PostureTest::SetUp() {}

void PostureTest::TearDown() {}

void PostureDataCallbackImpl(SensorEvent *event)
{
    if (event == nullptr) {
        SEN_HILOGE("event is nullptr");
        return;
    }
    if (event[0].data == nullptr) {
        SEN_HILOGE("event[0].data is nullptr");
        return;
    }
    if (event[0].dataLen < sizeof(PostureData)) {
        SEN_HILOGE("Event dataLen less than posture data size, event.dataLen:%{public}u", event[0].dataLen);
        return;
    }
    PostureData *postureData = reinterpret_cast<PostureData *>(event[0].data);
    float angle = postureData->angle;
    if ((angle < ANGLE_MIN && std::fabs(angle - ANGLE_MIN) > std::numeric_limits<float>::epsilon())
        || (angle > ANGLE_MAX && std::fabs(angle - ANGLE_MAX) > std::numeric_limits<float>::epsilon())) {
        SEN_HILOGE("Invalid posture angle, angle:%{public}f", angle);
        return;
    }
    SEN_HILOGD("sensorId:%{public}d, version:%{public}d, dataLen:%{public}u, gxm:%{public}f, "
        "gym:%{public}f, gzm:%{public}f, gxs:%{public}f, gys:%{public}f, gzs:%{public}f, angle:%{public}f",
        event[0].sensorTypeId, event[0].version, event[0].dataLen, postureData->gxm, postureData->gym,
        postureData->gzm, postureData->gxs, postureData->gys, postureData->gzs, postureData->angle);
}

void PostureDataCallbackImpl2(SensorEvent *event)
{
    if (event == nullptr) {
        SEN_HILOGE("event is nullptr");
        return;
    }
    if (event[0].data == nullptr) {
        SEN_HILOGE("event[0].data is nullptr");
        return;
    }
    if (event[0].dataLen < sizeof(PostureData)) {
        SEN_HILOGE("Event dataLen less than posture data size, event.dataLen:%{public}u", event[0].dataLen);
        return;
    }
    PostureData *postureData = reinterpret_cast<PostureData *>(event[0].data);
    float angle = postureData->angle;
    if ((std::fabs(angle - ANGLE_MIN) > std::numeric_limits<float>::epsilon() && angle < ANGLE_MIN)
        || (std::fabs(angle - ANGLE_MAX) > std::numeric_limits<float>::epsilon() && angle > ANGLE_MAX)) {
        SEN_HILOGE("Invalid posture angle, angle:%{public}f", angle);
        return;
    }
    SEN_HILOGD("sensorId:%{public}d, version:%{public}d, dataLen:%{public}u, gxm:%{public}f, "
        "gym:%{public}f, gzm:%{public}f, gxs:%{public}f, gys:%{public}f, gzs:%{public}f, angle:%{public}f",
        event[0].sensorTypeId, event[0].version, event[0].dataLen, postureData->gxm, postureData->gym,
        postureData->gzm, postureData->gxs, postureData->gys, postureData->gzs, postureData->angle);
}

HWTEST_F(PostureTest, PostureTest_001, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_001 enter");
    if (g_existPosture) {
        int32_t ret = ActivateSensor(SENSOR_TYPE_ID_POSTURE, nullptr);
        ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_002, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_002 enter");
    if (g_existPosture) {
        SensorUser user;
        user.callback = PostureDataCallbackImpl;
        int32_t ret = ActivateSensor(-1, &user);
        ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_003, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_003 enter");
    if (g_existPosture) {
        SensorUser user;
        user.callback = nullptr;
        int32_t ret = ActivateSensor(SENSOR_TYPE_ID_POSTURE, &user);
        ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_004, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_004 enter");
    if (g_existPosture) {
        int32_t ret = DeactivateSensor(SENSOR_TYPE_ID_POSTURE, nullptr);
        ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_005, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_005 enter");
    if (g_existPosture) {
        SensorUser user;
        user.callback = PostureDataCallbackImpl;
        int32_t ret = DeactivateSensor(-1, &user);
        ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_006, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_006 enter");
    if (g_existPosture) {
        SensorUser user;
        user.callback = nullptr;
        int32_t ret = DeactivateSensor(SENSOR_TYPE_ID_POSTURE, &user);
        ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_007, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_007 enter");
    if (g_existPosture) {
        int32_t ret = SetBatch(SENSOR_TYPE_ID_POSTURE, nullptr, 10000000, 10000000);
        ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_008, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_008 enter");
    if (g_existPosture) {
        SensorUser user;
        user.callback = PostureDataCallbackImpl;
        int32_t ret = SetBatch(-1, &user, 10000000, 10000000);
        ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_009, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_009 enter");
    if (g_existPosture) {
        SensorUser user;
        user.callback = nullptr;
        int32_t ret = SetBatch(SENSOR_TYPE_ID_POSTURE, &user, 10000000, 10000000);
        ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_010, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_010 enter");
    if (g_existPosture) {
        SensorUser user;
        user.callback = PostureDataCallbackImpl;
        int32_t ret = SetBatch(SENSOR_TYPE_ID_POSTURE, &user, -1, -1);
        ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_011, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_011 enter");
    if (g_existPosture) {
        int32_t ret = SubscribeSensor(SENSOR_TYPE_ID_POSTURE, nullptr);
        ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_012, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_012 enter");
    if (g_existPosture) {
        SensorUser user;
        user.callback = PostureDataCallbackImpl;
        int32_t ret = SubscribeSensor(-1, &user);
        ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_013, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_013 enter");
    if (g_existPosture) {
        SensorUser user;
        user.callback = nullptr;
        int32_t ret = SubscribeSensor(SENSOR_TYPE_ID_POSTURE, &user);
        ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_014, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_014 enter");
    if (g_existPosture) {
        int32_t ret = UnsubscribeSensor(SENSOR_TYPE_ID_POSTURE, nullptr);
        ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_015, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_015 enter");
    if (g_existPosture) {
        SensorUser user;
        user.callback = PostureDataCallbackImpl;
        int32_t ret = UnsubscribeSensor(-1, &user);
        ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_016, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_016 enter");
    if (g_existPosture) {
        SensorUser user;
        user.callback = nullptr;
        int32_t ret = UnsubscribeSensor(SENSOR_TYPE_ID_POSTURE, &user);
        ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_017, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_017 enter");
    if (g_existPosture) {
        SensorUser user;
        user.callback = PostureDataCallbackImpl;
        int32_t ret = SubscribeSensor(SENSOR_TYPE_ID_POSTURE, &user);
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
        ret = SetBatch(SENSOR_TYPE_ID_POSTURE, &user, 10000000, 10000000);
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
        ret = ActivateSensor(SENSOR_TYPE_ID_POSTURE, &user);
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
        ret = DeactivateSensor(SENSOR_TYPE_ID_POSTURE, &user);
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
        ret = UnsubscribeSensor(SENSOR_TYPE_ID_POSTURE, &user);
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(PostureTest, PostureTest_018, TestSize.Level1)
{
    SEN_HILOGI("PostureTest_018 enter");
    if (g_existPosture) {
        SensorUser user;
        user.callback = PostureDataCallbackImpl;
        int32_t ret = SubscribeSensor(SENSOR_TYPE_ID_POSTURE, &user);
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
        ret = SetBatch(SENSOR_TYPE_ID_POSTURE, &user, 10000000, 10000000);
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
        ret = ActivateSensor(SENSOR_TYPE_ID_POSTURE, &user);
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
        ret = DeactivateSensor(SENSOR_TYPE_ID_POSTURE, &user);
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
        ret = UnsubscribeSensor(SENSOR_TYPE_ID_POSTURE, &user);
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);

        SensorUser user2;
        user2.callback = PostureDataCallbackImpl2;
        ret = SubscribeSensor(SENSOR_TYPE_ID_POSTURE, &user2);
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
        ret = SetBatch(SENSOR_TYPE_ID_POSTURE, &user2, 20000000, 20000000);
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
        ret = ActivateSensor(SENSOR_TYPE_ID_POSTURE, &user2);
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
        ret = DeactivateSensor(SENSOR_TYPE_ID_POSTURE, &user2);
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
        ret = UnsubscribeSensor(SENSOR_TYPE_ID_POSTURE, &user2);
        ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    }
}
} // namespace Sensors
} // namespace OHOS
