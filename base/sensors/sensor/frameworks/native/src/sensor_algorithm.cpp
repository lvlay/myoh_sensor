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
#include "sensor_algorithm.h"

#include <cmath>
#include <vector>

#include "sensor_errors.h"
#include "sensor_utils.h"

#undef LOG_TAG
#define LOG_TAG "SensorAlgorithmAPI"
using namespace OHOS::Sensors;

int32_t SensorAlgorithm::CreateQuaternion(std::vector<float> rotationVector, std::vector<float> &quaternion)
{
    if (static_cast<int32_t>(rotationVector.size()) < ROTATION_VECTOR_LENGTH
        || static_cast<int32_t>(rotationVector.size()) > QUATERNION_LENGTH) {
        SEN_HILOGE("Invalid input rotationVector parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    if (static_cast<int32_t>(quaternion.size()) < QUATERNION_LENGTH) {
        SEN_HILOGE("Invalid input quaternion parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    if (static_cast<int32_t>(rotationVector.size()) == ROTATION_VECTOR_LENGTH) {
        quaternion[0] = 1 - static_cast<float>((pow(rotationVector[0], 2) + pow(rotationVector[1], 2)
            + pow(rotationVector[2], 2)));
        quaternion[0]  = (quaternion[0] > 0) ? static_cast<float>(std::sqrt(quaternion[0])) : 0;
    } else {
        quaternion[0] = rotationVector[3];
    }
    quaternion[1] = rotationVector[0];
    quaternion[2] = rotationVector[1];
    quaternion[3] = rotationVector[2];
    return OHOS::Sensors::SUCCESS;
}

int32_t SensorAlgorithm::TransformCoordinateSystemImpl(std::vector<float> inRotationMatrix, int32_t axisX,
    int32_t axisY, std::vector<float> &outRotationMatrix)
{
    if ((axisX & 0x7C) != 0 || (axisX & 0x3) == 0) {
        SEN_HILOGE("axisX is invalid parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    if ((axisY & 0x7C) != 0 || (axisY & 0x3) == 0 || (axisX & 0x3) == (axisY & 0x3)) {
        SEN_HILOGE("axisY is invalid parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    int32_t axisZ = axisX ^ axisY;
    int32_t x = (axisX & 0x3) - 1;
    int32_t y = (axisY & 0x3) - 1;
    int32_t z = (axisZ & 0x3) - 1;
    if (((x ^ ((z + 1) % 3)) | (y ^ ((z + 2) % 3))) != 0) {
        axisZ ^= 0x80;
    }
    int32_t inRotationMatrixLength = static_cast<int32_t>(inRotationMatrix.size());
    int32_t matrixDimension = ((inRotationMatrixLength == FOUR_DIMENSIONAL_MATRIX_LENGTH)
        ? QUATERNION_LENGTH : ROTATION_VECTOR_LENGTH);
    for (int32_t j = 0; j < ROTATION_VECTOR_LENGTH; j++) {
        int32_t offset = j * matrixDimension;
        for (int32_t i = 0; i < 3; i++) {
            if (x == i) {
                outRotationMatrix[offset + i] = (axisX >= 0x80) ? -inRotationMatrix[offset + 0] :
                    inRotationMatrix[offset + 0];
            }
            if (y == i) {
                outRotationMatrix[offset + i] = (axisY >= 0x80) ? -inRotationMatrix[offset + 1] :
                    inRotationMatrix[offset + 1];
            }
            if (z == i) {
                outRotationMatrix[offset + i] = (axisZ >= 0x80) ? -inRotationMatrix[offset + 2] :
                    inRotationMatrix[offset + 2];
            }
        }
    }
    if (inRotationMatrixLength == FOUR_DIMENSIONAL_MATRIX_LENGTH) {
        outRotationMatrix[3] = outRotationMatrix[7] = outRotationMatrix[11] =
            outRotationMatrix[12] = outRotationMatrix[13] = outRotationMatrix[14] = 0;
        outRotationMatrix[15] = 1;
    }
    return OHOS::Sensors::SUCCESS;
}

int32_t SensorAlgorithm::TransformCoordinateSystem(std::vector<float> inRotationMatrix, int32_t axisX, int32_t axisY,
    std::vector<float> &outRotationMatrix)
{
    if (axisX < 0 || axisY < 0) {
        SEN_HILOGE("Invalid axisX or axisY");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    int32_t inRotationMatrixLength = static_cast<int32_t>(inRotationMatrix.size());
    if (((inRotationMatrixLength != THREE_DIMENSIONAL_MATRIX_LENGTH) &&
        (inRotationMatrixLength != FOUR_DIMENSIONAL_MATRIX_LENGTH)) ||
        (inRotationMatrixLength != static_cast<int32_t>(outRotationMatrix.size()))) {
        SEN_HILOGE("Invalid input parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    if (inRotationMatrix == outRotationMatrix) {
        std::vector<float> tempRotationMatrix(inRotationMatrixLength);
        if (TransformCoordinateSystemImpl(inRotationMatrix, axisX, axisY, tempRotationMatrix)
            != OHOS::Sensors::SUCCESS) {
            SEN_HILOGE("TransformCoordinateSystemImpl failed");
            return OHOS::Sensors::PARAMETER_ERROR;
        }
        for (int32_t i = 0; i < inRotationMatrixLength; i++) {
            outRotationMatrix[i] = tempRotationMatrix[i];
        }
        return OHOS::Sensors::SUCCESS;
    }
    return TransformCoordinateSystemImpl(inRotationMatrix, axisX, axisY, outRotationMatrix);
}

int32_t SensorAlgorithm::GetAltitude(float seaPressure, float currentPressure, float *altitude)
{
    if (altitude == nullptr) {
        SEN_HILOGE("Invalid parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    float coef = 1.0f / RECIPROCAL_COEFFICIENT;
    float rationOfStandardPressure = IsEqual(seaPressure, 0.0f) ?
        std::numeric_limits<float>::max() : currentPressure / seaPressure;
    float difference = pow(rationOfStandardPressure, coef);
    *altitude = ZERO_PRESSURE_ALTITUDE * (1.0f - difference);
    return OHOS::Sensors::SUCCESS;
}

int32_t SensorAlgorithm::GetGeomagneticDip(std::vector<float> inclinationMatrix, float *geomagneticDip)
{
    if (geomagneticDip == nullptr) {
        SEN_HILOGE("Invalid parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    int32_t matrixLength = static_cast<int32_t>(inclinationMatrix.size());
    if (matrixLength != THREE_DIMENSIONAL_MATRIX_LENGTH && matrixLength != FOUR_DIMENSIONAL_MATRIX_LENGTH) {
        SEN_HILOGE("Invalid input parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    if (matrixLength == THREE_DIMENSIONAL_MATRIX_LENGTH) {
        *geomagneticDip = std::atan2(inclinationMatrix[5], inclinationMatrix[4]);
    } else {
        *geomagneticDip = std::atan2(inclinationMatrix[6], inclinationMatrix[5]);
    }
    return OHOS::Sensors::SUCCESS;
}

int32_t SensorAlgorithm::GetAngleModify(std::vector<float> curRotationMatrix, std::vector<float> preRotationMatrix,
    std::vector<float> &angleChange)
{
    if (static_cast<int32_t>(angleChange.size()) < ROTATION_VECTOR_LENGTH) {
        SEN_HILOGE("Invalid parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    int32_t curRotationMatrixLength = static_cast<int32_t>(curRotationMatrix.size());
    int32_t preRotationMatrixLength = static_cast<int32_t>(preRotationMatrix.size());
    if ((curRotationMatrixLength != FOUR_DIMENSIONAL_MATRIX_LENGTH)
        && (curRotationMatrixLength != THREE_DIMENSIONAL_MATRIX_LENGTH)) {
        SEN_HILOGE("Invalid input curRotationMatrix parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    if ((preRotationMatrixLength != FOUR_DIMENSIONAL_MATRIX_LENGTH)
        && (preRotationMatrixLength != THREE_DIMENSIONAL_MATRIX_LENGTH)) {
        SEN_HILOGE("Invalid input currotationMatrix parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    float curMatrix[THREE_DIMENSIONAL_MATRIX_LENGTH] = {0};
    float preMatrix[THREE_DIMENSIONAL_MATRIX_LENGTH] = {0};
    int32_t curmatrixDimension = ((curRotationMatrixLength == FOUR_DIMENSIONAL_MATRIX_LENGTH)
        ? QUATERNION_LENGTH : ROTATION_VECTOR_LENGTH);
    int32_t prematrixDimension = ((preRotationMatrixLength == FOUR_DIMENSIONAL_MATRIX_LENGTH)
        ? QUATERNION_LENGTH : ROTATION_VECTOR_LENGTH);
    for (int32_t i = 0; i < THREE_DIMENSIONAL_MATRIX_LENGTH; i++) {
        int32_t curMatrixIndex = i % ROTATION_VECTOR_LENGTH + (i / ROTATION_VECTOR_LENGTH) * curmatrixDimension;
        curMatrix[i] = curRotationMatrix[curMatrixIndex];
        int32_t preMatrixIndex = i % ROTATION_VECTOR_LENGTH + (i / ROTATION_VECTOR_LENGTH) * prematrixDimension;
        preMatrix[i] = preRotationMatrix[preMatrixIndex];
    }
    float radian[THREE_DIMENSIONAL_MATRIX_LENGTH] = {0};
    radian[1] = preMatrix[0] * curMatrix[1] + preMatrix[3] * curMatrix[4] + preMatrix[6] * curMatrix[7];
    radian[4] = preMatrix[1] * curMatrix[1] + preMatrix[4] * curMatrix[4] + preMatrix[7] * curMatrix[7];
    radian[6] = preMatrix[2] * curMatrix[0] + preMatrix[5] * curMatrix[3] + preMatrix[8] * curMatrix[6];
    radian[7] = preMatrix[2] * curMatrix[1] + preMatrix[5] * curMatrix[4] + preMatrix[8] * curMatrix[7];
    radian[8] = preMatrix[2] * curMatrix[2] + preMatrix[5] * curMatrix[5] + preMatrix[8] * curMatrix[8];
    angleChange[0] = static_cast<float>(std::atan2(radian[1], radian[4]));
    angleChange[1] = static_cast<float>(std::asin(-radian[7]));
    angleChange[2] = static_cast<float>(std::atan2(-radian[6], radian[8]));
    return OHOS::Sensors::SUCCESS;
}

int32_t SensorAlgorithm::GetDirection(std::vector<float> rotationMatrix, std::vector<float> &rotationAngle)
{
    if (static_cast<int32_t>(rotationAngle.size()) < ROTATION_VECTOR_LENGTH) {
        SEN_HILOGE("Invalid parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    int32_t rotationMatrixLength = static_cast<int32_t>(rotationMatrix.size());
    if ((rotationMatrixLength != FOUR_DIMENSIONAL_MATRIX_LENGTH)
        && (rotationMatrixLength != THREE_DIMENSIONAL_MATRIX_LENGTH)) {
        SEN_HILOGE("Invalid input rotationMatrix parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    int32_t dimension = ((rotationMatrixLength == FOUR_DIMENSIONAL_MATRIX_LENGTH)
        ? QUATERNION_LENGTH : ROTATION_VECTOR_LENGTH);
    rotationAngle[0] = static_cast<float>(std::atan2(rotationMatrix[1],
        rotationMatrix[dimension * 1 + 1]));
    rotationAngle[1] = static_cast<float>(std::atan2(-rotationMatrix[2 * dimension + 1],
        std::sqrt(pow(rotationMatrix[1], 2) + pow(rotationMatrix[dimension + 1], 2))));
    rotationAngle[2] = static_cast<float>(std::atan2(-rotationMatrix[2 * dimension],
        rotationMatrix[2 * dimension + 2]));
    return OHOS::Sensors::SUCCESS;
}

int32_t SensorAlgorithm::CreateRotationMatrix(std::vector<float> rotationVector, std::vector<float> &rotationMatrix)
{
    int32_t rotationMatrixLength = static_cast<int32_t>(rotationMatrix.size());
    if ((static_cast<int32_t>(rotationVector.size()) < ROTATION_VECTOR_LENGTH)
        || ((rotationMatrixLength != FOUR_DIMENSIONAL_MATRIX_LENGTH)
        && (rotationMatrixLength != THREE_DIMENSIONAL_MATRIX_LENGTH))) {
        SEN_HILOGE("Invalid input rotationMatrix parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    std::vector<float> quaternion(4);
    int32_t ret = CreateQuaternion(rotationVector, quaternion);
    if (ret != OHOS::Sensors::SUCCESS) {
        SEN_HILOGE("Create quaternion failed");
        return ret;
    }
    float squareOfX = 2 * static_cast<float>(pow(quaternion[1], 2));
    float squareOfY = 2 * static_cast<float>(pow(quaternion[2], 2));
    float squareOfZ = 2 * static_cast<float>(pow(quaternion[3], 2));
    float productOfWZ = 2 * quaternion[0] * quaternion[3];
    float productOfXY = 2 * quaternion[1] * quaternion[2];
    float productOfWY = 2 * quaternion[0] * quaternion[2];
    float productOfXZ = 2 * quaternion[1] * quaternion[3];
    float productOfWX = 2 * quaternion[0] * quaternion[1];
    float productOfYZ = 2 * quaternion[2] * quaternion[3];
    int32_t rotationMatrixDimension =  ((rotationMatrixLength == FOUR_DIMENSIONAL_MATRIX_LENGTH)
        ? QUATERNION_LENGTH : ROTATION_VECTOR_LENGTH);
    rotationMatrix[0] = 1 - squareOfY - squareOfZ;
    rotationMatrix[1] = productOfXY - productOfWZ;
    rotationMatrix[2] = productOfXZ + productOfWY;
    rotationMatrix[3 % ROTATION_VECTOR_LENGTH + (3 / ROTATION_VECTOR_LENGTH) * rotationMatrixDimension]
        = productOfXY + productOfWZ;
    rotationMatrix[4 % ROTATION_VECTOR_LENGTH + (4 / ROTATION_VECTOR_LENGTH) * rotationMatrixDimension]
        = 1 - squareOfX - squareOfZ;
    rotationMatrix[5 % ROTATION_VECTOR_LENGTH + (5 / ROTATION_VECTOR_LENGTH) * rotationMatrixDimension]
        = productOfYZ - productOfWX;
    rotationMatrix[6 % ROTATION_VECTOR_LENGTH + (6 / ROTATION_VECTOR_LENGTH) * rotationMatrixDimension]
        = productOfXZ - productOfWY;
    rotationMatrix[7 % ROTATION_VECTOR_LENGTH + (7 / ROTATION_VECTOR_LENGTH) * rotationMatrixDimension]
        = productOfYZ + productOfWX;
    rotationMatrix[8 % ROTATION_VECTOR_LENGTH + (8 / ROTATION_VECTOR_LENGTH) * rotationMatrixDimension]
        = 1 - squareOfX - squareOfY;
    if (rotationMatrixLength == FOUR_DIMENSIONAL_MATRIX_LENGTH) {
        rotationMatrix[3] = rotationMatrix[7] = rotationMatrix[11] = rotationMatrix[12] = rotationMatrix[13]
            = rotationMatrix[14] = 0.0f;
        rotationMatrix[15] = 1.0f;
    }
    return OHOS::Sensors::SUCCESS;
}

int32_t SensorAlgorithm::CreateRotationAndInclination(std::vector<float> gravity, std::vector<float> geomagnetic,
    std::vector<float> &rotationMatrix, std::vector<float> &inclinationMatrix)
{
    if (static_cast<int32_t>(gravity.size()) < ROTATION_VECTOR_LENGTH
        || static_cast<int32_t>(geomagnetic.size()) < ROTATION_VECTOR_LENGTH) {
        SEN_HILOGE("Invalid input parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    float totalGravity = pow(gravity[0], 2) + pow(gravity[1], 2) + pow(gravity[2], 2);
    if (totalGravity < (0.01f * pow(GRAVITATIONAL_ACCELERATION, 2))) {
        SEN_HILOGE("Invalid input gravity parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    std::vector<float> componentH(3);
    componentH[0] = geomagnetic[1] * gravity[2] - geomagnetic[2] * gravity[1];
    componentH[1] = geomagnetic[2] * gravity[0] - geomagnetic[0] * gravity[2];
    componentH[2] = geomagnetic[0] * gravity[1] - geomagnetic[1] * gravity[0];
    float totalH = static_cast<float>(std::sqrt(pow(componentH[0], 2) + pow(componentH[1], 2)
        + pow(componentH[2], 2)));
    if (totalH < 0.1f) {
        SEN_HILOGE("The total strength of H is less than 0.1");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    float reciprocalH = 1.0f / totalH;
    componentH[0] *= reciprocalH;
    componentH[1] *= reciprocalH;
    componentH[2] *= reciprocalH;
    float reciprocalA = 1.0f / static_cast<float>(std::sqrt(totalGravity));
    gravity[0] *= reciprocalA;
    gravity[1] *= reciprocalA;
    gravity[2] *= reciprocalA;

    std::vector<float> measuredValue(3);
    measuredValue[0] = gravity[1] * componentH[2] - gravity[2] * componentH[1];
    measuredValue[1] = gravity[2] * componentH[0] - gravity[0] * componentH[2];
    measuredValue[2] = gravity[0] * componentH[1] - gravity[1] * componentH[0];
    int32_t rotationMatrixLength = static_cast<int32_t>(rotationMatrix.size());
    int32_t inclinationMatrixLength = static_cast<int32_t>(inclinationMatrix.size());
    if ((rotationMatrixLength != 9 && rotationMatrixLength != 16) || (inclinationMatrixLength != 9
        && inclinationMatrixLength != 16)) {
        SEN_HILOGE("Invalid input parameter");
        return OHOS::Sensors::PARAMETER_ERROR;
    }
    float e = static_cast<float>(std::sqrt(pow(geomagnetic[0], 2) + pow(geomagnetic[1], 2) + pow(geomagnetic[2], 2)));
    float reciprocalE = IsEqual(e, 0.0f) ? std::numeric_limits<float>::max() : 1.0f / e;
    float c = (geomagnetic[0] * measuredValue[0] + geomagnetic[1] * measuredValue[1]
        + geomagnetic[2] * measuredValue[2]) * reciprocalE;
    float s = (geomagnetic[0] * gravity[0] + geomagnetic[1] * gravity[1] + geomagnetic[2] * gravity[2]) * reciprocalE;

    int32_t rotationMatrixDimension =  ((rotationMatrixLength == FOUR_DIMENSIONAL_MATRIX_LENGTH)
        ? QUATERNION_LENGTH : ROTATION_VECTOR_LENGTH);
    int32_t inclinationMatrixDimension =  ((inclinationMatrixLength == FOUR_DIMENSIONAL_MATRIX_LENGTH)
        ? QUATERNION_LENGTH : ROTATION_VECTOR_LENGTH);
    rotationMatrix[0] = componentH[0];
    rotationMatrix[1] = componentH[1];
    rotationMatrix[2] = componentH[2];
    rotationMatrix[3 % ROTATION_VECTOR_LENGTH + (3 / ROTATION_VECTOR_LENGTH) * rotationMatrixDimension] =
        measuredValue[0];
    rotationMatrix[4 % ROTATION_VECTOR_LENGTH + (4 / ROTATION_VECTOR_LENGTH) * rotationMatrixDimension] =
        measuredValue[1];
    rotationMatrix[5 % ROTATION_VECTOR_LENGTH + (5 / ROTATION_VECTOR_LENGTH) * rotationMatrixDimension] =
        measuredValue[2];
    rotationMatrix[6 % ROTATION_VECTOR_LENGTH + (6 / ROTATION_VECTOR_LENGTH) * rotationMatrixDimension] = gravity[0];
    rotationMatrix[7 % ROTATION_VECTOR_LENGTH + (7 / ROTATION_VECTOR_LENGTH) * rotationMatrixDimension] = gravity[1];
    rotationMatrix[8 % ROTATION_VECTOR_LENGTH + (8 / ROTATION_VECTOR_LENGTH) * rotationMatrixDimension] = gravity[2];
    if (rotationMatrixLength == FOUR_DIMENSIONAL_MATRIX_LENGTH) {
        rotationMatrix[3] = rotationMatrix[7] = rotationMatrix[11] = rotationMatrix[12]
            = rotationMatrix[13] = rotationMatrix[14] = 0.0f;
        rotationMatrix[15] = 1.0f;
    }
    inclinationMatrix[0] = 1;
    inclinationMatrix[1] = 0;
    inclinationMatrix[2] = 0;
    inclinationMatrix[3 % ROTATION_VECTOR_LENGTH + (3 / ROTATION_VECTOR_LENGTH) * inclinationMatrixDimension] = 0;
    inclinationMatrix[4 % ROTATION_VECTOR_LENGTH + (4 / ROTATION_VECTOR_LENGTH) * inclinationMatrixDimension] = c;
    inclinationMatrix[5 % ROTATION_VECTOR_LENGTH + (5 / ROTATION_VECTOR_LENGTH) * inclinationMatrixDimension] = s;
    inclinationMatrix[6 % ROTATION_VECTOR_LENGTH + (6 / ROTATION_VECTOR_LENGTH) * inclinationMatrixDimension] = 0;
    inclinationMatrix[7 % ROTATION_VECTOR_LENGTH + (7 / ROTATION_VECTOR_LENGTH) * inclinationMatrixDimension] = -s;
    inclinationMatrix[8 % ROTATION_VECTOR_LENGTH + (8 / ROTATION_VECTOR_LENGTH) * inclinationMatrixDimension] = c;
    if (rotationMatrixLength == FOUR_DIMENSIONAL_MATRIX_LENGTH) {
        inclinationMatrix[3] = inclinationMatrix[7] = inclinationMatrix[11] = inclinationMatrix[12]
            = inclinationMatrix[13] = inclinationMatrix[14] = 0.0f;
        inclinationMatrix[15] = 1.0f;
    }
    return OHOS::Sensors::SUCCESS;
}