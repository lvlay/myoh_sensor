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

#ifndef CONVERSION_UTILS_H
#define CONVERSION_UTILS_H

#include <cfloat>
#include <cmath>
#include <functional>
#include <numeric>
#include <vector>

namespace OHOS {
namespace Sensors {
namespace {
constexpr double MIN_LOG_HZ = 1000.0;
constexpr double MIN_LOG_MEL = 15.0;
constexpr double FSP = 66.6666666667;
constexpr double MIN_F = 0.0;
constexpr double LOG_STEP = 0.06875177742094912;
constexpr double MAXFRE = 8000.0;
constexpr double HZ_COEF = 700.0;
constexpr double MEL_COEF = 2595.0;
constexpr int32_t LOG_BASE = 10;
constexpr double EPS_MIN = 1e-10;
constexpr int32_t NFFT = 2048;
constexpr int32_t ONSET_HOP_LEN = 512;
constexpr int32_t ENERGY_HOP_LEN = 1024;
constexpr int32_t WINDOW_LENGTH = ENERGY_HOP_LEN / 2;
constexpr int32_t MAGS_SIZE = NFFT / 2;
constexpr int32_t SAMPLE_RATE = 22050;
constexpr int32_t FRM_LEN = 2048;
constexpr double F_HALF = 0.5;
constexpr double F_THREE = 3.0;
constexpr double SAMPLE_IN_MS = 1000.0;
constexpr double INTERSITY_BOUNDARY_POINT = 0.25;
constexpr double INTERSITY_NUMBER_BOUNDARY_POINT  = 0.75;
}  // namespace

enum WindowType {
    WND_TYPE_BARTLETT = 1,
    WND_TYPE_HAMMING = 2,
    WND_TYPE_HANNING = 3,
};

enum FilterMethod{
    LOW_RESONANT_FILTER = 1,
    HIGH_RESONANT_FILTER = 2,
    BAND_PASS_FILTER = 3,
};

bool IsPowerOfTwo(uint32_t x);
uint32_t ObtainNumberOfBits(uint32_t powerOfTwo);
uint32_t ReverseBits(uint32_t index, uint32_t numBits);

/**
 * @brief Matrix transpose
 *
 * @param rows Number of rows
 * @param values Matrix
 *
 * @return Returns the transposed matrix.
 */
std::vector<double> TransposeMatrix(size_t rows, const std::vector<double> &values);

/**
 * @brief Make ID unique
 *  idx and time appear in pairs
 *
 * @param idx Original series indexs.
 * @param time Original series times.
 * @param newIdx Processed indexs.
 * @param newTime Processed times.
 *
 * @return Returns <b>0</b> if the operation is successful; returns a negative value otherwise.
 */
int32_t UniqueIdx(const std::vector<int32_t> &idx, const std::vector<double> &time, std::vector<int32_t> &newIdx,
    std::vector<double> &newTime);

/**
 * @brief Find the minimum and maximum in Orignal 'values', convert the values into numbers ranging from 0 to 100.
 *
 * @param values Original values
 *
 * @return Return the numbers ranging from 0 to 100.
 */
std::vector<int32_t> NormalizePercentage(const std::vector<double> &values);

/**
 * @brief The minimum is 0, convert the values into numbers ranging from 0 to 100.
 *
 * @param values Original values
 *
 * @return Return the numbers ranging from 0 to 100.
 */
std::vector<int32_t> NormalizePercentageMin(const std::vector<double> &values);

/**
 * @brief Based on the given minimum and maximum range, convert the values into numbers ranging from 0 to 100.
 *
 * @param values Original values
 * @param minValue The minimum of range.
 * @param maxValue The maximum of range.
 *
 * @return Return the numbers ranging from 0 to 100.
 */
std::vector<int32_t> NormalizePercentageRange(const std::vector<double> &values, double minValue, double maxValue);

/**
 * @brief Processing of effective values for filling in mute positions.
 *
 * @param values A set of volume(DB or HZ) values.
 * @param voiceFlag A set of voice and mute flags.
 * @param volumeData A set of volume values.
 * @param invalidValues Processed result, include valid values only.
 *
 * @return Returns <b>0</b> if the operation is successful; returns a negative value otherwise.
 */
int32_t ProcessSilence(const std::vector<double> &values, const std::vector<bool> &voiceFlag,
    const std::vector<int32_t> &volumeData, std::vector<double> &invalidValues);

/**
 * @brief Small window amplitude envelope.
 *
 * @param data Audio time series.
 * @param count length of analysis frame (in samples) for energy calculation
 * @param hopLength Hop length
 *
 * @return Returns amplitude value for each frame
 */
std::vector<double> ObtainAmplitudeEnvelop(const std::vector<double> &data, size_t count, size_t hopLength);

inline double ConvertSlaneyMel(double hz)
{
    return MEL_COEF * (log10(hz / HZ_COEF + 1.0));
}

inline double ConvertSlaneyHz(double mel)
{
    return HZ_COEF * (pow(LOG_BASE, mel / MEL_COEF) - 1.0);
}

template<typename T>
inline bool IsLessOrEqual(const T& left, const T& right)
{
    return (left - right) < std::numeric_limits<T>::epsilon();
}

template<typename T>
inline bool IsGreatOrEqual(const T& left, const T& right)
{
    return (left - right) > -std::numeric_limits<T>::epsilon();
}

template<typename T>
inline bool IsLessNotEqual(const T& left, const T& right)
{
    return (left - right) < -std::numeric_limits<T>::epsilon();
}

template<typename T>
inline bool IsGreatNotEqual(const T& left, const T& right)
{
    return (left - right) > std::numeric_limits<T>::epsilon();
}

template<typename T>
inline bool IsEqual(const T& left, const T& right)
{
    return (std::abs(left - right) <= std::numeric_limits<T>::epsilon());
}

inline double ConvertHtkMel(double frequencies)
{
    double mels = (frequencies - MIN_F) / FSP;
    if (IsGreatOrEqual(frequencies, MIN_LOG_HZ)) {
        mels = (MIN_LOG_HZ - MIN_F) / FSP + log(frequencies / MIN_LOG_HZ) / LOG_STEP;
    }
    return mels;
}

inline double ConvertHtkHz(double mels)
{
    double freqs = MIN_F + FSP * mels;
    if (IsGreatOrEqual(mels, MIN_LOG_MEL)) {
        freqs = MIN_LOG_HZ * exp(LOG_STEP * (mels - MIN_LOG_MEL));
    }
    return freqs;
}
}  // namespace Sensors
}  // namespace OHOS
#endif  // CONVERSION_UTILS_H