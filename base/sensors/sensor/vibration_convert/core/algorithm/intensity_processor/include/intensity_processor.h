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

#ifndef INTENSITY_PROCESSOR_H
#define INTENSITY_PROCESSOR_H

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <numeric>
#include <vector>

namespace OHOS {
namespace Sensors {
/**
Intensity functions
*/
class IntensityProcessor {
public:
    /**
     * @brief Compute root-mean-square (RMS) value for each frame, from the audio samples 'data'.
     *
     * @param data Audio time series.
     * @param hopLength hop length for STFT.
     * @param centerFlag If `True` and operating on time-domain input ('data'), pad the signal,by 'frame_length/2' on
     *   either side.
     *
     * @return Return RMS value for each frame
     */
    std::vector<double> GetRMS(const std::vector<double> &data, int32_t hopLength, bool centerFlag);

    /**
     * @brief Sum of sampling squares within the window
     *
     * @param data Audio time series.
     * @param nFft length of analysis frame (in samples) for energy calculation
     * @param hopLength Hop length for STFT.
     *
     * @return Return energy value for each frame
     */
    std::vector<double> EnergyEnvelop(const std::vector<double> &data, int32_t nFft, int32_t hopLength);

    /**
     * @brief RMS numerical normalization.
     * Eliminate low energy and normalize it.
     *
     * @param rmseEnvelope RMS value for each frame
     * @param lowerDelta The value below 'lowerDelta' is set to 0.
     * @param rmseBand Envelope after setting low energy to 0.
     * @param rmseNorm Normalized value, ranging from 0 to 100.
     */
    int32_t RmseNormalize(const std::vector<double> &rmseEnvelope, double lowerDelta, std::vector<double> &rmseBand,
        std::vector<int32_t> &rmseNorm);

    /**
     * @brief Calculate linear volume values within each window.
     *
     * @param data Audio time series.
     * @param hopLength The number of samples between consecutive frames (such as columns in a spectrogram) at each
     *  sliding distance (skip length) of the window.
     *
     * @return Volume value for each frame
     */
    std::vector<double> VolumeInLinary(const std::vector<double> &data, int32_t hopLength);

    /**
     * @brief Calculate the DB volume value within each window.
     *
     * @param data Audio time series.
     * @param hopLength The number of samples between consecutive frames (such as columns in a spectrogram) at each
     *  sliding distance (skip length) of the window.
     *
     * @return DB value for each frame
     */
    std::vector<double> VolumeInDB(const std::vector<double> &data, int32_t hopLength);
};
}  // namespace Sensors
}  // namespace OHOS
#endif // INTENSITY_PROCESSOR_H