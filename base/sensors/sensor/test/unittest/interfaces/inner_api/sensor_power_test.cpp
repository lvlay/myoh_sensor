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

#include <cinttypes>
#include <thread>

#include <sys/types.h>
#include <unistd.h>

#include "accesstoken_kit.h"
#include <gtest/gtest.h>
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "sensor_agent.h"
#include "sensor_errors.h"

#undef LOG_TAG
#define LOG_TAG "SensorPowerTest"

namespace OHOS {
namespace Sensors {
using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;

namespace {
constexpr int32_t SENSOR_ID { 1 };
constexpr int32_t INVALID_VALUE { -1 };
} // namespace

class SensorPowerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

int32_t g_processPid = 0;

void SensorPowerTest::SetUpTestCase()
{
    const char **perms = new (std::nothrow) const char *[2];
    CHKPV(perms);
    perms[0] = "ohos.permission.ACCELEROMETER";
    perms[1] = "ohos.permission.MANAGE_SENSOR";
    TokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "SensorPowerTest",
        .aplStr = "system_core",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    ASSERT_EQ(SetSelfTokenID(tokenId), 0);
    AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;

    g_processPid = getpid();
    SEN_HILOGI("Current process pid is %{public}d", g_processPid);
    ASSERT_NE(g_processPid, 0);
}

void SensorPowerTest::TearDownTestCase() {}

void SensorPowerTest::SetUp() {}

void SensorPowerTest::TearDown() {}

void SensorDataCallbackImpl(SensorEvent *event)
{
    if (event == nullptr) {
        SEN_HILOGE("event is null");
        return;
    }
    if (event[0].data == nullptr) {
        SEN_HILOGE("event[0].data is nullptr");
        return;
    }
    if (event[0].dataLen < sizeof(AccelData)) {
        SEN_HILOGE("Event dataLen less than acc data size, event.dataLen:%{public}u", event[0].dataLen);
        return;
    }
    AccelData *accelData = reinterpret_cast<AccelData *>(event[0].data);
    SEN_HILOGI("sensorId:%{public}d, version:%{public}d, dataLen:%{public}d, x:%{public}f, y:%{public}f, z:%{public}f",
        event[0].sensorTypeId, event[0].version, event[0].dataLen, accelData->x, accelData->y, accelData->z);
}

void SensorActiveInfoCBImpl(SensorActiveInfo &sensorActiveInfo)
{
    SEN_HILOGI("pid:%{public}d, sensorId:%{public}d, samplingPeriodNs:%{public}" PRId64 ", "
        "maxReportDelayNs:%{public}" PRId64 "", sensorActiveInfo.pid, sensorActiveInfo.sensorId,
        sensorActiveInfo.samplingPeriodNs, sensorActiveInfo.maxReportDelayNs);
}

void SensorActiveInfoCBImpl2(SensorActiveInfo &sensorActiveInfo)
{
    SEN_HILOGI("pid:%{public}d, sensorId:%{public}d, samplingPeriodNs:%{public}" PRId64 ", "
        "maxReportDelayNs:%{public}" PRId64 "", sensorActiveInfo.pid, sensorActiveInfo.sensorId,
        sensorActiveInfo.samplingPeriodNs, sensorActiveInfo.maxReportDelayNs);
}

HWTEST_F(SensorPowerTest, SensorPowerTest_001, TestSize.Level1)
{
    SEN_HILOGI("SensorPowerTest_001 in");
    int32_t ret = SuspendSensors(INVALID_VALUE);
    ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
}

HWTEST_F(SensorPowerTest, SensorPowerTest_002, TestSize.Level1)
{
    SEN_HILOGI("SensorPowerTest_002 in");
    int32_t ret = ResumeSensors(INVALID_VALUE);
    ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
}

HWTEST_F(SensorPowerTest, SensorPowerTest_003, TestSize.Level1)
{
    SEN_HILOGI("SensorPowerTest_003 in");
    SensorActiveInfo *sensorActiveInfos {nullptr};
    int32_t count { 0 };
    int32_t ret = GetActiveSensorInfos(INVALID_VALUE, &sensorActiveInfos, &count);
    ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
}

HWTEST_F(SensorPowerTest, SensorPowerTest_004, TestSize.Level1)
{
    SEN_HILOGI("SensorPowerTest_004 in");
    SensorActiveInfoCB callback = nullptr;
    int32_t ret = Register(callback);
    ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
}

HWTEST_F(SensorPowerTest, SensorPowerTest_005, TestSize.Level1)
{
    SEN_HILOGI("SensorPowerTest_005 in");
    SensorActiveInfoCB callback = nullptr;
    int32_t ret = Unregister(callback);
    ASSERT_NE(ret, OHOS::Sensors::SUCCESS);
}

HWTEST_F(SensorPowerTest, SensorPowerTest_006, TestSize.Level1)
{
    SEN_HILOGI("SensorPowerTest_006 in");
    SensorUser user;
    user.callback = SensorDataCallbackImpl;

    int32_t ret = SubscribeSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = SetBatch(SENSOR_ID, &user, 100000000, 0);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = ActivateSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ret = SuspendSensors(g_processPid);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ret = ResumeSensors(g_processPid);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ret = DeactivateSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = UnsubscribeSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
}

HWTEST_F(SensorPowerTest, SensorPowerTest_007, TestSize.Level1)
{
    SEN_HILOGI("SensorPowerTest_007 in");
    SensorUser user;
    user.callback = SensorDataCallbackImpl;

    int32_t ret = SubscribeSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = SetBatch(SENSOR_ID, &user, 100000000, 0);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = ActivateSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    SensorActiveInfo *sensorActiveInfos {nullptr};
    int32_t count { 0 };
    ret = GetActiveSensorInfos(g_processPid, &sensorActiveInfos, &count);
    for (int32_t i = 0; i < count; ++i) {
        SensorActiveInfo *curSensorActiveInfo = sensorActiveInfos + i;
        SEN_HILOGI("pid:%{public}d, sensorId:%{public}d, samplingPeriodNs:%{public}" PRId64 ", "
            "maxReportDelayNs:%{public}" PRId64 "", curSensorActiveInfo->pid, curSensorActiveInfo->sensorId,
            curSensorActiveInfo->samplingPeriodNs, curSensorActiveInfo->maxReportDelayNs);
    }
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);

    ret = SuspendSensors(g_processPid);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ret = ResumeSensors(g_processPid);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ret = DeactivateSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = UnsubscribeSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
}

HWTEST_F(SensorPowerTest, SensorPowerTest_008, TestSize.Level1)
{
    SEN_HILOGI("SensorPowerTest_008 in");
    SensorUser user;
    user.callback = SensorDataCallbackImpl;
    SensorActiveInfoCB callback = SensorActiveInfoCBImpl;

    int32_t ret = Register(callback);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);

    ret = SubscribeSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = SetBatch(SENSOR_ID, &user, 100000000, 0);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = ActivateSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ret = SuspendSensors(g_processPid);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ret = ResumeSensors(g_processPid);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ret = DeactivateSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = UnsubscribeSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);

    ret = Unregister(callback);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
}

HWTEST_F(SensorPowerTest, SensorPowerTest_009, TestSize.Level1)
{
    SEN_HILOGI("SensorPowerTest_009 in");
    SensorUser user;
    user.callback = SensorDataCallbackImpl;
    SensorActiveInfoCB callback = SensorActiveInfoCBImpl;
    SensorActiveInfoCB callback2 = SensorActiveInfoCBImpl2;

    int32_t ret = Register(callback);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = Register(callback2);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);

    ret = SubscribeSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = SetBatch(SENSOR_ID, &user, 100000000, 0);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = ActivateSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ret = SuspendSensors(g_processPid);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ret = ResumeSensors(g_processPid);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ret = DeactivateSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = UnsubscribeSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);

    ret = Unregister(callback);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = Unregister(callback2);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
}

HWTEST_F(SensorPowerTest, SensorPowerTest_010, TestSize.Level1)
{
    SEN_HILOGI("SensorPowerTest_010 in");
    SensorUser user;
    user.callback = SensorDataCallbackImpl;

    int32_t ret = SubscribeSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = SetBatch(SENSOR_ID, &user, 100000000, 0);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = ActivateSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ret = SuspendSensors(g_processPid);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ret = ResetSensors();
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ret = DeactivateSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ret = UnsubscribeSensor(SENSOR_ID, &user);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
}
} // namespace Sensors
} // namespace OHOS