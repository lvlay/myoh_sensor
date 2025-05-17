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

#include "conversion_mfcc.h"

#include "audio_utils.h"
#include "sensor_log.h"
#include "sensors_errors.h"
#include "utils.h"

#undef LOG_TAG
#define LOG_TAG "ConversionMfcc"

namespace OHOS {
namespace Sensors {
namespace {
constexpr double BANDS_MIN_THRESHOLD { 0.000001 };
constexpr uint32_t MEL_FILTERS_OR_COEFFS_MAX { 4096 * 4096 };
}  // namespace

int32_t ConversionMfcc::HandleMelFilterAndLogSquare(const std::vector<float> &powerSpectrum)
{
    size_t powerSpectrumSize = powerSpectrum.size();
    if ((powerSpectrumSize == 0) || (powerSpectrumSize >= numBins_)) {
        SEN_HILOGE("Invalid parameter");
        return Sensors::PARAMETER_ERROR;
    }
    for (uint32_t i = 0; i < numFilters_; ++i) {
        melBands_[i] = 0;
        for (uint32_t bin = 0; bin < numBins_; ++bin) {
            uint32_t idx = i + (bin * numFilters_);
            melBands_[i] += (melFilters_[idx] * powerSpectrum[bin]);
        }
    }
    for (uint32_t i = 0; i < numFilters_; ++i) {
        // log the square
        melBands_[i] = (melBands_[i] > BANDS_MIN_THRESHOLD) ? log(melBands_[i] * melBands_[i]) : 0;
    }
    return Sensors::SUCCESS;
}

int32_t ConversionMfcc::Init(uint32_t numBins, uint32_t numCoeffs, const MfccInputPara &para)
{
    if ((numBins <= 0) || (numCoeffs <= 0) || para.nMels > MEL_FILTERS_OR_COEFFS_MAX ||
        numCoeffs > MEL_FILTERS_OR_COEFFS_MAX) {
        SEN_HILOGE("Invalid parameter, para.nMels:%{public}d, numCoeffs:%{public}d", para.nMels, numCoeffs);
        return Sensors::PARAMETER_ERROR;
    }
    numFilters_ = para.nMels;
    numCoeffs_ = numCoeffs;
    minFreq_ = para.minFreq;
    maxFreq_ = para.maxFreq;
    sampleRate_ = para.sampleRate;
    numBins_ = numBins;
    melBands_.resize(numFilters_, 0.0);
    coeffs_.resize(numCoeffs, 0.0);
    // create new matrix
    dctMatrix_.resize(numCoeffs * numFilters_, 0.0);
    // now generate the coefficients for the mag spectrum
    melFilters_.resize(numFilters_ * numBins_, 0.0);
    if (CalcMelFilterBank(sampleRate_) != Sensors::SUCCESS) {
        SEN_HILOGE("CalcMelFilterBank failed");
        return Sensors::ERROR;
    }
    if (CreateDCTCoeffs() != Sensors::SUCCESS) {
        SEN_HILOGE("CreateDCTCoeffs failed");
        return Sensors::ERROR;
    }
    return Sensors::SUCCESS;
}

std::vector<double> ConversionMfcc::Mfcc(const std::vector<float> &powerSpectrum)
{
    int32_t ret = HandleMelFilterAndLogSquare(powerSpectrum);
    if (ret != Sensors::SUCCESS) {
        SEN_HILOGE("HandleMelFilterAndLogSquare failed");
        return {};
    }
    if (HandleDiscreteCosineTransform() != Sensors::SUCCESS) {
        SEN_HILOGE("HandleDiscreteCosineTransform failed");
        return {};
    }
    return coeffs_;
}

int32_t ConversionMfcc::HandleDiscreteCosineTransform()
{
    if (numCoeffs_ == 0) {
        SEN_HILOGE("numCoeffs_ should not be 0");
        return Sensors::ERROR;
    }
    for (uint32_t i = 0; i < numCoeffs_; ++i) {
        coeffs_[i] = 0;
        for (uint32_t j = 0; j < numFilters_; ++j) {
            uint32_t idx = i + (j * numCoeffs_);
            coeffs_[i] += (dctMatrix_[idx] * melBands_[j]);
        }
        coeffs_[i] /= numCoeffs_;
    }
    return Sensors::SUCCESS;
}

int32_t ConversionMfcc::SetMelFilters(uint32_t idx, double binFreq, double prevFreq, double thisFreq, double nextFreq)
{
    if (nextFreq == 0) {
        SEN_HILOGE("Invalid parameter");
        return Sensors::ERROR;
    }
    if (IsEqual(nextFreq, prevFreq)) {
        SEN_HILOGE("The divisor cannot be 0");
        return Sensors::ERROR;
    }
    melFilters_[idx] = 0;
    double height = 2.0 / (nextFreq - prevFreq);
    if (IsLessOrEqual(binFreq, thisFreq)) {
        if (IsEqual(thisFreq, prevFreq)) {
            SEN_HILOGE("The divisor cannot be 0");
            return Sensors::ERROR;
        }
        melFilters_[idx] = (binFreq - prevFreq) * (height / (thisFreq - prevFreq));
    } else {
        if (IsEqual(nextFreq, thisFreq)) {
            SEN_HILOGE("The divisor cannot be 0");
            return Sensors::ERROR;
        }
        melFilters_[idx] = height + ((binFreq - thisFreq) * (-height / (nextFreq - thisFreq)));
    }
    return Sensors::SUCCESS;
}

int32_t ConversionMfcc::CalcMelFilterBank(double sampleRate)
{
    if (numBins_ == 0) {
        SEN_HILOGE("numBins_ should not be 0");
        return Sensors::PARAMETER_ERROR;
    }
    double nyquist = sampleRate / 2;
    if (IsGreatNotEqual(maxFreq_, nyquist)) {
        maxFreq_ = nyquist;
    }
    double maxMel = OHOS::Sensors::ConvertSlaneyMel(maxFreq_);
    double minMel = OHOS::Sensors::ConvertSlaneyMel(minFreq_);
    double stepMel = (maxMel - minMel) / (numFilters_ + 1);
    std::vector<double> filterHzPos(numFilters_ + 2);
    double nextMel = minMel;
    for (uint32_t i = 0; i < (numFilters_ + 2); ++i) {
        filterHzPos[i] = OHOS::Sensors::ConvertSlaneyHz(nextMel);
        nextMel += stepMel;
    }
    std::vector<double> binFs(numBins_);
    double stepHz = sampleRate / (2 * numBins_);
    for (uint32_t bin = 0; bin < numBins_; ++bin) {
        binFs[bin] = stepHz * bin;
    }
    for (uint32_t i = 0; i < numFilters_; ++i) {
        for (uint32_t j = 0; j < numBins_; ++j) {
            uint32_t idx = i + (j * numFilters_);
            if (SetMelFilters(idx, binFs[j], filterHzPos[i], filterHzPos[i + 1],
                filterHzPos[i + 2]) != Sensors::SUCCESS) {
                SEN_HILOGE("SetMelFilters failed");
                return Sensors::ERROR;
            }
        }
    }
    return Sensors::SUCCESS;
}

std::vector<double> ConversionMfcc::GetMelFilterBank() const
{
    std::vector<double> melFilters;
    uint32_t num = numFilters_ * numBins_;
    for (uint32_t i = 0; i < num; ++i) {
        melFilters.emplace_back(melFilters_[i]);
    }
    return melFilters;
}

int32_t ConversionMfcc::CreateDCTCoeffs()
{
    if (numFilters_ == 0) {
        SEN_HILOGE("numFilters_ should not be 0");
        return Sensors::ERROR;
    }
    double k = M_PI / numFilters_;
    double w1 = 1.0 / (sqrt(numFilters_));
    double w2 = sqrt(2.0 / numFilters_);
    // generate dct matrix
    for (uint32_t i = 0; i < numCoeffs_; ++i) {
        for (uint32_t j = 0; j < numFilters_; ++j) {
            uint32_t idx = i + (j * numCoeffs_);
            if (i == 0) {
                dctMatrix_[idx] = w1 * cos(k * (i + 1) * (j + F_HALF));
            } else {
                dctMatrix_[idx] = w2 * cos(k * (i + 1) * (j + F_HALF));
            }
        }
    }
    return Sensors::SUCCESS;
}

int32_t ConversionMfcc::FiltersMel(int32_t nFft, MfccInputPara para,
    size_t &frmCount, std::vector<double> &melBasis)
{
    if (nFft == 0) {
        SEN_HILOGE("nFft should not be 0");
        return Sensors::PARAMETER_ERROR;
    }
    int32_t sr = para.sampleRate;
    double fMax = para.maxFreq;
    if (IsLessNotEqual(fMax, EPS_MIN)) {
        fMax = sr / 2.0;
    }
    size_t nMels = static_cast<size_t>(para.nMels);
    frmCount = nFft / 2;
    std::vector<double> basis(nMels * frmCount, 0.0);
    // generate mel frequencies.
    double fMin = para.minFreq;
    double minMel = OHOS::Sensors::ConvertHtkMel(fMin);
    double maxMel = OHOS::Sensors::ConvertHtkMel(fMax);
    std::vector<double> filterHzPos(nMels + 2);
    double stepMel = (maxMel - minMel) / (nMels + 1);
    double stepHz = static_cast<double>(sr) / nFft;
    double nextMel = minMel;
    for (size_t i = 0; i < (nMels + 2); ++i) {
        filterHzPos[i] = OHOS::Sensors::ConvertHtkHz(nextMel);
        nextMel += stepMel;
    }
    std::vector<double> binFs(frmCount);
    for (int32_t j = 0; j < frmCount; ++j) {
        binFs[j] = stepHz * j;
    }
    int32_t index = 0;
    for (size_t i = 2; i < filterHzPos.size(); ++i) {
        double prevFreq = filterHzPos[i - 2];
        double thisFreq = filterHzPos[i - 1];
        double nextFreq = filterHzPos[i];
        if (IsEqual(thisFreq, prevFreq) || IsEqual(nextFreq, thisFreq) || IsEqual(nextFreq, prevFreq)) {
            SEN_HILOGE("The divisor cannot be 0");
            return Sensors::ERROR;
        }
        for (int32_t j = 0; j < frmCount; ++j) {
            double binFreq = binFs[j];
            double lower = (binFreq - prevFreq) / (thisFreq - prevFreq);
            double upper = (nextFreq - binFreq) / (nextFreq - thisFreq);
            double min = IsLessNotEqual(lower, upper) ? lower : upper;
            min = IsGreatNotEqual(min, 0.0) ? min : 0.0;
            basis[index++] = min * 2.0 / (nextFreq - prevFreq);
        }
    }
    melBasis = OHOS::Sensors::TransposeMatrix(nMels, basis);
    return melBasis.empty() ? Sensors::ERROR : Sensors::SUCCESS;
}
}  // namespace Sensors
}  // namespace OHOS