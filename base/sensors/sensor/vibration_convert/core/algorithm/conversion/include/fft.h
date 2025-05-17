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

#ifndef FFT_H
#define FFT_H

#include "utils.h"

namespace OHOS {
namespace Sensors {
/**
 * @brief The parameters and calculation results used in FFT.
 *   Used to encapsulate 6 parameters in the AlgFFT function.
 */
struct FftParaAndResult {
    /** Number of samples */
    int32_t numSamples { 0 };
    /** The real part of the input */
    std::vector<float> realIn;
    /** The imaginary part of the input */
    std::vector<float> imagIn;
    /** The real part of the output */
    std::vector<float> realOut;
    /** The imaginary part of the output */
    std::vector<float> imagOut;

    FftParaAndResult() = default;
    explicit FftParaAndResult(int32_t size)
    {
        numSamples = size;
        realIn.resize(size, 0.0F);
        imagIn.resize(size, 0.0F);
        realOut.resize(size, 0.0F);
        imagOut.resize(size, 0.0F);
    }
};

class Fft {
public:
    Fft() = default;
    ~Fft();

    void Init(int32_t fftSize);

    std::vector<float> GetReal() const;
    std::vector<float> GetImg() const;

    /* Calculate the power spectrum */
    void CalcFFT(const std::vector<float> &data, const std::vector<float> &window);
    void ConvertPolar(std::vector<float> &magnitude, std::vector<float> &phase);
    void CalculatePowerSpectrum(const std::vector<float> &data, const std::vector<float> &window,
        std::vector<float> &magnitude, std::vector<float> &phase);
    /** the inverse */
    void ConvertCart(const std::vector<float> &magnitude, const std::vector<float> &phase);
    void CalcIFFT(const std::vector<float> &window, std::vector<float> &finalOut);
    void InverseFFTComplex(const std::vector<float> &window, const std::vector<float> &real,
        const std::vector<float> &imaginary, std::vector<float> &finalOut);
    void InversePowerSpectrum(const std::vector<float> &window, const std::vector<float> &magnitude,
        const std::vector<float> &phase, std::vector<float> &finalOut);
    void ConvertDB(const std::vector<float> &in, std::vector<float> &out);
    int32_t GenWindow(int32_t whichFunction, int32_t numSamples, std::vector<float> &window);

private:
    int32_t WindowFunc(int32_t whichFunction, int32_t numSamples, float *out);
    uint32_t FastReverseBits(uint32_t pos, uint32_t numBits);

    /**
     * @brief Power Spectrum
     *
     * 1. This function computes the same as AlgRealFFT, above, but adds the squares of the real and imaginary part
     *  of each coefficient, extracting the power and throwing away the phase.
     * 2. For speed, it does not call AlgRealFFT, but duplicates some of its code.
     *
     * @param numSamples Number of samples.
     * @param in The part of the input.
     * @param out The part of the output
     *
     * @return Returns <b>0</b> if the operation is successful; returns a negative value otherwise.
     */
    int32_t AlgPowerSpectrum(int32_t numSamples, const std::vector<float> &in, std::vector<float> &out);

    /**
     * @brief Real Fast Fourier Transform
     *
     * 1. This function was based on the code in Numerical Recipes in C.
     * 2.In Num. Rec., the inner loop is based on a single 1-based array of interleaved real and imaginary numbers.
     *   Because we have two separate zero-based arrays, our indices are quite different.
     * 3. Here is the correspondence between Num. Rec.
     *
     * @return Returns <b>0</b> if the operation is successful; returns a negative value otherwise.
     */
    int32_t AlgRealFFT(FftParaAndResult &paraRes);
    int32_t AlgFFT(bool inverseTransform, FftParaAndResult &paraRes);
    int32_t AlgInitFFT();

private:
    /** fftSize */
    int32_t fftSize_ { 0 };
    /** halfFFTSize */
    int32_t half_ { 0 };

    FftParaAndResult fftParaRes_;
    /**
     * 'para_' needs to be created every time the sfft occurs, so it is created once during initialization.
     * It's half the size of fftParaRes_, only used in one function.
     */
    FftParaAndResult para_;
    uint32_t** fftBitTable_ { nullptr };
};
} // namespace Sensors
} // namespace OHOS
#endif // FFT_H