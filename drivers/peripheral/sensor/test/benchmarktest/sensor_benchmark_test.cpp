/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <benchmark/benchmark.h>
#include <cmath>
#include <cstdio>
#include <gtest/gtest.h>
#include <securec.h>
#include <string>
#include <unistd.h>
#include <vector>
#include "hdf_base.h"
#include "osal_time.h"
#include "sensor_callback_impl.h"
#include "sensor_type.h"
#include "sensor_uhdf_log.h"
#include "v2_0/isensor_interface.h"

using namespace OHOS::HDI::Sensor::V2_0;
using namespace testing::ext;
using namespace std;

namespace {
    sptr<ISensorInterface>  g_sensorInterface = nullptr;
    sptr<ISensorCallback> g_traditionalCallback = new SensorCallbackImpl();
    sptr<ISensorCallback> g_medicalCallback = new SensorCallbackImpl();
    std::vector<HdfSensorInformation> g_info;

    constexpr int32_t ITERATION_FREQUENCY = 100;
    constexpr int32_t REPETITION_FREQUENCY = 3;
    constexpr int32_t SENSOR_INTERVAL1 = 20;
    constexpr int32_t SENSOR_INTERVAL2 = 2;
    constexpr int32_t SENSOR_POLL_TIME = 3;
    constexpr int32_t SENSOR_WAIT_TIME = 10;
    constexpr uint32_t OPTION = 0;
    constexpr uint32_t SENSOR_DATA_FLAG = 1;

class SensorBenchmarkTest : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State &state);
    void TearDown(const ::benchmark::State &state);
};

void SensorBenchmarkTest::SetUp(const ::benchmark::State &state)
{
    g_sensorInterface = ISensorInterface::Get();
}

void SensorBenchmarkTest::TearDown(const ::benchmark::State &state)
{
}

/**
  * @tc.name: DriverSystem_SensorBenchmark_GetAllSensorInfo
  * @tc.desc: Benchmarktest for interface GetAllSensorInfo
  * Obtains information about all sensors in the system
  * @tc.type: FUNC
  */
BENCHMARK_F(SensorBenchmarkTest, GetAllSensorInfo)(benchmark::State &state)
{
    ASSERT_NE(nullptr, g_sensorInterface);

    int32_t ret;

    for (auto _ : state) {
        ret = g_sensorInterface->GetAllSensorInfo(g_info);
        EXPECT_EQ(SENSOR_SUCCESS, ret);
    }
}

BENCHMARK_REGISTER_F(SensorBenchmarkTest, GetAllSensorInfo)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
  * @tc.name: DriverSystem_SensorBenchmark_register
  * @tc.desc: Benchmarktest for interface register
  * Returns 0 if the callback is successfully registered; returns a negative value otherwise
  * @tc.type: FUNC
  */
BENCHMARK_F(SensorBenchmarkTest, register)(benchmark::State &state)
{
    ASSERT_NE(nullptr, g_sensorInterface);

    int32_t ret;

    for (auto _ : state) {
        ret = g_sensorInterface->Register(TRADITIONAL_SENSOR_TYPE, g_traditionalCallback);
        EXPECT_EQ(SENSOR_SUCCESS, ret);
    }
    ret = g_sensorInterface->Unregister(TRADITIONAL_SENSOR_TYPE, g_traditionalCallback);
    EXPECT_EQ(SENSOR_SUCCESS, ret);
}

BENCHMARK_REGISTER_F(SensorBenchmarkTest, register)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
  * @tc.name: DriverSystem_SensorBenchmark_Unregister
  * @tc.desc: Benchmarktest for interface Unregister
  * Returns 0 if the callback is successfully registered; returns a negative value otherwise
  * @tc.type: FUNC
  */
BENCHMARK_F(SensorBenchmarkTest, Unregister)(benchmark::State &state)
{
    ASSERT_NE(nullptr, g_sensorInterface);

    int32_t ret;

    for (auto _ : state) {
        ret = g_sensorInterface->Register(TRADITIONAL_SENSOR_TYPE, g_traditionalCallback);
        OsalMSleep(SENSOR_POLL_TIME);
        EXPECT_EQ(SENSOR_SUCCESS, ret);
        ret = g_sensorInterface->Unregister(TRADITIONAL_SENSOR_TYPE, g_traditionalCallback);
        OsalMSleep(SENSOR_POLL_TIME);
        EXPECT_EQ(SENSOR_SUCCESS, ret);
    }
}

BENCHMARK_REGISTER_F(SensorBenchmarkTest, Unregister)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
  * @tc.name: DriverSystem_SensorBenchmark_Enable
  * @tc.desc: Benchmarktest for interface Enable
  * Enables the sensor unavailable in the sensor list based on the specified sensor ID
  * @tc.type: FUNC
  */
BENCHMARK_F(SensorBenchmarkTest, Enable)(benchmark::State &state)
{
    ASSERT_NE(nullptr, g_sensorInterface);

    int32_t ret;

    ret = g_sensorInterface->Register(TRADITIONAL_SENSOR_TYPE, g_traditionalCallback);
    EXPECT_EQ(SENSOR_SUCCESS, ret);

    EXPECT_GT(g_info.size(), 0);

    for (auto iter : g_info) {
        HDF_LOGI("get sensoriId[%{public}d], info name[%{public}s], power[%{public}f]\n\r",
            iter.sensorId, iter.sensorName.c_str(), iter.power);
        ret = g_sensorInterface->SetBatch(iter.sensorId, SENSOR_INTERVAL1, SENSOR_POLL_TIME);
        EXPECT_EQ(SENSOR_SUCCESS, ret);
        for (auto _ : state) {
            ret = g_sensorInterface->Enable(iter.sensorId);
            EXPECT_EQ(SENSOR_SUCCESS, ret);
        }
        OsalMSleep(SENSOR_POLL_TIME);
        ret = g_sensorInterface->Disable(iter.sensorId);
        EXPECT_EQ(SENSOR_SUCCESS, ret);
    }
    ret = g_sensorInterface->Unregister(TRADITIONAL_SENSOR_TYPE, g_traditionalCallback);
    EXPECT_EQ(SensorCallbackImpl::sensorDataFlag, SENSOR_DATA_FLAG);
    SensorCallbackImpl::sensorDataFlag = SENSOR_DATA_FLAG;
}

BENCHMARK_REGISTER_F(SensorBenchmarkTest, Enable)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
  * @tc.name: DriverSystem_SensorBenchmark_Disable
  * @tc.desc: Benchmarktest for interface Disable
  * Enables the sensor unavailable in the sensor list based on the specified sensor ID
  * @tc.type: FUNC
  */
BENCHMARK_F(SensorBenchmarkTest, Disable)(benchmark::State &state)
{
    ASSERT_NE(nullptr, g_sensorInterface);

    int32_t ret;

    ret = g_sensorInterface->Register(TRADITIONAL_SENSOR_TYPE, g_traditionalCallback);
    EXPECT_EQ(SENSOR_SUCCESS, ret);
    EXPECT_GT(g_info.size(), 0);

    for (auto iter : g_info) {
        HDF_LOGI("get sensoriId[%{public}d], info name[%{public}s], power[%{public}f]\n\r",
            iter.sensorId, iter.sensorName.c_str(), iter.power);
        ret = g_sensorInterface->SetBatch(iter.sensorId, SENSOR_INTERVAL1, SENSOR_POLL_TIME);
        EXPECT_EQ(SENSOR_SUCCESS, ret);
        ret = g_sensorInterface->Enable(iter.sensorId);
        EXPECT_EQ(SENSOR_SUCCESS, ret);
        OsalMSleep(SENSOR_POLL_TIME);
        for (auto _ : state) {
            ret = g_sensorInterface->Disable(iter.sensorId);
            EXPECT_EQ(SENSOR_SUCCESS, ret);
        }
    }
    ret = g_sensorInterface->Unregister(TRADITIONAL_SENSOR_TYPE, g_traditionalCallback);
    EXPECT_EQ(SensorCallbackImpl::sensorDataFlag, SENSOR_DATA_FLAG);
    SensorCallbackImpl::sensorDataFlag = SENSOR_DATA_FLAG;
}

BENCHMARK_REGISTER_F(SensorBenchmarkTest, Disable)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
  * @tc.name: DriverSystem_SensorBenchmark_SetBatch
  * @tc.desc: Benchmarktest for interface SetBatch
  * Sets the sampling time and data report interval for sensors in batches
  * @tc.type: FUNC
  */
BENCHMARK_F(SensorBenchmarkTest, SetBatch)(benchmark::State &state)
{
    ASSERT_NE(nullptr, g_sensorInterface);

    int32_t ret;

    ret = g_sensorInterface->Register(TRADITIONAL_SENSOR_TYPE, g_traditionalCallback);
    EXPECT_EQ(SENSOR_SUCCESS, ret);

    EXPECT_GT(g_info.size(), 0);
    for (auto iter : g_info) {
        HDF_LOGI("get sensoriId[%{public}d], info name[%{public}s], power[%{public}f]\n\r",
            iter.sensorId, iter.sensorName.c_str(), iter.power);
        for (auto _ : state) {
            ret = g_sensorInterface->SetBatch(iter.sensorId, SENSOR_INTERVAL2, SENSOR_POLL_TIME);
            EXPECT_EQ(SENSOR_SUCCESS, ret);
        }
        ret = g_sensorInterface->Enable(iter.sensorId);
        EXPECT_EQ(SENSOR_SUCCESS, ret);
        OsalMSleep(SENSOR_WAIT_TIME);
        ret = g_sensorInterface->Disable(iter.sensorId);
        EXPECT_EQ(SENSOR_SUCCESS, ret);
    }
    ret = g_sensorInterface->Unregister(TRADITIONAL_SENSOR_TYPE, g_traditionalCallback);
    EXPECT_EQ(SENSOR_SUCCESS, ret);
    EXPECT_EQ(SensorCallbackImpl::sensorDataFlag, SENSOR_DATA_FLAG);
    SensorCallbackImpl::sensorDataFlag = SENSOR_DATA_FLAG;
}

BENCHMARK_REGISTER_F(SensorBenchmarkTest, SetBatch)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
  * @tc.name: DriverSystem_SensorBenchmark_SetMode
  * @tc.desc: Benchmarktest for interface SetMode
  * Sets the data reporting mode for the specified sensor
  * @tc.type: FUNC
  */
BENCHMARK_F(SensorBenchmarkTest, SetMode)(benchmark::State &state)
{
    ASSERT_NE(nullptr, g_sensorInterface);
    EXPECT_GT(g_info.size(), 0);

    int32_t ret;
    EXPECT_GT(g_info.size(), 0);
    for (auto iter : g_info) {
        HDF_LOGI("get sensoriId[%{public}d], info name[%{public}s], power[%{public}f]\n\r",
            iter.sensorId, iter.sensorName.c_str(), iter.power);
        ret = g_sensorInterface->SetBatch(iter.sensorId, SENSOR_INTERVAL1, SENSOR_POLL_TIME);
        EXPECT_EQ(SENSOR_SUCCESS, ret);
        for (auto _ : state) {
            if (SENSOR_TYPE_HALL == 0) {
                ret = g_sensorInterface->SetMode(iter.sensorId, SENSOR_MODE_ON_CHANGE);
            } else {
                ret = g_sensorInterface->SetMode(iter.sensorId, SENSOR_MODE_REALTIME);
            }
            EXPECT_EQ(SENSOR_SUCCESS, ret);
        }
        ret = g_sensorInterface->Enable(iter.sensorId);
        EXPECT_EQ(SENSOR_SUCCESS, ret);
        OsalMSleep(SENSOR_WAIT_TIME);
        ret = g_sensorInterface->Disable(iter.sensorId);
        EXPECT_EQ(SENSOR_SUCCESS, ret);
    }
}

BENCHMARK_REGISTER_F(SensorBenchmarkTest, SetMode)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
  * @tc.name: DriverSystem_SensorBenchmark_SetOption
  * @tc.desc: Benchmarktest for interface SetOption
  * Sets options for the specified sensor, including its measurement range and accuracy
  * @tc.type: FUNC
  */
BENCHMARK_F(SensorBenchmarkTest, SetOption)(benchmark::State &state)
{
    ASSERT_NE(nullptr, g_sensorInterface);
    EXPECT_GT(g_info.size(), 0);

    int32_t ret;
    EXPECT_GT(g_info.size(), 0);
    for (auto iter : g_info) {
        HDF_LOGI("get sensoriId[%{public}d], info name[%{public}s], power[%{public}f]\n\r",
            iter.sensorId, iter.sensorName.c_str(), iter.power);
        for (auto _ : state) {
            ret = g_sensorInterface->SetOption(iter.sensorId, OPTION);
            EXPECT_EQ(SENSOR_SUCCESS, ret);
        }
    }
}

BENCHMARK_REGISTER_F(SensorBenchmarkTest, SetOption)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();
}

BENCHMARK_MAIN();
