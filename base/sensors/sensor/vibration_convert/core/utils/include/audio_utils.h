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

#ifndef AUDIO_UTILS_H
#define AUDIO_UTILS_H

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <vector>

#include "utils.h"

namespace OHOS {
namespace Sensors {
namespace {
constexpr double DB_TO_AMP_COEF { 0.05 };
constexpr double AMP_TO_DB_COEF { 20.0 };
}  // namespace

/**
 *@brief Basic processing of audio, called by other modules.
 */
class AudioUtils {
public:
    /**
     * @brief Convert from MIDI note number to frequency (Hz)
     *
     * @param midinote A MIDI note number
     */
    double ConvertMtof(int32_t midinote);

    /**
     * @brief Convert from milliseconds to samples
     *
     * @param timeMs The number of milliseconds
     *
     */
    size_t ConvertMsToSamps(double timeMs, double sampleRate)
    {
        return static_cast<size_t>((timeMs / SAMPLE_IN_MS) * sampleRate);
    }

    /**
     * @brief Convert from samples to milliseconds
     *
     * @param samples The number of samples
     */
    double ConvertSampsToMs(size_t samples, double sampleRate)
    {
        if (IsLessOrEqual(sampleRate, 0.0)) {
            return 0.0;
        }
        return static_cast<double>(samples) / sampleRate * SAMPLE_IN_MS;
    }

    /**
     * @brief Convert from amplitude to decibels
     *
     * @param amp Amplitude
     */
    double ConvertAmpToDbs(double amp)
    {
        return std::log10(amp) * AMP_TO_DB_COEF;
    }

    /**
     * @brief Convert from decibels to amplitude
     *
     * @param dbs Decibels
     */
    double ConvertDbsToAmp(double dbs)
    {
        return std::pow(LOG_BASE, dbs * DB_TO_AMP_COEF);
    }

    /**
     * @brief Audio data padding in center.
     *
     * If `True` and operating on time-domain input (`data`), pad the signal
     *   by `hopLength/2` on either side.
     *
     * @param data Audio data.
     */
    std::vector<double> PadData(const std::vector<double> &data, int32_t hopLength);
};
}  // namespace Sensors
}  // namespace OHOS
#endif