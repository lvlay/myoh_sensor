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

#include "conversion_fft.h"

#include "sensor_log.h"
#include "sensors_errors.h"
#include "utils.h"

#undef LOG_TAG
#define LOG_TAG "ConversionFFT"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t SPECTRUM_COUNT_MAX { 8192 };
constexpr int32_t MAX_FFT_SIZE { 10240 };
}  // namespace

int32_t ConversionFFT::Init(const FFTInputPara &fftPara)
{
    if ((fftPara.sampleRate <= 0) ||
        (fftPara.fftSize <= 0 || !(IsPowerOfTwo(static_cast<uint32_t>(fftPara.fftSize)))) ||
        (fftPara.hopSize <= 0 || !(IsPowerOfTwo(static_cast<uint32_t>(fftPara.hopSize))))) {
        SEN_HILOGE("sampleRate:%{public}d, fftSize:%{public}d, hopSize:%{public}d", fftPara.sampleRate,
            fftPara.fftSize, fftPara.hopSize);
        return Sensors::PARAMETER_ERROR;
    }
    para_ = fftPara;
    fft_.Init(para_.fftSize);
    para_.windowSize = (fftPara.windowSize > para_.fftSize) ? fftPara.windowSize : para_.fftSize;
    pos_ = para_.windowSize - para_.hopSize;
    if (pos_ < 0) {
        pos_ = 0;
    }
    isFrameFull_ = false;
    isFftCalcFinish_ = false;
    bins_ = para_.fftSize / 2;
    fftResult_.buffer.resize(para_.fftSize, 0.0F);
    fftResult_.window.resize(para_.fftSize, 0.0F);
    fftResult_.magnitudes.resize(bins_, 0.0F);
    fftResult_.magnitudesDB.resize(bins_, 0.0F);
    fftResult_.phases.resize(bins_, 0.0F);
    fft_.GenWindow(WND_TYPE_HANNING, para_.windowSize, fftResult_.window);
    return Sensors::SUCCESS;
}

bool ConversionFFT::ProcessSingle(float value, FFTModes mode)
{
    int32_t bufferSize = static_cast<int32_t>(fftResult_.buffer.size());
    // add value to buffer at current pos
    if (pos_ < bufferSize) {
        fftResult_.buffer[pos_++] = value;
    }
    // if buffer full, run fft
    isFrameFull_ = (pos_ == para_.windowSize);
    if (!isFrameFull_) {
        return isFrameFull_;
    }
    if (mode == ConversionFFT::WITH_POLAR_CONVERSION) {
        fft_.CalculatePowerSpectrum(fftResult_.buffer, fftResult_.window, fftResult_.magnitudes, fftResult_.phases);
    } else {
        fft_.CalcFFT(fftResult_.buffer, fftResult_.window);
    }
    // reset pos to start of hop
    pos_ = para_.windowSize - para_.hopSize;
    if ((pos_ + para_.hopSize) >= bufferSize) {
            pos_ = bufferSize - para_.hopSize;
            SEN_HILOGW("modify pos data, pos:%{public}d", pos_);
        }
    /** shift buffer back by one hop size. */
    for (int32_t i = 0; i < pos_; i++) {
        fftResult_.buffer[i] = fftResult_.buffer[i + para_.hopSize];
    }
    isFftCalcFinish_ = true;
    return isFrameFull_;
}

int32_t ConversionFFT::Process(const std::vector<double> &values, int32_t &frameCount,
    std::vector<float> &frameMagsArr)
{
    FFTModes mode = ConversionFFT::WITH_POLAR_CONVERSION;
    pos_ = para_.windowSize - para_.hopSize;
    frameCount = 0;
    bool isFrameFull = false;
    size_t valuesSize = values.size();
    for (size_t j = 0; j < valuesSize; j++) {
        // add value to buffer at current pos
        if (pos_ < static_cast<int32_t>(fftResult_.buffer.size())) {
            fftResult_.buffer[pos_++] = static_cast<float>(values[j]);
        }
        // if buffer full, run fft
        isFrameFull = (pos_ == para_.windowSize);
        if (!isFrameFull) {
            continue;
        }
        if (mode == ConversionFFT::WITH_POLAR_CONVERSION) {
            fft_.CalculatePowerSpectrum(fftResult_.buffer, fftResult_.window,
                fftResult_.magnitudes, fftResult_.phases);
        } else {
            fft_.CalcFFT(fftResult_.buffer, fftResult_.window);
        }
        // reset pos to start of hop
        pos_ = para_.windowSize - para_.hopSize;
        /** shift buffer back by one hop size. */
        for (int32_t i = 0; i < pos_; i++) {
            fftResult_.buffer[i] = fftResult_.buffer[i + para_.hopSize];
        }
        isFftCalcFinish_ = true;
        frameMagsArr.insert(frameMagsArr.end(), fftResult_.magnitudes.begin(), fftResult_.magnitudes.end());
        ++frameCount;
    }
    return Sensors::SUCCESS;
}

std::vector<float> &ConversionFFT::ConvertDB()
{
    if (isFftCalcFinish_) {
        fft_.ConvertDB(fftResult_.magnitudes, fftResult_.magnitudesDB);
        isFftCalcFinish_ = false;
    }
    return fftResult_.magnitudesDB;
}

float ConversionFFT::GetSpectralFlatness() const
{
    if (bins_ == 0) {
        SEN_HILOGE("bins_ should not be 0");
        return 0.0F;
    }
    float geometricMean = 0.0F;
    float arithmaticMean = 0.0F;
    for (int32_t i = 0; i < bins_; i++) {
        if (fftResult_.magnitudes[i] != 0) {
            geometricMean += logf(fftResult_.magnitudes[i]);
        }
        arithmaticMean += fftResult_.magnitudes[i];
    }
    geometricMean = expf(geometricMean / bins_);
    arithmaticMean /= bins_;
    bool isEqual = IsEqual(arithmaticMean, 0.0F);
    return isEqual ? 0.0F : (geometricMean / arithmaticMean);
}

float ConversionFFT::GetSpectralCentroid() const
{
    float x = 0.0F;
    float y = 0.0F;
    for (int32_t i = 0; i < bins_; i++) {
        x += fabs(fftResult_.magnitudes[i]) * i;
        y += fabs(fftResult_.magnitudes[i]);
    }
    if (IsEqual(y, 0.0F) || (para_.fftSize == 0)) {
        return 0.0F;
    }
    return ((x / y) * (static_cast<float>(para_.sampleRate) / para_.fftSize));
}

// INVERSE FFT
int32_t ConversionIFFT::Init(int32_t fftSize, int32_t hopSize, int32_t windowSize)
{
    if (fftSize > MAX_FFT_SIZE) {
        SEN_HILOGE("fftSize is greater than MAX_FFT_SIZE,fftSize:%{public}d", fftSize);
        return Sensors::ERROR;
    }
    fft_.Init(fftSize);
    para_.fftSize = fftSize;
    para_.windowSize = (windowSize > 0) ? windowSize : para_.fftSize;
    bins_ = para_.fftSize / 2;
    para_.hopSize = hopSize;
    fftResult_.buffer.resize(para_.fftSize, 0.0F);
    ifftOut_.resize(para_.fftSize, 0.0F);
    pos_ = 0;
    fftResult_.window.resize(para_.fftSize, 0.0F);
    fft_.GenWindow(WND_TYPE_HANNING, para_.windowSize, fftResult_.window);
    return Sensors::SUCCESS;
}

float ConversionIFFT::Process(const std::vector<float> &mags, const std::vector<float> &phases, const FFTModes mode)
{
    if (pos_ == 0) {
        ifftOut_.clear();
        if (mode == ConversionIFFT::SPECTRUM) {
            fft_.InversePowerSpectrum(fftResult_.window, mags, phases, ifftOut_);
        } else {
            fft_.InverseFFTComplex(fftResult_.window, mags, phases, ifftOut_);
        }
        // add to output
        // shift back by one hop
        for (int32_t i = 0; i < (para_.fftSize - para_.hopSize); i++) {
            fftResult_.buffer[i] = fftResult_.buffer[i + para_.hopSize];
        }
        // clear the end chunk
        for (int32_t i = 0; i < para_.hopSize; i++) {
            fftResult_.buffer[i + para_.fftSize - para_.hopSize] = 0.0F;
        }
        // merge new output
        for (int32_t i = 0; i < para_.fftSize; i++) {
            fftResult_.buffer[i] += ifftOut_[i];
        }
    }
    nextValue_ = fftResult_.buffer[pos_];
    // limit the values, this alg seems to spike occasionally (and break the audio drivers)
    if (para_.hopSize == ++pos_) {
        pos_ = 0;
    }
    return nextValue_;
}

int32_t ConversionOctave::Init(float samplingRate, int32_t nBandsInTheFFT, int32_t nAveragesPerOctave)
{
    samplingRate_ = samplingRate;
    nSpectrum_ = nBandsInTheFFT;
    if ((nSpectrum_ <= 0) || (nSpectrum_ > SPECTRUM_COUNT_MAX)) {
        SEN_HILOGE("The nSpectrum_ value is invalid");
        return Sensors::PARAMETER_ERROR;
    }
    spectrumFrequencySpan_ = (samplingRate / 2.0) / nSpectrum_;
    nAverages_ = nBandsInTheFFT;
    // fe:  2f for octave bands, sqrt(2) for half-octave bands, cuberoot(2) for third-octave bands, etc
    // um, wtf
    if (nAveragesPerOctave == 0) {
        nAveragesPerOctave = 1;
    }
    averageFrequencyIncrement_ = pow(2.0, 1.0F / nAveragesPerOctave);
    // this isn't currently configurable (used once here then no effect), but here's some reasoning
    firstOctaveFrequency_ = 55.0F;
    // for each spectrum[] bin, calculate the mapping into the appropriate average[] bin.
    auto spe2avg = new (std::nothrow) int32_t[nSpectrum_];
    CHKPR(spe2avg, Sensors::ERROR);
    std::shared_ptr<int32_t> spe2avgShared(spe2avg, std::default_delete<int32_t[]>());
    spe2avg_ = std::move(spe2avgShared);
    int32_t avgIdx = 0;
    float averageFreq = firstOctaveFrequency_; // the "top" of the first averaging bin
    // we're looking for the "top" of the first spectrum bin, and i'm just sort of
    // guessing that this is where it is (or possibly spectrumFrequencySpan/2?)
    // ... either way it's probably close enough for these purposes
    float spectrumFreq = spectrumFrequencySpan_;
    for (int32_t speIdx = 0; speIdx < nSpectrum_; speIdx++) {
        while (spectrumFreq > averageFreq) {
            ++avgIdx;
            averageFreq *= averageFrequencyIncrement_;
        }
        *(spe2avg_.get() + speIdx) = avgIdx;
        spectrumFreq += spectrumFrequencySpan_;
    }
    nAverages_ = avgIdx;
    auto averages = new (std::nothrow) float[nAverages_];
    CHKPR(averages, Sensors::ERROR);
    std::shared_ptr<float> averagesShared(averages, std::default_delete<float[]>());
    averages_ = std::move(averagesShared);
    auto peaks = new (std::nothrow) float[nAverages_];
    CHKPR(peaks, Sensors::ERROR);
    std::shared_ptr<float> peaksShared(peaks, std::default_delete<float[]>());
    peaks_ = std::move(peaksShared);
    auto peakHoldTimes = new (std::nothrow) int32_t[nAverages_];
    CHKPR(peakHoldTimes, Sensors::ERROR);
    std::shared_ptr<int32_t> peakHoldTimesShared(peakHoldTimes, std::default_delete<int32_t[]>());
    peakHoldTimes_ = std::move(peakHoldTimesShared);

    peakHoldTime_ = 0;
    peakDecayRate_ = 0.9F;
    linearEQIntercept_ = 1.0F;
    linearEQSlope_ = 0.0F;
    return Sensors::SUCCESS;
}

int32_t ConversionOctave::Calculate(const std::vector<float> &fftData)
{
    if (fftData.empty()) {
        SEN_HILOGE("fftData is empty");
        return Sensors::PARAMETER_ERROR;
    }
    CHKPR(spe2avg_, Sensors::PARAMETER_ERROR);
    CHKPR(averages_, Sensors::PARAMETER_ERROR);
    CHKPR(peaks_, Sensors::PARAMETER_ERROR);
    CHKPR(peakHoldTimes_, Sensors::PARAMETER_ERROR);
    int32_t lastAvgIdx = 0; // tracks when we've crossed into a new averaging bin, so store current average
    float sum = 0.0F;        // running total of spectrum data
    int32_t count = 0;       // count of spectrums accumulated (for averaging)
    for (int32_t speIdx = 0; speIdx < nSpectrum_; speIdx++) {
        ++count;
        sum += (fftData[speIdx] * (linearEQIntercept_ + speIdx * linearEQSlope_));
        int32_t avgIdx = *(spe2avg_.get() + speIdx);
        if (avgIdx != lastAvgIdx) {
            for (int32_t j = lastAvgIdx; j < avgIdx; j++) {
                *(averages_.get() + j) = sum / static_cast<float>(count);
            }
            count = 0;
            sum = 0.0F;
        }
        lastAvgIdx = avgIdx;
    }
    // the last average was probably not calculated...
    if ((count > 0) && (lastAvgIdx < nAverages_)) {
        *(averages_.get() + lastAvgIdx) = sum / count;
    }
    // update the peaks separately
    for (int32_t i = 0; i < nAverages_; i++) {
        if (IsGreatOrEqual(*(averages_.get() + i), *(peaks_.get() + i))) {
            // save new peak level, also reset the hold timer
            *(peaks_.get() + i) = *(averages_.get() + i);
            *(peakHoldTimes_.get() + i) = peakHoldTime_;
        } else {
            // current average does not exceed peak, so hold or decay the peak
            if (*(peakHoldTimes_.get() + i) > 0) {
                --*(peakHoldTimes_.get() + i);
            } else {
                *(peaks_.get() + i) *= peakDecayRate_;
            }
        }
    }
    return Sensors::SUCCESS;
}
}  // namespace Sensors
}  // namespace OHOS