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
#include <thread>

#include "medical_native_impl.h"
#include "medical_errors.h"
#include "medical_log_domain.h"

namespace OHOS {
namespace Sensors {
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_TEST, "MedicalNativeTest" };
}  // namespace

class MedicalNativeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void MedicalNativeTest::SetUpTestCase()
{}

void MedicalNativeTest::TearDownTestCase()
{}

void MedicalNativeTest::SetUp()
{}

void MedicalNativeTest::TearDown()
{}

void AfeDataCallbackImpl(SensorEvent *event)
{
    if (event == nullptr) {
        HiLog::Error(LABEL, "AfeDataCallbackImpl event is null");
        return;
    }
    uint32_t *sensorData = (uint32_t *)(event[0].data);
    HiLog::Info(LABEL, "AfeDataCallbackImpl sensorTypeId: %{public}d, dataLen: %{public}d, data[0]: %{public}d\n",
        event[0].sensorTypeId, event[0].dataLen, *(sensorData));
}

/*
 * @tc.name: AfeNativeApiTest_001
 * @tc.desc: afe native api test
 * @tc.type: FUNC
 * @tc.author: wuzhihui
 */
HWTEST_F(MedicalNativeTest, AfeNativeApiTest_001, TestSize.Level1)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);

    int32_t sensorTypeId = 0;
    MedicalSensorUser user;

    user.callback = AfeDataCallbackImpl;

    int32_t ret = SubscribeSensor(sensorTypeId, &user);
    ASSERT_EQ(ret, 0);

    ret = SetBatch(sensorTypeId, &user, 100000000, 100000000);
    ASSERT_EQ(ret, 0);

    ret = ActivateSensor(sensorTypeId, &user);
    ASSERT_EQ(ret, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    ASSERT_EQ(ret, 0);

    ret = DeactivateSensor(sensorTypeId, &user);
    ASSERT_EQ(ret, 0);

    ret = UnsubscribeSensor(sensorTypeId, &user);
    ASSERT_EQ(ret, 0);
}
}  // namespace Sensors
}  // namespace OHOS
