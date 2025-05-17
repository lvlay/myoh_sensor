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

#ifndef CONVERSION_MFCC_H
#define CONVERSION_MFCC_H

#include <vector>

namespace OHOS {
namespace Sensors {
/**
 * @brief Calculate the parameters required for MFCC
 */
struct MfccInputPara {
    /** sampling rate of audio file. */
    int32_t sampleRate { 0 };
    /** Number of MEL filters. */
    int32_t nMels { 0 };
    /** The effective minimum frequency of audio. lowest frequency (in Hz). */
    double minFreq { 0.0 };
    /** Effective maximum frequency of audio.
     * highest frequency (in Hz). If `zero`, use 'fmax = sr / 2.0' */
    double maxFreq { 0.0 };
};

/**
 * @brief Mel-frequency cepstral coefficients (MFCCs).the MFCC calculation will depend on the peak
 *  loudness (in decibels).
 *
 */
class ConversionMfcc {
public:
    ConversionMfcc() = default;
    ~ConversionMfcc() = default;

    /**
     * @brief Init mfcc module
     *
     * @param numBins The value equal hopLength for sfft.
     * @param numCoeffs 12 semitones, the value is 12+1.
     * @param para Calculate the parameters required for MFCC.
     *
     * @return Returns <b>0</b> if the operation is successful; returns a negative value otherwise.
     */
    int32_t Init(uint32_t numBins, uint32_t numCoeffs, const MfccInputPara &para);

    /**
     * @brief Calculate MFCC based on power spectrum.
     *
     * @param powerSpectrum Log-power Mel spectrogram
     * @return Return MFCC.
     */
    std::vector<double> Mfcc(const std::vector<float> &powerSpectrum);

    /**
     * @brief Get the Mel Filter Bank
     *  1. Call the setup function at first.
     *  2. use Slaney formula instead of HTK
     *
     * @return Slaney Bias
     */
    std::vector<double> GetMelFilterBank() const;

    /**
     * @brief Create a Mel filter-bank.
     *  1. Call the setup function at first.
     *  2. use HTK formula instead of Slaney
     *
     * @param nFft number of FFT components
     * @param para Calculate the parameters required for MFCC.
     * @param frmCount Return frame counts.
     * @param melBasis [shape=(n_mels, 1 + nFft/2)] Mel transform matrix
     *
     * @return Returns <b>0</b> if the operation is successful; returns a negative value otherwise.
     */
    int32_t FiltersMel(int32_t nFft, MfccInputPara para, size_t &frmCount, std::vector<double> &melBasis);

private:
    int32_t HandleDiscreteCosineTransform();
    int32_t HandleMelFilterAndLogSquare(const std::vector<float> &powerSpectrum);
    int32_t CalcMelFilterBank(double sampleRate);
    int32_t CreateDCTCoeffs();
    int32_t SetMelFilters(uint32_t idx, double binFreq, double prevFreq, double thisFreq, double nextFreq);

private:
    std::vector<double> melBands_;
    uint32_t numFilters_ { 0 };
    uint32_t numCoeffs_ { 0 };
    double minFreq_ { 0.0 };
    double maxFreq_ { 0.0 };
    uint32_t sampleRate_ { 0 };
    std::vector<double> melFilters_;
    uint32_t numBins_ { 0 };
    std::vector<double> dctMatrix_;
    std::vector<double> coeffs_;
};
} // namespace Sensors
} // namespace OHOS
#endif // CONVERSION_MFCC_H