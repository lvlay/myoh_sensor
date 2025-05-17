/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "geomagnetic_field.h"
#include "sensor_algorithm.h"
#include "sensor_errors.h"

#undef LOG_TAG
#define LOG_TAG "SensorAlgorithmTest"

namespace OHOS {
namespace Sensors {
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
constexpr int32_t QUATERNION_LENGTH = 4;
constexpr int32_t ROTATION_VECTOR_LENGTH = 3;
constexpr int32_t THREE_DIMENSIONAL_MATRIX_LENGTH = 9;
constexpr float EPS = 0.01;
} // namespace

class SensorAlgorithmTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void SensorAlgorithmTest::SetUpTestCase() {}

void SensorAlgorithmTest::TearDownTestCase() {}

void SensorAlgorithmTest::SetUp() {}

void SensorAlgorithmTest::TearDown() {}

SensorAlgorithm sensorAlgorithm;

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_001, TestSize.Level1)
{
    SEN_HILOGI("SensorAlgorithmTest_001 in");
    std::vector<float> rotationVector = {0.52, -0.336, -0.251};
    std::vector<float> quaternion(QUATERNION_LENGTH);
    int32_t ret = sensorAlgorithm.CreateQuaternion(rotationVector, quaternion);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ASSERT_EQ(quaternion.size(), QUATERNION_LENGTH);
    std::vector<float> result = {0.7441122531890869, 0.5199999809265137, -0.335999995470047, -0.25099998712539673};
    for (size_t i = 0; i < QUATERNION_LENGTH; ++i) {
        ASSERT_EQ(quaternion[i], result[i]);
    }
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_002, TestSize.Level1)
{
    SEN_HILOGI("SensorAlgorithmTest_002 in");
    std::vector<float> rotationVector = {};
    std::vector<float> quaternion(QUATERNION_LENGTH);
    int32_t ret = sensorAlgorithm.CreateQuaternion(rotationVector, quaternion);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_003, TestSize.Level1)
{
    SEN_HILOGI("SensorAlgorithmTest_003 in");
    std::vector<float> rotationVector = {0.52, -0.336, -0.251};
    std::vector<float> quaternion(ROTATION_VECTOR_LENGTH - 1);
    int32_t ret = sensorAlgorithm.CreateQuaternion(rotationVector, quaternion);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_004, TestSize.Level1)
{
    SEN_HILOGI("SensorAlgorithmTest_004 in");
    std::vector<float> inRotationMatrix = {1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5};
    std::vector<float> outRotationMatrix(THREE_DIMENSIONAL_MATRIX_LENGTH);
    int32_t ret = sensorAlgorithm.TransformCoordinateSystem(inRotationMatrix, 1, 2, outRotationMatrix);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::vector<float> result =
        {1.500000, 1.500000, 1.500000, 1.500000, 1.500000, 1.500000, 1.500000, 1.500000, 1.500000};
    ASSERT_EQ(outRotationMatrix.size(), THREE_DIMENSIONAL_MATRIX_LENGTH);
    for (size_t i = 0; i < THREE_DIMENSIONAL_MATRIX_LENGTH; ++i) {
        ASSERT_EQ(outRotationMatrix[i], result[i]);
    }
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_005, TestSize.Level1)
{
    SEN_HILOGI("SensorAlgorithmTest_005 in");
    std::vector<float> inRotationMatrix(3);
    std::vector<float> outRotationMatrix(THREE_DIMENSIONAL_MATRIX_LENGTH);
    int32_t ret = sensorAlgorithm.TransformCoordinateSystem(inRotationMatrix, 1, 2, outRotationMatrix);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_006, TestSize.Level1)
{
    SEN_HILOGI("SensorAlgorithmTest_006 in");
    std::vector<float> inRotationMatrix = {1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5};
    std::vector<float> outRotationMatrix(THREE_DIMENSIONAL_MATRIX_LENGTH);
    int32_t ret = sensorAlgorithm.TransformCoordinateSystem(inRotationMatrix, 1, -1, outRotationMatrix);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_007, TestSize.Level1)
{
    SEN_HILOGI("SensorAlgorithmTest_007 in");
    std::vector<float> inRotationMatrix = {1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5};
    std::vector<float> outRotationMatrix(THREE_DIMENSIONAL_MATRIX_LENGTH);
    int32_t ret = sensorAlgorithm.TransformCoordinateSystem(inRotationMatrix, -1, 1, outRotationMatrix);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_008, TestSize.Level1)
{
    float altitude = -1.0;
    int32_t ret = sensorAlgorithm.GetAltitude(5.0, 0.0, &altitude);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ASSERT_EQ(altitude, 44330.0);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_009, TestSize.Level1)
{
    int32_t ret = sensorAlgorithm.GetAltitude(5.0, 0.0, nullptr);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_010, TestSize.Level1)
{
    float geomagneticDip = -1.0;
    std::vector<float> inclinationMatrix = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    int32_t ret = sensorAlgorithm.GetGeomagneticDip(inclinationMatrix, &geomagneticDip);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ASSERT_EQ(geomagneticDip, 0.8760581016540527);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_011, TestSize.Level1)
{
    std::vector<float> inclinationMatrix = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    int32_t ret = sensorAlgorithm.GetGeomagneticDip(inclinationMatrix, nullptr);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_012, TestSize.Level1)
{
    std::vector<float> inclinationMatrix(3);
    float geomagneticDip = -1.0;
    int32_t ret = sensorAlgorithm.GetGeomagneticDip(inclinationMatrix, &geomagneticDip);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_013, TestSize.Level1)
{
    std::vector<float> currotationMatrix = {1.17549e-38, 1.17549e-38, 1.17549e-38,
        1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38};
    std::vector<float> preRotationMatrix = {1.17549e-38, 1.17549e-38, 1.17549e-38,
        1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38};
    std::vector<float> angleChange(ROTATION_VECTOR_LENGTH);
    int32_t ret = sensorAlgorithm.GetAngleModify(currotationMatrix, preRotationMatrix, angleChange);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ASSERT_EQ(angleChange.size(), ROTATION_VECTOR_LENGTH);
    std::vector<float> result = {0.0, -0.0, -0.0};
    for (size_t i = 0; i < ROTATION_VECTOR_LENGTH; ++i) {
        ASSERT_EQ(angleChange[i], result[i]);
    }
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_014, TestSize.Level1)
{
    std::vector<float> currotationMatrix(3);
    std::vector<float> preRotationMatrix = {1.17549e-38, 1.17549e-38, 1.17549e-38,
        1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38};
    std::vector<float> angleChange(ROTATION_VECTOR_LENGTH);
    int32_t ret = sensorAlgorithm.GetAngleModify(currotationMatrix, preRotationMatrix, angleChange);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_015, TestSize.Level1)
{
    std::vector<float> currotationMatrix = {1.17549e-38, 1.17549e-38, 1.17549e-38,
        1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38};
    std::vector<float> preRotationMatrix(ROTATION_VECTOR_LENGTH);
    std::vector<float> angleChange(ROTATION_VECTOR_LENGTH);
    int32_t ret = sensorAlgorithm.GetAngleModify(currotationMatrix, preRotationMatrix, angleChange);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_016, TestSize.Level1)
{
    std::vector<float> currotationMatrix = {1.17549e-38, 1.17549e-38, 1.17549e-38,
        1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38};
    std::vector<float> preRotationMatrix = {1.17549e-38, 1.17549e-38, 1.17549e-38,
        1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38, 1.17549e-38};
    std::vector<float> angleChange(ROTATION_VECTOR_LENGTH - 1);
    int32_t ret = sensorAlgorithm.GetAngleModify(currotationMatrix, preRotationMatrix, angleChange);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_017, TestSize.Level1)
{
    std::vector<float> rotationMatrix = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    std::vector<float> rotationAngle(ROTATION_VECTOR_LENGTH);
    int32_t ret = sensorAlgorithm.GetDirection(rotationMatrix, rotationAngle);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::vector<float> result = {0.38050639629364014, -0.9783217310905457, -0.6610431671142578};
    ASSERT_EQ(rotationAngle.size(), ROTATION_VECTOR_LENGTH);
    for (size_t i = 0; i < ROTATION_VECTOR_LENGTH; ++i) {
        ASSERT_EQ(rotationAngle[i], result[i]);
    }
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_018, TestSize.Level1)
{
    std::vector<float> rotationMatrix(5);
    std::vector<float> rotationAngle(ROTATION_VECTOR_LENGTH);
    int32_t ret = sensorAlgorithm.GetDirection(rotationMatrix, rotationAngle);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_019, TestSize.Level1)
{
    std::vector<float> rotationMatrix = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    std::vector<float> rotationAngle(ROTATION_VECTOR_LENGTH - 1);
    int32_t ret = sensorAlgorithm.GetDirection(rotationMatrix, rotationAngle);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_020, TestSize.Level1)
{
    std::vector<float> rotationVector = {0.0, 0.0, 0.0};
    std::vector<float> rotationMatrix(THREE_DIMENSIONAL_MATRIX_LENGTH);
    int32_t ret = sensorAlgorithm.CreateRotationMatrix(rotationVector, rotationMatrix);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    std::vector<float> result = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    ASSERT_EQ(rotationMatrix.size(), THREE_DIMENSIONAL_MATRIX_LENGTH);
    for (size_t i = 0; i < THREE_DIMENSIONAL_MATRIX_LENGTH; ++i) {
        ASSERT_EQ(rotationMatrix[i], result[i]);
    }
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_021, TestSize.Level1)
{
    std::vector<float> rotationVector(ROTATION_VECTOR_LENGTH - 1);
    std::vector<float> rotationMatrix(THREE_DIMENSIONAL_MATRIX_LENGTH);
    int32_t ret = sensorAlgorithm.CreateRotationMatrix(rotationVector, rotationMatrix);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_022, TestSize.Level1)
{
    std::vector<float> rotationVector = {0.0, 0.0, 0.0};
    std::vector<float> rotationMatrix(ROTATION_VECTOR_LENGTH - 1);
    int32_t ret = sensorAlgorithm.CreateRotationMatrix(rotationVector, rotationMatrix);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_023, TestSize.Level1)
{
    std::vector<float> gravity = {9.0, 9.0, 9.0};
    std::vector<float> geomagnetic = {30.0, 25.0, 41.0};
    std::vector<float> rotationMatrix(THREE_DIMENSIONAL_MATRIX_LENGTH);
    std::vector<float> inclinationMatrix(THREE_DIMENSIONAL_MATRIX_LENGTH);
    int32_t ret = sensorAlgorithm.CreateRotationAndInclination(gravity, geomagnetic, rotationMatrix, inclinationMatrix);
    ASSERT_EQ(ret, OHOS::Sensors::SUCCESS);
    ASSERT_EQ(rotationMatrix.size(), THREE_DIMENSIONAL_MATRIX_LENGTH);
    ASSERT_EQ(inclinationMatrix.size(), THREE_DIMENSIONAL_MATRIX_LENGTH);
    std::vector<float> rotationMatrixResult =
        {-0.7980074882507324, 0.5486301183700562, 0.24937734007835388, -0.17277367413043976,
        -0.6047078967094421, 0.7774815559387207, 0.5773502588272095, 0.5773502588272095, 0.5773502588272095};
    std::vector<float> inclinationMatrixResult = {1.0, 0.0, 0.0, 0.0, 0.20444221794605255,
        0.9788785576820374, 0, -0.9788785576820374, 0.20444221794605255};
    for (size_t i = 0; i < THREE_DIMENSIONAL_MATRIX_LENGTH; ++i) {
        ASSERT_EQ(rotationMatrix[i], rotationMatrixResult[i]);
        ASSERT_EQ(inclinationMatrix[i], inclinationMatrixResult[i]);
    }
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_024, TestSize.Level1)
{
    std::vector<float> gravity(ROTATION_VECTOR_LENGTH - 1);
    std::vector<float> geomagnetic = {30.0, 25.0, 41.0};
    std::vector<float> rotationMatrix(THREE_DIMENSIONAL_MATRIX_LENGTH);
    std::vector<float> inclinationMatrix(THREE_DIMENSIONAL_MATRIX_LENGTH);
    int32_t ret = sensorAlgorithm.CreateRotationAndInclination(gravity, geomagnetic, rotationMatrix, inclinationMatrix);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_025, TestSize.Level1)
{
    std::vector<float> gravity = {9.0, 9.0, 9.0};
    std::vector<float> geomagnetic(ROTATION_VECTOR_LENGTH - 1);
    std::vector<float> rotationMatrix(THREE_DIMENSIONAL_MATRIX_LENGTH);
    std::vector<float> inclinationMatrix(THREE_DIMENSIONAL_MATRIX_LENGTH);
    int32_t ret = sensorAlgorithm.CreateRotationAndInclination(gravity, geomagnetic, rotationMatrix, inclinationMatrix);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_026, TestSize.Level1)
{
    std::vector<float> gravity = {9.0, 9.0, 9.0};
    std::vector<float> geomagnetic = {30.0, 25.0, 41.0};
    std::vector<float> rotationMatrix(ROTATION_VECTOR_LENGTH - 1);
    std::vector<float> inclinationMatrix(THREE_DIMENSIONAL_MATRIX_LENGTH);
    int32_t ret = sensorAlgorithm.CreateRotationAndInclination(gravity, geomagnetic, rotationMatrix, inclinationMatrix);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_027, TestSize.Level1)
{
    std::vector<float> gravity = {9.0, 9.0, 9.0};
    std::vector<float> geomagnetic = {30.0, 25.0, 41.0};
    std::vector<float> rotationMatrix(THREE_DIMENSIONAL_MATRIX_LENGTH);
    std::vector<float> inclinationMatrix(ROTATION_VECTOR_LENGTH - 1);
    int32_t ret = sensorAlgorithm.CreateRotationAndInclination(gravity, geomagnetic, rotationMatrix, inclinationMatrix);
    ASSERT_EQ(ret, OHOS::Sensors::PARAMETER_ERROR);
}

HWTEST_F(SensorAlgorithmTest, SensorAlgorithmTest_028, TestSize.Level1)
{
    GeomagneticField geomagneticField(80.0, 0.0, 0.0, 1580486400000);
    ASSERT_TRUE(fabs(geomagneticField.ObtainX() - 6570.3935546875) < EPS);
    ASSERT_TRUE(fabs(geomagneticField.ObtainY() - -146.3289337158203) < EPS);
    ASSERT_TRUE(fabs(geomagneticField.ObtainZ() - 54606.0078125) < EPS);
    ASSERT_TRUE(fabs(geomagneticField.ObtainGeomagneticDip() - 83.13726043701172) < EPS);
    ASSERT_TRUE(fabs(geomagneticField.ObtainDeflectionAngle() - -1.2758207321166992) < EPS);
    ASSERT_TRUE(fabs(geomagneticField.ObtainLevelIntensity() - 6572.02294921875) < EPS);
    ASSERT_TRUE(fabs(geomagneticField.ObtainTotalIntensity() - 55000.0703125) < EPS);
}
} // namespace Sensors
} // namespace OHOS
