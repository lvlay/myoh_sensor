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

#ifndef CONVERSION_FILTER_H
#define CONVERSION_FILTER_H

#include <algorithm>
#include <cmath>
#include <functional>
#include <numeric>

#include "utils.h"

namespace OHOS {
namespace Sensors {
namespace {
constexpr size_t ARRAY_SIZE { 10 };
} // namespace

/**
 * @brief selection of filters
 */
class ConversionFilter {
public:
    ConversionFilter() = default;
    /**
     * @brief resonant low pass filter
     *
     * @param input A signal
     * @param cutoff1 The cutoff frequency (in Hz)
     * @param resonance The amount of resonance
     */
    double FilterLowResonant(double input, double cutoff, double resonance);
    /**
     * @brief resonant high pass filter
     *
     * @param input A signal
     * @param cutoff1 The cutoff frequency (in Hz)
     * @param resonance The amount of resonance
     */
    double FilterHighResonant(double input, double cutoff, double resonance);

    /**
     * @brief resonant band pass filter
     *
     * @param input A signal
     * @param cutoff1 The cutoff frequency (in Hz)
     * @param resonance The amount of resonance
     */
    double FilterBandPass(double input, double cutoff, double resonance);

    /**
     * @brief simple low pass filter
     *
     * @param input A signal
     * @param cutoff1 The cutoff frequency (between 0 and 1)
     */
    double FilterLowPass(double input, double cutoff);

    /**
     * @brief simple high pass filter
     *
     * @param input A sampling value.
     * @param cutoff1 The cutoff frequency (between 0 and 1)
     */
    double FilterHighPass(double input, double cutoff);

    /**
     * @brief the cutoff frequency
     *
     * @param cut The cutoff frequency
     */
    void SetCutOff(double cutoff)
    {
        cutoff_ = cutoff;
    }

    /**
     * @brief the resonance
     *
     * @param res The resonance
     */
    void SetResonance(double resonance)
    {
        resonance_ = resonance;
    }

    /**
     * @brief Returns the cutoff value
     */
    double GetCutOff() const
    {
        return cutoff_;
    }

    /**
     * @brief Return the resonance value
     */
    double GetResonance() const
    {
        return resonance_;
    }

    double HandleResonant(double input, double cutoff, double resonance, FilterMethod filterMethod);
    double HandleHighPassOrLowPass(double input, double cutoff, bool isHighPass);

private:
    double cutoff_ { 0.0 };
    double resonance_ { 0.0 };
    double output_ { 0.0 };
    double sampledData_[ARRAY_SIZE] { 0.0 };
    double filteredData_[ARRAY_SIZE] { 0.0 };
    double speed_ { 0.0 };
    double pos_ { 0.0 };
    double pole_ { 0.0 };
    double filterCoefficient_ { 0.0 };
    int32_t sampleRate_ { 0 };
};
} // namespace Sensors
} // namespace OHOS
#endif // CONVERSION_FILTER_H