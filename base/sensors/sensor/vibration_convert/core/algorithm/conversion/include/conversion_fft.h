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

#ifndef CONVERSION_FFT_H
#define CONVERSION_FFT_H

#include "fft.h"

namespace OHOS {
namespace Sensors {
/**
 * @brief The parameters required for FFT
 */
struct FFTInputPara {
    /** sampling rate of audio file. */
    int32_t sampleRate { 0 };
    /** the FFT size. This must be a power of 2. */
    int32_t fftSize { 0 };
    /** the hop size. */
    int32_t hopSize { 0 };
    /** the window size. */
    int32_t windowSize { 0 };
};

struct FFTOutputResult {
    std::vector<float> magnitudes;
    std::vector<float> phases;
    std::vector<float> magnitudesDB;
    std::vector<float> buffer;
    std::vector<float> window;
};

/**
 * Fast fourier transform. For spectral audio process and machine listening.
 */
class ConversionFFT {
public:
    /**
     * @brief How to run the FFT - with or without polar conversion
     */
    enum FFTModes {
        NO_POLAR_CONVERSION = 0,
        WITH_POLAR_CONVERSION = 1
    };

    ConversionFFT() = default;
    ~ConversionFFT() = default;

    /**
     * @brief the FFT.
     *
     * @param fftSize the FFT size. This must be a power of 2
     * @param hopSize the hop size
     * @param windowSize the window size
     *
     * @return Returns <b>0</b> if the operation is successful; returns a negative value otherwise.
     */
    int32_t Init(const FFTInputPara &fftPara);

    /**
     * @brief The sampled data is processed frame by frame
     * @param values A set of signal data
     * @return Return 0 when the analysis has run (every [hopsize] samples)
     */
    int32_t Process(const std::vector<double> &values, int32_t &frameCount, std::vector<float> &frameMagsArr);

    /**
     * @brief Process the sampled data one by one
     * @param value A sampling value.
     * @param mode see FFTModes enumeration
     * @return Return true when the analysis has run (every [hopsize] samples)
     */
    bool ProcessSingle(float value, FFTModes mode = ConversionFFT::WITH_POLAR_CONVERSION);

    /**
     * @brief Get the complex real object
     *
     * @return Return a pointer to an array of values containing the real components of the fft analysis.
     */
    std::vector<float> GetReal()
    {
        return fft_.GetReal();
    };

    /**
     * @brief Get the complex imaginary part
     *
     * @return Return a pointer to an array of values containing the imaginary components of the fft analysis
     */
    std::vector<float> GetImag()
    {
        return fft_.GetImg();
    };

    /**
     * @brief Get the magnitudes object
     *
     * @return a vector of magnitudes (assuming the FFT was calcuated with polar conversion)
     */
    std::vector<float> GetMagnitudes()
    {
        return fftResult_.magnitudes;
    }

    /**
     * @brief Get the magnitudes DB object
     *
     * @return Return a vector of magnitudes in decibels (assuming the FFT was calcuated with polar conversion).
     */
    std::vector<float> GetMagnitudesDB()
    {
        return ConvertDB();
    }

    /**
     * @brief Get the phases object
     *
     * @return Return a vector of phases (assuming the FFT was calcuated with polar conversion)
     */
    std::vector<float> GetPhases()
    {
        return fftResult_.phases;
    }

    /**
     * @brief Get the num bins object
     *
     * @return Return the number of bins in the FFT analysis
     */
    int32_t GetNumBins() const
    {
        return bins_;
    }

    /**
     * @brief Get the FFT size
     *
     * @return Return the FFT size
     */
    int32_t GetFFTSize()
    {
        return para_.fftSize;
    }

    /**
     * @brief Get the hop size
     *
     * @return Return the hop size
     */
    int32_t GetHopSize()
    {
        return para_.hopSize;
    }

    /**
     * @brief Get the window size
     *
     * @return Return the window size
     */
    int32_t GetWindowSize()
    {
        return para_.windowSize;
    }

    /**
     * @brief Calculate the spectral flatness of the most recent FFT
     *
     * @return Return the spectral flatness of the most recent FFT calculation
     */
    float GetSpectralFlatness() const;

    /**
     * @brief Calculate the spectral centroid of the most recent FFT.
     *
     * @return Return the spectral centroid of the most recent FFT calculation
     */
    float GetSpectralCentroid() const;

private:
    std::vector<float> &ConvertDB();

private:
    FFTInputPara para_;
    FFTOutputResult fftResult_;
    int32_t pos_ { 0 };
    Fft fft_;
    bool isFrameFull_ { false };
    int32_t bins_ { 0 };
    bool isFftCalcFinish_ { false };
};

/**
 * @brief Inverse FFT transform
 */
class ConversionIFFT {
public:
    /**
     * @brief Configure what kind of data is being given to the inverse FFT
     *
     */
    enum FFTModes {
         /** Magnitudes and phases */
        SPECTRUM = 0,
        /** Real and imaginary components */
        COMPLEX = 1
    };

    ConversionIFFT() = default;
    ~ConversionIFFT() = default;

    /**
     * @brief the inverse FFT.
     *
     * @param fftSize the FFT size. This must be a power of 2
     * @param hopSize the hop size
     * @param windowSize the window size
     * @return Returns <b>0</b> if the operation is successful; returns a negative value otherwise.
     */
    int32_t Init(int32_t fftSize, int32_t hopSize, int32_t windowSize);

    /**
     * @brief the inverse transform. Call this function at audio rate, but update the FFT information at FFT rate.
     *
     * @param data1 either magnitudes or real values
     * @param data2 either phases or imaginary values
     * @param mode see FFTModes
     * @return Return the most recent sample an audio signal, creates from the inverse transform of the FFT data
     */
    float Process(const std::vector<float> &mags, const std::vector<float> &phases,
	    const FFTModes mode = ConversionIFFT::SPECTRUM);

    /**
     * @brief Get the num bins
     *
     * @return Return the number of fft bins
     */
    int32_t GetNumBins() const
    {
        return bins_;
    }

private:
    FFTInputPara para_;
    /** output buffer and window. */
    FFTOutputResult fftResult_;
    std::vector<float> ifftOut_;
    int32_t bins_ { 0 };
    int32_t pos_ { 0 };
    float nextValue_ { 0.0F };
    Fft fft_;
};

/**
 * @brief octave analyser. It takes FFT magnitudes and remaps them into pitches
 */
class ConversionOctave {
public:
    ConversionOctave() = default;
    ~ConversionOctave() = default;

    /**
     * @brief Initializes the octave note analyzer
     *
     * @param samplingRate the sample rate
     * @param nBandsInTheFFT the number of bins in the FFT
     * @param nAveragesPerOctave how many frequency bands to split each octave into
     *
     * @return Returns <b>0</b> if the operation is successful; returns a negative value otherwise.
     */
    int32_t Init(float samplingRate, int32_t nBandsInTheFFT, int32_t nAveragesPerOctave);

    /**
     * @brief Calculate the octave note related parameters
     *
     * @param fftData a pointer to an array of fft magnitudes
     *
     * @return Returns <b>0</b> if the operation is successful; returns a negative value otherwise.
     */
    int32_t Calculate(const std::vector<float> &fftData);

private:
    float samplingRate_ { 0.0F }; // sampling rate in Hz (needed to calculate frequency spans)
    int32_t nSpectrum_ { 0 };  // number of spectrum bins in the fft
    /** the number of averages, after analysis */
    int32_t nAverages_ { 0 };               // number of averaging bins here
    float spectrumFrequencySpan_ { 0.0F };     // the "width" of an fft spectrum bin in Hz
    float firstOctaveFrequency_ { 0.0F };      // the "top" of the first averaging bin here in Hz
    float averageFrequencyIncrement_ { 0.0F }; // the root-of-two multiplier between averaging bin frequencies
    /** An array of averages - the energy across the pitch spectrum */
    std::shared_ptr<float> averages_ { nullptr };        // the actual averages
    std::shared_ptr<float> peaks_ { nullptr };           // peaks of the averages, aka "maxAverages" in other implementations
    std::shared_ptr<int32_t> peakHoldTimes_ { nullptr }; // how long to hold THIS peak meter?  decay if == 0
    int32_t peakHoldTime_ { 0 };   // how long do we hold peaks? (in fft frames)
    float peakDecayRate_ { 0.0F };    // how quickly the peaks decay:  0f=instantly .. 1f=not at all
    std::shared_ptr<int32_t> spe2avg_ { nullptr };       // the mapping between spectrum[] indices and averages[] indices
    // the fft's log equalizer() is no longer of any use (it would be nonsense to log scale
    // the spectrum values into log-sized average bins) so here's a quick-and-dirty linear
    // equalizer instead:
    float linearEQSlope_ { 0.0F };     // the rate of linear eq
    float linearEQIntercept_ { 0.0F }; // the base linear scaling used at the first averaging bin
    // the formula is:  spectrum[i] * (linearEQIntercept + i * linearEQSlope)
    // so.. note that clever use of it can also provide a "gain" control of sorts
    // (fe: set intercept to 2f and slope to 0f to double gain)
};
} // namespace Sensors
} // namespace OHOS
#endif // CONVERSION_FFT_H
