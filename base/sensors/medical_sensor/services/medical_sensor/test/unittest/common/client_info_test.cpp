/*
 * Copyright (c) 2022 Chipsea Technologies (Shenzhen) Corp., Ltd.
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

#include "client_info.h"
#include "medical_errors.h"
#include "medical_log_domain.h"

namespace OHOS {
namespace Sensors {
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_TEST, "ClientInfoTest" };
const uint32_t INVALID_SENSOR_ID = -1;
const uint32_t ACC_SENSOR_ID = (1 << 16) | (1 << 8);
const uint32_t MAG_SENSOR_ID = (1 << 24) | (1 << 16) | (1 << 8);
}  // namespace

class ClientInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    ClientInfo &clientInfo_ = ClientInfo::GetInstance();
};

void ClientInfoTest::SetUpTestCase()
{}

void ClientInfoTest::TearDownTestCase()
{}

void ClientInfoTest::SetUp()
{}

void ClientInfoTest::TearDown()
{}

/*
 * @tc.name: UpdateSensorInfo_001
 * @tc.desc: update sensor info
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, UpdateSensorInfo_001, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    int32_t pid = 1000;
    int64_t samplingPeriodNs = 100000000;
    int64_t maxReportDelayNs = 0;
    MedicalSensorBasicInfo sensorInfo;
    sensorInfo.SetSamplingPeriodNs(samplingPeriodNs);
    sensorInfo.SetMaxReportDelayNs(maxReportDelayNs);
    sensorInfo.SetSensorState(SENSOR_ENABLED);
    bool ret = clientInfo_.UpdateSensorInfo(ACC_SENSOR_ID, pid, sensorInfo);
    ASSERT_TRUE(ret);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

/*
 * @tc.name: UpdateSensorInfo_002
 * @tc.desc: update sensor info
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, UpdateSensorInfo_002, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    int32_t pid = 1067;
    int64_t samplingPeriodNs = 100000000L;
    int64_t maxReportDelayNs = 0;
    MedicalSensorBasicInfo sensorInfo;
    sensorInfo.SetSamplingPeriodNs(samplingPeriodNs);
    sensorInfo.SetMaxReportDelayNs(maxReportDelayNs);
    sensorInfo.SetSensorState(SENSOR_ENABLED);
    bool ret = clientInfo_.UpdateSensorInfo(ACC_SENSOR_ID, pid, sensorInfo);
    ASSERT_TRUE(ret);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

/*
 * @tc.name: UpdateSensorInfo_003
 * @tc.desc: update sensor info
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, UpdateSensorInfo_003, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    int32_t pid = 1000;
    int64_t samplingPeriodNs = 100000000;
    int64_t maxReportDelayNs = 0;
    MedicalSensorBasicInfo sensorInfo;
    sensorInfo.SetSamplingPeriodNs(samplingPeriodNs);
    sensorInfo.SetMaxReportDelayNs(maxReportDelayNs);
    sensorInfo.SetSensorState(SENSOR_ENABLED);
    bool ret = clientInfo_.UpdateSensorInfo(MAG_SENSOR_ID, pid, sensorInfo);
    ASSERT_TRUE(ret);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

/*
 * @tc.name: UpdateSensorInfo_004
 * @tc.desc: update sensor info
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, UpdateSensorInfo_004, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    int32_t pid = 1000;
    int64_t samplingPeriodNs = 100000000;
    int64_t maxReportDelayNs = 0;
    MedicalSensorBasicInfo sensorInfo;
    sensorInfo.SetSamplingPeriodNs(samplingPeriodNs);
    sensorInfo.SetMaxReportDelayNs(maxReportDelayNs);
    sensorInfo.SetSensorState(SENSOR_ENABLED);
    bool ret = clientInfo_.UpdateSensorInfo(INVALID_SENSOR_ID, pid, sensorInfo);
    ASSERT_FALSE(ret);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

/*
 * @tc.name: UpdateSensorInfo_005
 * @tc.desc: update sensor info
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, UpdateSensorInfo_005, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    int32_t pid = -1;
    int64_t samplingPeriodNs = 100000000;
    int64_t maxReportDelayNs = 0;
    MedicalSensorBasicInfo sensorInfo;
    sensorInfo.SetSamplingPeriodNs(samplingPeriodNs);
    sensorInfo.SetMaxReportDelayNs(maxReportDelayNs);
    sensorInfo.SetSensorState(SENSOR_DISABLED);
    bool ret = clientInfo_.UpdateSensorInfo(MAG_SENSOR_ID, pid, sensorInfo);
    ASSERT_FALSE(ret);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

/*
 * @tc.name: UpdateSensorInfo_006
 * @tc.desc: update sensor info
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, UpdateSensorInfo_006, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    int32_t pid = 1067;
    int64_t samplingPeriodNs = 50000000;
    int64_t maxReportDelayNs = 0;
    MedicalSensorBasicInfo sensorInfo;
    sensorInfo.SetSamplingPeriodNs(samplingPeriodNs);
    sensorInfo.SetMaxReportDelayNs(maxReportDelayNs);
    sensorInfo.SetSensorState(SENSOR_ENABLED);
    bool ret = clientInfo_.UpdateSensorInfo(ACC_SENSOR_ID, pid, sensorInfo);
    ASSERT_TRUE(ret);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

/*
 * @tc.name: GetSensorState_001
 * @tc.desc: get sensor state
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, GetSensorState_001, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    MedicalSensorState ret = clientInfo_.GetSensorState(ACC_SENSOR_ID);
    ASSERT_EQ(ret, SENSOR_ENABLED);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

/*
 * @tc.name: GetSensorState_002
 * @tc.desc: get sensor state
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, GetSensorState_002, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    MedicalSensorState ret = clientInfo_.GetSensorState(MAG_SENSOR_ID);
    ASSERT_EQ(ret, SENSOR_ENABLED);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

/*
 * @tc.name: UpdateSensorChannel_001
 * @tc.desc: update sensor channel
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, UpdateSensorChannel_001, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    int32_t pid = 1067;
    sptr<MedicalSensorBasicDataChannel> sensorChannel = new (std::nothrow) MedicalSensorBasicDataChannel();
    ASSERT_NE(sensorChannel, nullptr);
    auto ret = clientInfo_.UpdateSensorChannel(pid, sensorChannel);
    ASSERT_TRUE(ret);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

/*
 * @tc.name: UpdateSensorChannel_002
 * @tc.desc: update sensor channel
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, UpdateSensorChannel_002, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    int32_t pid = -1;
    sptr<MedicalSensorBasicDataChannel> sensorChannel = new (std::nothrow) MedicalSensorBasicDataChannel();
    ASSERT_NE(sensorChannel, nullptr);
    auto ret = clientInfo_.UpdateSensorChannel(pid, sensorChannel);
    ASSERT_FALSE(ret);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

/*
 * @tc.name: UpdateSensorChannel_003
 * @tc.desc: update sensor channel
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, UpdateSensorChannel_003, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    int32_t pid = 1000;
    sptr<MedicalSensorBasicDataChannel> sensorChannel = nullptr;
    auto ret = clientInfo_.UpdateSensorChannel(pid, sensorChannel);
    ASSERT_FALSE(ret);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

/*
 * @tc.name: UpdateSensorChannel_004
 * @tc.desc: update sensor channel
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, UpdateSensorChannel_004, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    int32_t pid = -1;
    sptr<MedicalSensorBasicDataChannel> sensorChannel = nullptr;
    auto ret = clientInfo_.UpdateSensorChannel(pid, sensorChannel);
    ASSERT_FALSE(ret);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

/*
 * @tc.name: UpdateSensorChannel_005
 * @tc.desc: update sensor channel
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, UpdateSensorChannel_005, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    int32_t pid = 1000;
    sptr<MedicalSensorBasicDataChannel> sensorChannel = new (std::nothrow) MedicalSensorBasicDataChannel();
    ASSERT_NE(sensorChannel, nullptr);
    auto ret = clientInfo_.UpdateSensorChannel(pid, sensorChannel);
    ASSERT_TRUE(ret);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

/*
 * @tc.name: GetSensorChannel_001
 * @tc.desc: get sensor channel
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, GetSensorChannel_001, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    auto ret = clientInfo_.GetSensorChannel(ACC_SENSOR_ID);
    ASSERT_EQ(ret.size(), 2UL);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

/*
 * @tc.name: GetSensorChannel_002
 * @tc.desc: get sensor channel
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, GetSensorChannel_002, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    auto ret = clientInfo_.GetSensorChannel(INVALID_SENSOR_ID);
    ASSERT_EQ(ret.size(), 0UL);
    HiLog::Info(LABEL, "%{public}s end ret.size() : %{public}d", __func__, int32_t { ret.size() });
}

/*
 * @tc.name: GetSensorChannel_003
 * @tc.desc: get sensor channel
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, GetSensorChannel_003, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    auto ret = clientInfo_.GetSensorChannel(INVALID_SENSOR_ID);
    ASSERT_EQ(ret.size(), 0UL);
    HiLog::Info(LABEL, "%{public}s end ret.size() : %{public}d", __func__, int32_t { ret.size() });
}

/*
 * @tc.name: GetSensorChannel_001
 * @tc.desc: get sensor channel
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, GetSensorChannel_004, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    auto ret = clientInfo_.GetSensorChannel(MAG_SENSOR_ID);
    ASSERT_EQ(ret.size(), 1UL);
    HiLog::Info(LABEL, "%{public}s end", __func__);
}

/*
 * @tc.name: DestroySensorChannel_001
 * @tc.desc: destroy sensor channel
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, DestroySensorChannel_001, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);
    int32_t pid = 1067;

    MedicalSensorState ret = clientInfo_.GetSensorState(ACC_SENSOR_ID);
    ASSERT_EQ(ret, SENSOR_ENABLED);

    auto channelList = clientInfo_.GetSensorChannel(ACC_SENSOR_ID);
    ASSERT_EQ(channelList.size(), 2UL);

    auto flag = clientInfo_.DestroySensorChannel(pid);
    ASSERT_TRUE(flag);

    ret = clientInfo_.GetSensorState(ACC_SENSOR_ID);
    ASSERT_EQ(ret, SENSOR_ENABLED);

    channelList = clientInfo_.GetSensorChannel(ACC_SENSOR_ID);
    ASSERT_EQ(channelList.size(), 1UL);

    HiLog::Info(LABEL, "%{public}s end ret.size() : %{public}d", __func__, int32_t { channelList.size() });
}

/*
 * @tc.name: ClearSensorInfo_001
 * @tc.desc: clear sensor info
 * @tc.type: FUNC
 */
HWTEST_F(ClientInfoTest, ClearSensorInfo_001, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);

    MedicalSensorState ret = clientInfo_.GetSensorState(ACC_SENSOR_ID);
    ASSERT_EQ(ret, SENSOR_ENABLED);

    auto flag = clientInfo_.ClearSensorInfo(ACC_SENSOR_ID);
    ASSERT_TRUE(flag);

    ret = clientInfo_.GetSensorState(ACC_SENSOR_ID);
    ASSERT_NE(ret, SENSOR_ENABLED);

    HiLog::Info(LABEL, "%{public}s end", __func__);
}
}  // namespace Sensors
}  // namespace OHOS
