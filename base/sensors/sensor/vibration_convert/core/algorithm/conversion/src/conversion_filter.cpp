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

#include "conversion_filter.h"

#include "sensor_log.h"

#undef LOG_TAG
#define LOG_TAG "ConversionFilter"

namespace OHOS {
namespace Sensors {
namespace {
constexpr double CUT_OFF_MIN { 10.0 };
constexpr double VALIDE_RESONANCE_MAX { 0.999999 };
constexpr double AMOUNT_RESONANCE_MIN { 1.0 };
constexpr double BAND_PASS_COEF { 4.0 };
} // namespace

// I particularly like these. cutoff between 0 and 1
double ConversionFilter::FilterLowPass(double input, double cutoff)
{
    output_ = filteredData_[0] + cutoff * (input - filteredData_[0]);
    filteredData_[0] = output_;
    return output_;
}

// as above
double ConversionFilter::FilterHighPass(double input, double cutoff)
{
    output_ = input - (filteredData_[0] + cutoff * (input - filteredData_[0]));
    filteredData_[0] = output_;
    return output_;
}

// awesome. cuttof is freq in hz. res is between 1 and whatever. Watch out!
double ConversionFilter::FilterLowResonant(double input, double cutoff, double resonance)
{
    cutoff_ = cutoff;
    if (IsLessNotEqual(cutoff_, CUT_OFF_MIN)) {
        cutoff_ = CUT_OFF_MIN;
    }
    if (IsGreatNotEqual(cutoff_, static_cast<double>(sampleRate_))) {
        cutoff_ = static_cast<double>(sampleRate_);
    }
    if (IsLessNotEqual(resonance, AMOUNT_RESONANCE_MIN)) {
        resonance = AMOUNT_RESONANCE_MIN;
    }
    if (sampleRate_ == 0) {
        SEN_HILOGE("sampleRate_ should not be 0");
        return 0.0;
    }
    pole_ = cos(M_PI * 2 * cutoff_ / sampleRate_);
    filterCoefficient_ = 2 - 2 * pole_;
    if (IsEqual(pole_, 1.0)) {
        SEN_HILOGE("pole_ should not be 1.0");
        return 0.0;
    }
    double r = (sqrt(2.0) * sqrt(-pow((pole_ - 1.0), 3.0)) + resonance * (pole_ - 1.0)) / (resonance * (pole_ - 1.0));
    speed_ = speed_ + (input - pos_) * filterCoefficient_;
    pos_ = pos_ + speed_;
    speed_ = speed_ * r;
    output_ = pos_;
    return output_;
}

// working hires filter
double ConversionFilter::FilterHighResonant(double input, double cutoff, double resonance)
{
    cutoff_ = cutoff;
    if (IsLessNotEqual(cutoff_, CUT_OFF_MIN)) {
        cutoff_ = CUT_OFF_MIN;
    }
    if (IsGreatNotEqual(cutoff_, static_cast<double>(sampleRate_))) {
        cutoff_ = sampleRate_;
    }
    if (IsLessNotEqual(resonance, AMOUNT_RESONANCE_MIN)) {
        resonance = AMOUNT_RESONANCE_MIN;
    }
    if (sampleRate_ == 0) {
        SEN_HILOGE("sampleRate_ should not be 0");
        return 0.0;
    }
    pole_ = cos(M_PI * 2 * cutoff_ / sampleRate_);
    filterCoefficient_ = 2 - 2 * pole_;
    if (IsEqual(pole_, 1.0)) {
        SEN_HILOGE("pole_ should not be 1.0");
        return 0.0;
    }
    double r = (sqrt(2.0) * sqrt(-pow((pole_ - 1.0), F_THREE)) +
        resonance * (pole_ - 1.0)) / (resonance * (pole_ - 1.0));
    speed_ = speed_ + (input - pos_) * filterCoefficient_;
    pos_ = pos_ + speed_;
    speed_ = speed_ * r;
    output_ = input - pos_;
    return output_;
}

// This works a bit. Needs attention.
double ConversionFilter::FilterBandPass(double input, double cutoff, double resonance)
{
    cutoff_ = cutoff;
    if (IsGreatNotEqual(cutoff_, sampleRate_ * F_HALF)) {
        cutoff_ = (sampleRate_ * F_HALF);
    }
    if (IsGreatNotEqual(resonance, VALIDE_RESONANCE_MAX)) {
        resonance = VALIDE_RESONANCE_MAX;
    }
    if (sampleRate_ == 0) {
        SEN_HILOGE("sampleRate_ should not be 0");
        return 0.0;
    }
    pole_ = cos(M_PI * 2 * cutoff_ / sampleRate_);
    sampledData_[0] = (1 - resonance) * (sqrt(resonance * (resonance - BAND_PASS_COEF * pow(pole_, 2.0) + 2.0) + 1));
    sampledData_[1] = 2 * pole_ * resonance;
    sampledData_[2] = pow(-resonance, 2);

    output_ = sampledData_[0] * input + sampledData_[1] * filteredData_[1] + sampledData_[2] * filteredData_[2];
    filteredData_[2] = filteredData_[1];
    filteredData_[1] = output_;
    return output_;
}

double ConversionFilter::HandleResonant(double input, double cutoff, double resonance, FilterMethod filterMethod)
{
    double output = 0.0;
    switch (filterMethod) {
        case LOW_RESONANT_FILTER: {
            output = FilterLowResonant(input, cutoff, resonance);
            break;
        }
        case HIGH_RESONANT_FILTER: {
            output = FilterHighResonant(input, cutoff, resonance);
            break;
        }
        case BAND_PASS_FILTER: {
            output = FilterBandPass(input, cutoff, resonance);
            break;
        }
        default: {
            SEN_HILOGE("Unsupported filter Method");
            return output;
        }
    }
    return output;
}

double ConversionFilter::HandleHighPassOrLowPass(double input, double cutoff, bool isHighPass)
{
    return isHighPass ? FilterHighPass(input, cutoff) : FilterLowPass(input, cutoff);
}
} // namespace Sensors
} // namespace OHOS