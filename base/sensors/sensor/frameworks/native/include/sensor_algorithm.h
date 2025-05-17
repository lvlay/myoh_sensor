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

#ifndef SENSOR_ALGORITHM_H
#define SENSOR_ALGORITHM_H

#include <cstdint>
#include <vector>

class SensorAlgorithm {
public:
    SensorAlgorithm() = default;
    ~SensorAlgorithm() = default;
    int32_t CreateQuaternion(std::vector<float> rotationVector, std::vector<float> &quaternion);
    int32_t TransformCoordinateSystem(std::vector<float> inRotationMatrix, int32_t axisX,
                                    int32_t axisY, std::vector<float> &outRotationMatrix);
    int32_t GetAltitude(float seaPressure, float currentPressure, float *altitude);
    int32_t GetGeomagneticDip(std::vector<float> inclinationMatrix, float *geomagneticDip);
    int32_t GetAngleModify(std::vector<float> currotationMatrix, std::vector<float> prerotationMatrix,
                        std::vector<float> &angleChange);
    int32_t GetDirection(std::vector<float> rotationMatrix, std::vector<float> &rotationAngle);
    int32_t CreateRotationMatrix(std::vector<float> rotationVector, std::vector<float> &rotationMatrix);
    int32_t CreateRotationAndInclination(std::vector<float> gravity, std::vector<float> geomagnetic,
                                        std::vector<float> &rotationMatrix, std::vector<float> &inclinationMatrix);

private:
    int32_t TransformCoordinateSystemImpl(std::vector<float> inRotationMatrix, int32_t axisX,
                                                           int32_t axisY, std::vector<float> &outRotationMatrix);
    static constexpr int32_t QUATERNION_LENGTH = 4;
    static constexpr int32_t ROTATION_VECTOR_LENGTH = 3;
    static constexpr int32_t THREE_DIMENSIONAL_MATRIX_LENGTH = 9;
    static constexpr int32_t FOUR_DIMENSIONAL_MATRIX_LENGTH = 16;
    static constexpr float GRAVITATIONAL_ACCELERATION = 9.81f;
    static constexpr float RECIPROCAL_COEFFICIENT = 5.255f;
    static constexpr float ZERO_PRESSURE_ALTITUDE = 44330.0f;
};
#endif // SENSOR_ALGORITHM_H
