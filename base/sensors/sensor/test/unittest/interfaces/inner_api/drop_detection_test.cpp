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
#define LOG_TAG "DropDetectionTest"

namespace OHOS {
namespace Sensors {
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
std::atomic_bool g_hasDropDetection = false;
} // namespace

class DropDetectionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DropDetectionTest::SetUpTestCase()
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
        if (sensorInfo[i].sensorId == SENSOR_TYPE_ID_DROP_DETECTION) {
            g_hasDropDetection = true;
            SEN_HILOGD("Exist drop detection sensor");
            break;
        }
    }
    SEN_HILOGD("Not exist drop detection sensor");
}

void DropDetectionTest::TearDownTestCase() {}

void DropDetectionTest::SetUp() {}

void DropDetectionTest::TearDown() {}

void DropDetectionDataCallbackImpl(SensorEvent *event)
{
    if (event == nullptr) {
        SEN_HILOGE("event is nullptr");
        return;
    }
    if (event[0].data == nullptr) {
        SEN_HILOGE("event[0].data is nullptr");
        return;
    }
    if (event[0].dataLen < sizeof(DropDetectionData)) {
        SEN_HILOGE("Event dataLen less than drop detection data size, event.dataLen:%{public}u", event[0].dataLen);
        return;
    }
    DropDetectionData *dropDetectionData = reinterpret_cast<DropDetectionData *>(event[0].data);
    SEN_HILOGD("sensorId:%{public}d, version:%{public}d, dataLen:%{public}u, status:%{public}f", event[0].sensorTypeId,
        event[0].version, event[0].dataLen, dropDetectionData->status);
}

void DropDetectionDataCallbackImpl2(SensorEvent *event)
{
    if (event == nullptr) {
        SEN_HILOGE("event is nullptr");
        return;
    }
    if (event[0].data == nullptr) {
        SEN_HILOGE("event[0].data is nullptr");
        return;
    }
    if (event[0].dataLen < sizeof(DropDetectionData)) {
        SEN_HILOGE("Event dataLen less than drop detrction data size, event.dataLen:%{public}u", event[0].dataLen);
        return;
    }
    DropDetectionData *dropDetectionData = reinterpret_cast<DropDetectionData *>(event[0].data);
    SEN_HILOGD("sensorId:%{public}d, version:%{public}d, dataLen:%{public}u, status:%{public}f", event[0].sensorTypeId,
        event[0].version, event[0].dataLen, dropDetectionData->status);
}

HWTEST_F(DropDetectionTest, DropDetectionTest_001, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_001 enter");
    if (g_hasDropDetection) {
        ASSERT_NE(ActivateSensor(SENSOR_TYPE_ID_DROP_DETECTION, nullptr), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_002, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_002 enter");
    if (g_hasDropDetection) {
        SensorUser user;
        user.callback = DropDetectionDataCallbackImpl;
        ASSERT_NE(ActivateSensor(-1, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_003, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_003 enter");
    if (g_hasDropDetection) {
        SensorUser user;
        user.callback = nullptr;
        ASSERT_NE(ActivateSensor(SENSOR_TYPE_ID_DROP_DETECTION, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_004, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_004 enter");
    if (g_hasDropDetection) {
        ASSERT_NE(DeactivateSensor(SENSOR_TYPE_ID_DROP_DETECTION, nullptr), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_005, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_005 enter");
    if (g_hasDropDetection) {
        SensorUser user;
        user.callback = DropDetectionDataCallbackImpl;
        ASSERT_NE(DeactivateSensor(-1, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_006, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_006 enter");
    if (g_hasDropDetection) {
        SensorUser user;
        user.callback = nullptr;
        ASSERT_NE(DeactivateSensor(SENSOR_TYPE_ID_DROP_DETECTION, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_007, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_007 enter");
    if (g_hasDropDetection) {
        ASSERT_NE(SetBatch(SENSOR_TYPE_ID_DROP_DETECTION, nullptr, 10000000, 10000000), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_008, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_008 enter");
    if (g_hasDropDetection) {
        SensorUser user;
        user.callback = DropDetectionDataCallbackImpl;
        ASSERT_NE(SetBatch(-1, &user, 10000000, 10000000), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_009, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_009 enter");
    if (g_hasDropDetection) {
        SensorUser user;
        user.callback = nullptr;
        ASSERT_NE(SetBatch(SENSOR_TYPE_ID_DROP_DETECTION, &user, 10000000, 10000000), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_010, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_010 enter");
    if (g_hasDropDetection) {
        SensorUser user;
        user.callback = DropDetectionDataCallbackImpl;
        ASSERT_NE(SetBatch(SENSOR_TYPE_ID_DROP_DETECTION, &user, -1, -1), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_011, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_011 enter");
    if (g_hasDropDetection) {
        ASSERT_NE(SubscribeSensor(SENSOR_TYPE_ID_DROP_DETECTION, nullptr), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_012, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_012 enter");
    if (g_hasDropDetection) {
        SensorUser user;
        user.callback = DropDetectionDataCallbackImpl;
        ASSERT_NE(SubscribeSensor(-1, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_013, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_013 enter");
    if (g_hasDropDetection) {
        SensorUser user;
        user.callback = nullptr;
        ASSERT_NE(SubscribeSensor(SENSOR_TYPE_ID_DROP_DETECTION, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_014, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_014 enter");
    if (g_hasDropDetection) {
        ASSERT_NE(UnsubscribeSensor(SENSOR_TYPE_ID_DROP_DETECTION, nullptr), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_015, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_015 enter");
    if (g_hasDropDetection) {
        SensorUser user;
        user.callback = DropDetectionDataCallbackImpl;
        ASSERT_NE(UnsubscribeSensor(-1, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_016, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_016 enter");
    if (g_hasDropDetection) {
        SensorUser user;
        user.callback = nullptr;
        ASSERT_NE(UnsubscribeSensor(SENSOR_TYPE_ID_DROP_DETECTION, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_017, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_017 enter");
    if (g_hasDropDetection) {
        SensorUser user;
        user.callback = DropDetectionDataCallbackImpl;
        ASSERT_EQ(SubscribeSensor(SENSOR_TYPE_ID_DROP_DETECTION, &user), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(SetBatch(SENSOR_TYPE_ID_DROP_DETECTION, &user, 10000000, 10000000), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(ActivateSensor(SENSOR_TYPE_ID_DROP_DETECTION, &user), OHOS::Sensors::SUCCESS);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(DeactivateSensor(SENSOR_TYPE_ID_DROP_DETECTION, &user), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(UnsubscribeSensor(SENSOR_TYPE_ID_DROP_DETECTION, &user), OHOS::Sensors::SUCCESS);
    }
}

HWTEST_F(DropDetectionTest, DropDetectionTest_018, TestSize.Level1)
{
    SEN_HILOGI("DropDetectionTest_018 enter");
    if (g_hasDropDetection) {
        SensorUser user;
        user.callback = DropDetectionDataCallbackImpl;
        ASSERT_EQ(SubscribeSensor(SENSOR_TYPE_ID_DROP_DETECTION, &user), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(SetBatch(SENSOR_TYPE_ID_DROP_DETECTION, &user, 10000000, 10000000), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(ActivateSensor(SENSOR_TYPE_ID_DROP_DETECTION, &user), OHOS::Sensors::SUCCESS);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(DeactivateSensor(SENSOR_TYPE_ID_DROP_DETECTION, &user), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(UnsubscribeSensor(SENSOR_TYPE_ID_DROP_DETECTION, &user), OHOS::Sensors::SUCCESS);

        SensorUser user2;
        user2.callback = DropDetectionDataCallbackImpl2;
        ASSERT_EQ(SubscribeSensor(SENSOR_TYPE_ID_DROP_DETECTION, &user2), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(SetBatch(SENSOR_TYPE_ID_DROP_DETECTION, &user2, 20000000, 20000000), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(ActivateSensor(SENSOR_TYPE_ID_DROP_DETECTION, &user2), OHOS::Sensors::SUCCESS);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        ASSERT_EQ(DeactivateSensor(SENSOR_TYPE_ID_DROP_DETECTION, &user2), OHOS::Sensors::SUCCESS);
        ASSERT_EQ(UnsubscribeSensor(SENSOR_TYPE_ID_DROP_DETECTION, &user2), OHOS::Sensors::SUCCESS);
    }
}
} // namespace Sensors
} // namespace OHOS
