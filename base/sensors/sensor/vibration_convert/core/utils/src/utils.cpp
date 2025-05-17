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

#include "utils.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <numeric>

#include <unistd.h>

#include "sensor_log.h"
#include "sensors_errors.h"

#undef LOG_TAG
#define LOG_TAG "Utils"

namespace OHOS {
namespace Sensors {
namespace {
constexpr double PERCENTAGE_RANGE = 100.0;
constexpr int32_t VOICE_MIN_INTENSITY_NORM = 25;
constexpr size_t MAX_SIZE = 26460000;
}  // namespace

bool IsPowerOfTwo(uint32_t x)
{
    if ((x & (x - 1)) || (x < 2)) {
        return false;
    }
    return true;
}

uint32_t ObtainNumberOfBits(uint32_t powerOfTwo)
{
    if (powerOfTwo < 2) {
        SEN_HILOGE("powerOfTwo is an invalid value");
        return 0;
    }
    for (uint32_t i = 0;; i++) {
        if (powerOfTwo & (1 << i)) {
            return i;
        }
    }
    return 0;
}

uint32_t ReverseBits(uint32_t index, uint32_t numBits)
{
    uint32_t rev = 0;
    for (uint32_t i = 0; i < numBits; i++) {
        rev = (rev << 1) | (index & 1);
        index >>= 1;
    }
    return rev;
}

std::vector<double> TransposeMatrix(size_t rows, const std::vector<double> &values)
{
    CALL_LOG_ENTER;
    size_t valuesSize = values.size();
    SEN_HILOGD("valuesSize:%{public}zu", valuesSize);
    if ((rows == 0) || (valuesSize == 0) || (valuesSize > MAX_SIZE)) {
        SEN_HILOGE("Parameter error");
        return {};
    }
    std::vector<double> dst(valuesSize, 0.0);
    size_t cols = valuesSize / rows;
    size_t index = 0;
    if ((((cols - 1) * rows) + (rows - 1)) >= valuesSize) {
        SEN_HILOGE("dst cross-border access");
        return {};
    }
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            dst[j * rows + i] = values[index++];
        }
    }
    return dst;
}

int32_t UniqueIdx(const std::vector<int32_t> &idx, const std::vector<double> &time, std::vector<int32_t> &newIdx,
    std::vector<double> &newTime)
{
    CALL_LOG_ENTER;
    if (idx.size() != time.size()) {
        SEN_HILOGE("size of idx and time vectors not equal");
        return Sensors::ERROR;
    }
    int32_t oldIdxLen = static_cast<int32_t>(idx.size());
    int32_t idxLen = oldIdxLen;
    if (idxLen < 1) {
        return Sensors::SUCCESS;
    }
    newIdx = idx;
    newTime = time;
    int32_t preIdx = newIdx[0];
    int32_t i = 1;
    while (i < idxLen) {
        if (newIdx[i] == preIdx) {
            newIdx.erase(newIdx.begin() + i);
            newTime.erase(newTime.begin() + i);
            --i;
            --idxLen;
        }
        preIdx = newIdx[i];
        ++i;
    }
    if (idxLen != oldIdxLen) {
        SEN_HILOGI("Idx unique process");
    }
    return Sensors::SUCCESS;
}

std::vector<int32_t> NormalizePercentage(const std::vector<double> &values)
{
    CALL_LOG_ENTER;
    double minValue = *min_element(values.begin(), values.end());
    double maxValue = *max_element(values.begin(), values.end());
    double range = maxValue - minValue;
    if (IsLessNotEqual(range, EPS_MIN)) {
        SEN_HILOGE("range is less than EPS_MIN");
        return {};
    }
    std::vector<int32_t> norm;
    for (size_t i = 0; i < values.size(); i++) {
        norm.push_back(static_cast<int32_t>(round((values[i] - minValue) / range * PERCENTAGE_RANGE)));
    }
    return norm;
}

std::vector<int32_t> NormalizePercentageMin(const std::vector<double> &values)
{
    CALL_LOG_ENTER;
    std::vector<double> trimValues = values;
    for (size_t i = 0; i < trimValues.size(); i++) {
        if (IsLessNotEqual(trimValues[i], 0.0)) {
            trimValues[i] = 0.0;
        }
    }
    double minValue = 0.0;
    double maxValue = *max_element(trimValues.begin(), trimValues.end());
    double range = maxValue - minValue;
    if (IsLessNotEqual(range, EPS_MIN)) {
        SEN_HILOGE("range is less than EPS_MIN");
        return {};
    }
    std::vector<int32_t> normValues;
    for (size_t i = 0; i < trimValues.size(); i++) {
        normValues.push_back(static_cast<int32_t>(round((values[i] - minValue) / range * PERCENTAGE_RANGE)));
    }
    return normValues;
}

std::vector<int32_t> NormalizePercentageRange(const std::vector<double> &values, double minValue, double maxValue)
{
    CALL_LOG_ENTER;
    std::vector<double> trimValues = values;
    std::vector<int32_t> norm;
    for (size_t i = 0; i < trimValues.size(); i++) {
        if (IsLessNotEqual(trimValues[i], minValue)) {
            trimValues[i] = minValue;
        }
        if (IsGreatNotEqual(trimValues[i], maxValue)) {
            trimValues[i] = maxValue;
        }
    }
    double range = maxValue - minValue;
    if (IsLessNotEqual(range, EPS_MIN)) {
        SEN_HILOGE("range is less than EPS_MIN");
        return {};
    }
    for (size_t i = 0; i < trimValues.size(); i++) {
        norm.push_back(static_cast<int32_t>(round((values[i] - minValue) / range * PERCENTAGE_RANGE)));
    }
    return norm;
}

int32_t ProcessSilence(const std::vector<double> &values, const std::vector<bool> &voiceFlag,
    const std::vector<int32_t> &volumeData, std::vector<double> &invalidValues)
{
    CALL_LOG_ENTER;
    std::vector<bool> flag;
    if (!volumeData.empty()) {
        for (size_t i = 0; i < volumeData.size(); i++) {
            if (volumeData[i] > VOICE_MIN_INTENSITY_NORM) {
                flag.push_back(true);
            } else {
                flag.push_back(false);
            }
        }
    } else {
        if (voiceFlag.size() < 1) {
            SEN_HILOGE("The voice flag do not preprocess");
            return Sensors::ERROR;
        }
        flag = voiceFlag;
    }
    invalidValues = values;
    double preUnzero = -1;
    bool bExist = false;
    for (size_t j = 0; j < invalidValues.size(); j++) {
        if (flag[j]) {
            preUnzero = invalidValues[j];
            bExist = true;
            break;
        }
    }
    if (bExist) {
        for (size_t k = 0; k < invalidValues.size(); k++) {
            if (!flag[k]) {
                invalidValues[k] = preUnzero;
            } else {
                preUnzero = invalidValues[k];
            }
        }
    } else {
        SEN_HILOGI("The audio is silence");
    }
    return Sensors::SUCCESS;
}

std::vector<double> ObtainAmplitudeEnvelop(const std::vector<double> &data, size_t count, size_t hopLength)
{
    CALL_LOG_ENTER;
    if (data.empty() || (data.size() < count)) {
        SEN_HILOGE("data is empty or data is less than count");
        return {};
    }
    std::vector<double> absData;
    for (size_t i = 0; i < data.size(); i++) {
        absData.push_back(fabs(data[i]));
    }
    std::vector<double> partAmplitude;
    std::vector<double> enery;
    std::vector<double>::iterator it = absData.begin();
    double accum = 0.0;
    for (size_t j = 0; j < (data.size() - count);) {
        partAmplitude.assign(it + j, it + j + count);
        accum = accumulate(partAmplitude.begin(), partAmplitude.end(), 0);
        enery.push_back(accum);
        j += hopLength;
    }
    return enery;
}
}  // namespace Sensors
}  // namespace OHOS