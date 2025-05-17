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

#include "intensity_processor.h"

#include "audio_utils.h"
#include "sensor_log.h"
#include "sensors_errors.h"

#undef LOG_TAG
#define LOG_TAG "IntensityProcessor"

namespace OHOS {
namespace Sensors {
namespace {
constexpr double VOLUME_DB_COEF { 10.0 };
}  // namespace

std::vector<double> IntensityProcessor::GetRMS(const std::vector<double> &data, int32_t hopLength, bool centerFlag)
{
    CALL_LOG_ENTER;
    std::vector<double> rmseEnvelop;
    if ((data.empty()) || (hopLength < 1)) {
        SEN_HILOGE("data is empty or hopLength is less than 1");
        return rmseEnvelop;
    }
    std::vector<double> paddingData;
    if (centerFlag) {
        AudioUtils audioUtil;
        paddingData = audioUtil.PadData(data, hopLength);
    } else {
        paddingData = data;
    }
    size_t paddingDataSize = paddingData.size();
    size_t frmN = paddingDataSize / static_cast<size_t>(hopLength);
    for (size_t i = 0; i < frmN; ++i) {
        double accum = 0.0;
        int32_t index = i * hopLength;
        for (int32_t j = 0; j < hopLength; ++j, ++index) {
            accum += pow(paddingData[index], 2);
        }
        rmseEnvelop.push_back(sqrt(accum / hopLength));
    }
    return rmseEnvelop;
}

std::vector<double> IntensityProcessor::EnergyEnvelop(const std::vector<double> &data, int32_t nFft, int32_t hopLength)
{
    std::vector<double> dataAbsPower;
    size_t dataSize = data.size();
    for (size_t i = 0; i < dataSize; ++i) {
        dataAbsPower.push_back(data[i] * data[i]);
    }
    std::vector<double> oneFrmData;
    std::vector<double> blockAmplitudeSum;
    auto it = dataAbsPower.begin();
    for (size_t i = 0; (i + nFft) < dataSize;) {
        oneFrmData.assign(it + i, it + i + nFft);
        double accum = accumulate(begin(oneFrmData), end(oneFrmData), 0);
        blockAmplitudeSum.emplace_back(accum);
        i += hopLength;
    }
    return blockAmplitudeSum;
}

int32_t IntensityProcessor::RmseNormalize(const std::vector<double> &rmseEnvelope, double lowerDelta,
    std::vector<double> &rmseBand, std::vector<int32_t> &rmseNorm)
{
    CALL_LOG_ENTER;
    if (rmseEnvelope.empty()) {
        SEN_HILOGE("rmseEnvelope is empty");
        return Sensors::ERROR;
    }
    // Set low energy to 0.
    rmseBand = rmseEnvelope;
    for (size_t i = 0; i < rmseBand.size(); ++i) {
        if (rmseBand[i] < lowerDelta) {
            rmseBand[i] = 0;
        }
    }
    rmseNorm = OHOS::Sensors::NormalizePercentage(rmseBand);
    if (rmseNorm.empty()) {
        SEN_HILOGE("rmseNorm is empty");
        return Sensors::ERROR;
    }
    return Sensors::SUCCESS;
}

std::vector<double> IntensityProcessor::VolumeInLinary(const std::vector<double> &data, int32_t hopLength)
{
    CALL_LOG_ENTER;
    std::vector<double> dataAbsPower;
    size_t dataSize = data.size();
    for (size_t i = 0; i < dataSize; ++i) {
        dataAbsPower.push_back(abs(data[i]));
    }
    std::vector<double> oneFrmData;
    std::vector<double> blockAmplitudeSum;
    auto it = dataAbsPower.begin();
    for (size_t i = 0; (i + hopLength) < dataSize;) {
        oneFrmData.assign(it + i, it + i + hopLength);
        double accum = accumulate(oneFrmData.begin(), oneFrmData.end(), 0);
        blockAmplitudeSum.push_back(accum);
        i += hopLength;
    }
    return blockAmplitudeSum;
}

std::vector<double> IntensityProcessor::VolumeInDB(const std::vector<double> &data, int32_t hopLength)
{
    CALL_LOG_ENTER;
    std::vector<double> blockSum = EnergyEnvelop(data, hopLength, hopLength);
    std::vector<double> db(blockSum.size(), 0.0);
    for (size_t i = 0; i < blockSum.size(); ++i) {
        db[i] = VOLUME_DB_COEF * log(blockSum[i]);
        if (IsLessNotEqual(db[i], 0.0)) {
            db[i] = 0.0;
        }
    }
    return db;
}
}  // namespace Sensors
}  // namespace OHOS