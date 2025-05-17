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

#include "peak_finder.h"

#include <algorithm>
#include <cmath>
#include <iterator>

#include "sensor_log.h"
#include "sensors_errors.h"
#include "utils.h"

#undef LOG_TAG
#define LOG_TAG "PeakFinder"

namespace OHOS {
namespace Sensors {
namespace {
constexpr size_t MAX_N { 32 };
constexpr int32_t PEAKS_MIN_SAMPLE_COUNT { 512 };
constexpr double REMOVE_RATIO { 0.1 };
constexpr double PEAKMAX_THRESHOLD_HIGH { 0.6 };
constexpr double PEAKMAX_THRESHIOLD_MID { 0.4 };
constexpr double PEAK_LOWDELTA_RATIO_HIGH { 0.3 };
constexpr double PEAK_LOWDELTA_RATIO_MID { 0.2 };
constexpr double PEAK_LOWDELTA_RATIO_LOW { 0.1 };
constexpr int32_t LOOP_TIMES_MAX { 1000000 };
constexpr size_t GRADIENT_SIZE_MIN { 3 };
// The smaller the value, the faster the descent speed
constexpr double DOWN_TREND_MAX { 0.3 };
constexpr size_t DESCENT_WNDLEN { 128 };
constexpr int32_t HUNDRED_POINT_DESCENT_HEIGHT { 100 };
constexpr double DROP_HIGHT { 1.0 };
constexpr int32_t AMPLITUDE_ENVELOPE_HOP_LENGTH { 256 };
constexpr double DROP_HIGHT_THRESHOLD { 0.3 }; // 30%
}  // namespace

std::vector<double> PeakFinder::ExtractValues(const std::vector<double> &envelope, const std::vector<int32_t> &idxs)
{
    size_t envelopSize = envelope.size();
    size_t idxsSize = idxs.size();
    std::vector<double> values;
    for (size_t i = 0; i < idxsSize; i++) {
        if (idxs[i] >= envelopSize) {
            break;
        }
        values.push_back(envelope[idxs[i]]);
    }
    return values;
}

// In order to reduce the impact of voiceless and noise on the frequency of voiced sounds, the threshold is increased.
std::vector<bool> PeakFinder::GetVoiceFlag(const std::vector<double> &data, const std::vector<int32_t> &peaks,
    double lowerAmp)
{
    if (data.empty()) {
        SEN_HILOGE("data is empty");
        return {};
    }
    std::vector<int32_t> peakAmp;
    for (size_t i = 0; i < peaks.size(); i++) {
        if (peaks[i] > data.size()) {
            break;
        }
        peakAmp.push_back(fabs(data[peaks[i]]));
    }
    lowerAmp = *max_element(peakAmp.begin(), peakAmp.end()) * INTERSITY_BOUNDARY_POINT;
    if (peakAmp.size() > 2) {
        lowerAmp = peakAmp[static_cast<int32_t>(peakAmp.size() * INTERSITY_NUMBER_BOUNDARY_POINT)];
    }
    std::vector<double> triangularEnvelope(data.size(), 0.0);
    for (size_t i = 0; i < data.size(); i++) {
        triangularEnvelope[i] = fabs(data[i]);
    }
    double threshold = lowerAmp;
    for (size_t i = 0; i < triangularEnvelope.size(); i++) {
        if (triangularEnvelope[i] < threshold) {
            triangularEnvelope[i] = 0;
        }
    }
    size_t envelopeLength = triangularEnvelope.size();
    auto it = triangularEnvelope.begin();
    for (size_t i = 0; i < envelopeLength; i++) {
        if ((i + MAX_N) >= envelopeLength) {
            break;
        }
        triangularEnvelope[i] = *max_element(it + i, it + i + MAX_N);
    }
    MountainPosition mountainPosition;
    // Obtain the independent envelope of each peak point
    GetEachIndependentEnvelope(triangularEnvelope, peaks, lowerAmp, mountainPosition);
    // Obtain sound and silent envelopes
    return SplitVoiceSlienceRange(envelopeLength, mountainPosition.firstPos, mountainPosition.lastPos);
}

std::vector<bool> PeakFinder::GetVoiceSegmentFlag() const
{
    return voiceSegmentFlag_;
}

// Obtain soundand silent envelopes.
std::vector<bool> PeakFinder::SplitVoiceSlienceRange(int32_t dataSize, const std::vector<int32_t> &envelopeStart,
    const std::vector<int32_t> &envelopeLast)
{
    if (dataSize < 0 || hopLength_ == 0) {
        SEN_HILOGE("Invalid parameter");
        return {};
    }
    std::vector<bool> vioceFlag(dataSize, false);
    std::vector<bool> segmentFlag(ceil(static_cast<double>(dataSize) / hopLength_), false);
    for (size_t i = 0; i < envelopeStart.size(); ++i) {
        for (int32_t boundary = envelopeStart[i]; boundary <= envelopeLast[i]; ++boundary) {
            if (boundary > vioceFlag.size()) {
                break;
            }
            vioceFlag[boundary] = true;
        }
    }
    for (int32_t i = 0; i < dataSize;) {
        if (vioceFlag[i]) {
            int32_t j = i / hopLength_;
            segmentFlag[j] = true;
            i = (j + 1) * hopLength_;
        } else {
            ++i;
        }
    }
    return segmentFlag;
}

// Obtain the independent envelope of each peak point.
void PeakFinder::GetEachIndependentEnvelope(const std::vector<double> &data, const std::vector<int32_t> &peaks,
    double lowerAmp, MountainPosition &mountainPosition)
{
    int32_t lastLeftPos = -1;
    int32_t lastRightPos = -1;
    bool findBoundary = false;
    BothSidesOfPeak bothSides;
    for (size_t i = 0; i < peaks.size(); i++) {
        int32_t index = peaks[i];
        // Peak points in the same envelope.
        if ((index >= lastLeftPos) && (index < lastRightPos)) {
            mountainPosition.firstPos.push_back(bothSides.leftPos);
            mountainPosition.peakPos.push_back(index);
            mountainPosition.lastPos.push_back(bothSides.rightPos);
            continue;
        }
        // Find boundary point from peak to both sides.
        findBoundary = FindPeakBoundary(data, index, lowerAmp, bothSides);
        if (findBoundary) {
            lastLeftPos = bothSides.leftPos;
            lastRightPos = bothSides.rightPos;
            mountainPosition.firstPos.push_back(bothSides.leftPos);
            mountainPosition.peakPos.push_back(index);
            mountainPosition.lastPos.push_back(bothSides.rightPos);
        }
    }
}

// Find boundary point from peak to both sides.
bool PeakFinder::FindPeakBoundary(const std::vector<double> &data, int32_t peakPlace, double threshold,
    BothSidesOfPeak &bothSides)
{
    if (data.empty() || (peakPlace > data.size())) {
        SEN_HILOGE("data is empty or peakPlace greater than data");
        return false;
    }
    bool findLeft = false;
    int32_t i = 0;
    for (i = peakPlace; i >= 0; i--) {
        if (data[i] < threshold) {
            findLeft = true;
            bothSides.leftPos = i;
            break;
        }
    }
    // Zero hour high opening
    if (!findLeft && (i == -1)) {
        bothSides.leftPos = 0;
        findLeft = true;
    }
    size_t dataSize = data.size();
    bool findRight = false;
    size_t j = 0;
    // Find Right
    for (j = peakPlace; j < dataSize; j++) {
        if (data[j] < threshold) {
            findRight = true;
            bothSides.rightPos = j;
            break;
        }
    }
    if (!findRight && (j == dataSize)) {
        bothSides.rightPos = dataSize - 1;
        findRight = true;
    }
    return (findLeft && findRight);
}

// Based on the time difference between peak points, retain a larger value and filter the peak points.
std::vector<int32_t> PeakFinder::PeakFilterMinRange(const std::vector<double> &data, std::vector<int32_t> &peaks,
    int32_t minSampleCount)
{
    if (peaks.empty() || peaks.size() <= 1) {
        return peaks;
    }
    for (size_t i = 0; i < (peaks.size() - 1); i++) {
        if (fabs(peaks[i] - peaks[i + 1]) < minSampleCount) {
            if (peaks[i + 1] > data.size()) {
                SEN_HILOGW("peaks value greater than data size");
                return peaks;
            }
            if (data[peaks[i]] >= data[peaks[i + 1]]) {
                auto it = (peaks.begin() + i + 1);
                peaks.erase(it);
            } else {
                auto it = (peaks.begin() + i);
                peaks.erase(it);
            }
            --i;
        }
    }
    return peaks;
}

// As a short event peak, directly remove peaks smaller than 0.1 * max().
std::vector<int32_t> PeakFinder::FilterLowPeak(const std::vector<double> &envelope, const std::vector<int32_t> &peaks,
    double removeRatio)
{
    std::vector<int32_t> peakPoint = peaks;
    size_t peaksSize = peakPoint.size();
    if (peaksSize == 0) {
        return peakPoint;
    }
    std::vector<double> peakValue;

    for (size_t i = 0; i < peaksSize; i++) {
        peakValue.push_back(envelope[peakPoint[i]]);
    }
    double threshold = *max_element(peakValue.begin(), peakValue.end()) * removeRatio;
    for (int32_t i = (peaksSize - 1); i >= 0; i--) {
        if (peakValue[i] < threshold) {
            peakPoint.erase(peakPoint.begin() + i);
        }
    }
    return peakPoint;
}

// Filter secondary peak points.
// Like heartbeat.wav, mp5.wav and clavels.wav process here.
std::vector<int32_t> PeakFinder::FilterSecondaryPeak(const std::vector<double> &envelope,
    const std::vector<int32_t> &peaks, double lowerAmp)
{
    MountainPosition wholeEnvelop;
    // Obtain the independent envelope of each peak point
    GetEachIndependentEnvelope(envelope, peaks, lowerAmp, wholeEnvelop);
    if (wholeEnvelop.firstPos.empty()) {
        SEN_HILOGE("firstPos is empty");
        return {};
    }
    int32_t lastFirstPos = wholeEnvelop.firstPos[0];
    int32_t lastEndPos = wholeEnvelop.lastPos[0];
    size_t frontIndex = 0;
    int32_t n = 0;
    int32_t times = 0;
    size_t i = 1;
    while (i < wholeEnvelop.peakPos.size()) {
        ++times;
        if ((wholeEnvelop.firstPos[i] == lastFirstPos) && (wholeEnvelop.lastPos[i] == lastEndPos)) {
            ++n;
            if (i != (wholeEnvelop.peakPos.size() - 1)) {
                ++i;
                continue;
            }
        }
        if (n == 0) {
            lastFirstPos = wholeEnvelop.firstPos[i];
            lastEndPos = wholeEnvelop.lastPos[i];
            frontIndex = i;
            ++i;
            continue;
        }
        if (frontIndex == (wholeEnvelop.peakPos.size() - 1)) {
            break;
        }
        int32_t toIndex = frontIndex + n;
        // The peak points are enveloped together, and the drop of the secondary peak is less than 30 %.
        // Delete this peak point
        int32_t index = 0;
        if (DeletePeaks(envelope, frontIndex, toIndex, wholeEnvelop, index) != Sensors::SUCCESS) {
            SEN_HILOGE("DeletePeaks failed");
            return {};
        }
        if (times > LOOP_TIMES_MAX) {
            SEN_HILOGW("times should not be greater than LOOP_TIMES_MAX, times:%{public}d", times);
            break;
        }
        frontIndex = index;
        i = index;
        n = 0;
    }
    return wholeEnvelop.peakPos;
}

// The peak points are enveloped together, and the drop of the secondary peak is less than 30 %. Delete this peak point
int32_t PeakFinder::DeletePeaks(const std::vector<double> &envelope, int32_t startPos, int32_t endPos,
    MountainPosition &mountainPosition, int32_t &endIndex)
{
    bool delFlag = true;
    while (delFlag) {
        ValleyPoint valleyPoint;
        // The valley in both peaks detection.
        if (DetectValley(envelope, startPos, endPos, mountainPosition, valleyPoint) != Sensors::SUCCESS) {
            SEN_HILOGE("DetectValley failed");
            return Sensors::ERROR;
        }
        std::vector<double> valleysValue = valleyPoint.values;
        // Filter out secondary peak points
        delFlag = false;
        int32_t valleyIndex = 0;
        for (int32_t peakIndex = startPos; peakIndex < (endPos + 1); peakIndex++) {
            delFlag = GetDeleteFlagOfPeak(envelope, peakIndex, valleyIndex, valleysValue, mountainPosition);
            if (delFlag && (peakIndex < static_cast<int32_t>(mountainPosition.firstPos.size()))) {
                mountainPosition.firstPos.erase(mountainPosition.firstPos.begin() + peakIndex);
                mountainPosition.peakPos.erase(mountainPosition.peakPos.begin() + peakIndex);
                mountainPosition.lastPos.erase(mountainPosition.lastPos.begin() + peakIndex);
                --endPos;
                break;
            }
            ++valleyIndex;
        }
        if (endPos <= startPos) {
            break;
        }
    }
    endIndex = endPos;
    return Sensors::SUCCESS;
}

bool PeakFinder::GetDeleteFlagOfPeak(const std::vector<double> &envelope, int32_t peakIndex, int32_t valleyIndex,
    const std::vector<double> &valleysValue, const MountainPosition &mountainPosition)
{
    if (valleyIndex >= valleysValue.size() || peakIndex >= mountainPosition.peakPos.size()) {
        SEN_HILOGW("invalid parameter");
        return false;
    }
    double peakValue = envelope[mountainPosition.peakPos[peakIndex]];
    double hightThreshold = DROP_HIGHT_THRESHOLD * peakValue;
    bool delFlag = false;
    if (((peakValue - valleysValue[valleyIndex]) < hightThreshold) &&
        ((peakValue - valleysValue[valleyIndex + 1]) < hightThreshold)) {
        delFlag = true;
    } else if (((peakValue - valleysValue[valleyIndex]) > hightThreshold) &&
        ((peakValue - valleysValue[valleyIndex + 1]) < hightThreshold)) {
        if (peakValue < envelope[mountainPosition.peakPos[peakIndex+1]]) {
            delFlag = true;
        }
    } else if (((peakValue - valleysValue[valleyIndex]) < hightThreshold) &&
        ((peakValue - valleysValue[valleyIndex + 1]) > hightThreshold)) {
        if (peakValue < envelope[mountainPosition.peakPos[peakIndex-1]]) {
            delFlag = true;
        }
    }
    return delFlag;
}

// The valley in both peaks detection.
int32_t PeakFinder::DetectValley(const std::vector<double> &envelope, int32_t startPos, int32_t endPos,
    const MountainPosition &mountainPosition, ValleyPoint &valleyPoint)
{
    if (mountainPosition.peakPos.empty()) {
        SEN_HILOGE("peakPos is empty");
        return Sensors::ERROR;
    }
    if (startPos < 0 || endPos < 0) {
        SEN_HILOGE("startPos or endPos is wrong");
        return Sensors::ERROR;
    }
    valleyPoint.values.resize(mountainPosition.peakPos.size() + 1);
    valleyPoint.pos.resize(mountainPosition.peakPos.size() + 1);

    if ((startPos >= mountainPosition.firstPos.size()) || (endPos >= mountainPosition.lastPos.size())) {
        SEN_HILOGE("The parameter is invalid or out of bounds");
        return Sensors::ERROR;
    }
    int32_t valleyPos = mountainPosition.firstPos[startPos];
    if (startPos > 0) {
        int32_t startIndex = mountainPosition.peakPos[startPos - 1];
        int32_t endindex = mountainPosition.peakPos[startPos];
        auto iter = std::min_element((envelope.begin() + startIndex), (envelope.begin() + endindex));
        valleyPos = iter - envelope.begin();
    }
    int32_t indexCount = 0;
    valleyPoint.values[indexCount] = envelope[valleyPos];
    valleyPoint.pos[indexCount] = valleyPos;
    ++indexCount;
    for (int32_t i = startPos; i < endPos; i++) {
        int32_t startIndex = mountainPosition.peakPos[i];
        int32_t endindex = mountainPosition.peakPos[i + 1];
        auto iter = std::min_element((envelope.begin() + startIndex), (envelope.begin() + endindex));
        valleyPos = iter - envelope.begin();
        valleyPoint.values[indexCount] = envelope[valleyPos];
        valleyPoint.pos[indexCount] = valleyPos;
        ++indexCount;
    }
    valleyPos = mountainPosition.lastPos[endPos];
    valleyPoint.values[indexCount] = envelope[valleyPos];
    valleyPoint.pos[indexCount] = valleyPos;
    return Sensors::SUCCESS;
}

// Calculate peak value by difference, method: low+high+low
// 1. When equal to 0, all peak points are selected
// 2. In situations such as OnCarpet.wav and CoinDrop.wav with zero hour high opening,
//    having more than 2 segments is the true high
// 3. Like heartbeat. wav, the secondary peak point is not from the lowest point and must be removed
std::vector<int32_t> PeakFinder::DetectPeak(const std::vector<double> &envelope, double peakThreshold)
{
    CALL_LOG_ENTER;
    if (envelope.empty() || envelope.size() <= 1) {
        SEN_HILOGE("envelope is empty or envelope is less than or equal to one");
        return {};
    }
    std::vector<double> gradientEnvelope;
    for (size_t i = 0; i < (envelope.size() - 1); i++) {
         gradientEnvelope.push_back(envelope[i+1] - envelope[i]);
    }
    if (gradientEnvelope.empty()) {
        SEN_HILOGE("gradientEnvelope is empty");
        return {};
    }
    std::vector<double> gradient;
    for (size_t i = 0; i < (gradientEnvelope.size() - 1); i++) {
        if ((gradientEnvelope[i] > 0) && (gradientEnvelope[i+1] < 0)) {
            gradient.push_back((gradientEnvelope[i] + fabs(gradientEnvelope[i+1])) / 2);
        }
    }
    // At begining, no rising edge, directly at the maximum point
    std::vector<int32_t> peaks;
    if ((gradientEnvelope[0] < 0) && (gradientEnvelope[1] < 0)) {
        peaks.push_back(0);
    }
    if (peakThreshold < EPS_MIN) {
        for (size_t j = 0; j < (gradientEnvelope.size() - 1); j++) {
            if((gradientEnvelope[j] > 0) && (gradientEnvelope[j+1] < 0)) {
                peaks.push_back(j+1);
            }
        }
        return peaks;
    }
    double thresholdGradient = *max_element(gradient.begin(), gradient.end()) / 2;
    if (gradient.size() > GRADIENT_SIZE_MIN) {
        gradient.erase(max_element(gradient.begin(), gradient.end()));
        gradient.erase(max_element(gradient.begin(), gradient.end()));
        thresholdGradient = *max_element(gradient.begin(), gradient.end()) * peakThreshold;
    }
    for (size_t k = 0; k < (gradientEnvelope.size() - 1); k++) {
        if ((gradientEnvelope[k] > 0) && (gradientEnvelope[k+1] < 0)) {
            if ((gradientEnvelope[k] + fabs(gradientEnvelope[k+1])) / 2 > thresholdGradient) {
                peaks.push_back(k+1);
            }
        }
    }
    return peaks;
}

// Starting from the peak, find the first lowest point on both sides
// The minimum amplitude obtained through this method is small
// Envelope: multi point maximum envelope
double PeakFinder::GetLowestPeakValue(const std::vector<double> &envelope, const std::vector<int32_t> &peaks)
{
    if (peaks.empty()) {
        SEN_HILOGE("peaks is empty");
        return Sensors::ERROR;
    }
    std::vector<double> peakValues;
    for (size_t i = 0; i < peaks.size(); i++) {
        peakValues.push_back(envelope[peaks[i]]);
    }
    double maxPeak = *max_element(peakValues.begin(), peakValues.end());
    double minPeak = *min_element(peakValues.begin(), peakValues.end());
    if (maxPeak > PEAKMAX_THRESHOLD_HIGH) {
        minPeak *= PEAK_LOWDELTA_RATIO_HIGH;
    } else if (maxPeak > PEAKMAX_THRESHIOLD_MID) {
        minPeak *= PEAK_LOWDELTA_RATIO_MID;
    } else {
        minPeak *= PEAK_LOWDELTA_RATIO_LOW;
    }
    return minPeak;
}

// Determine the peak position and parameters of short events through amplitude values.
int32_t PeakFinder::GetPeakEnvelope(const std::vector<double> &data, int32_t samplingRate, int32_t hopLength,
    PeaksInfo &peakDetection)
{
    CALL_LOG_ENTER;
    if ((data.empty()) || (samplingRate == 0) || (hopLength == 0)) {
        SEN_HILOGE("Invalid parameter");
        return Sensors::PARAMETER_ERROR;
    }
    std::vector<double> absData;
    for (size_t i = 0; i < data.size(); i++) {
        absData.push_back(fabs(data[i]));
    }
    size_t maxN = static_cast<size_t>(hopLength);
    std::vector<double> peakEnvelope;
    std::vector<int32_t> peakEnvelopeIdx;
    size_t i = 0;
    while ((i + maxN) <= absData.size()) {
        auto it = std::max_element(absData.begin() + i, absData.begin() + i + maxN);
        int32_t maxIndex = it - absData.begin();
        peakEnvelopeIdx.push_back(maxIndex);
        peakEnvelope.push_back(*it);
        i += maxN;
    }
    // Peak detection
    std::vector<int32_t> peakAllIdx = DetectPeak(peakEnvelope, REMOVE_RATIO);

    // Filter low peak
    peakAllIdx = FilterLowPeak(peakEnvelope, peakAllIdx, REMOVE_RATIO);
    std::vector<double> extractValues = ExtractValues(peakEnvelope, peakAllIdx);
    double lowerAmp = *std::min_element(extractValues.begin(), extractValues.end()) * PEAK_LOWDELTA_RATIO_LOW;

    // Filter secondary peak points
    std::vector<int32_t> ampPeakBigIdx = FilterSecondaryPeak(peakEnvelope, peakAllIdx, lowerAmp);
    if (ampPeakBigIdx.empty()) {
        SEN_HILOGE("ampPeakBigIdx is empty");
        return Sensors::ERROR;
    }
    peakDetection.ampPeakEnvelope = peakEnvelope;
    peakDetection.ampPeakAllIdx = peakAllIdx;

    std::vector<int32_t> ampPeakAct;
    for (i = 0; i < ampPeakBigIdx.size(); i++) {
        if (ampPeakBigIdx[i] >= peakEnvelopeIdx.size()) {
            break;
        }
        ampPeakAct.push_back(peakEnvelopeIdx[ampPeakBigIdx[i]]);
    }
    // Based on the time difference between peak points, retain a larger value and filter the peak points
    peakDetection.ampPeakIdxs = PeakFilterMinRange(absData, ampPeakAct, PEAKS_MIN_SAMPLE_COUNT);
    double coef = 1.0 / samplingRate;
    for (i = 0; i < peakDetection.ampPeakIdxs.size(); i++) {
        peakDetection.ampPeakTimes.push_back(peakDetection.ampPeakIdxs[i] * coef);
    }
    return Sensors::SUCCESS;
 }

// Estimating the downward trend of isolated short events.
// A descent height of less than 0.03 at 100 points indicates a slow descent.
int32_t PeakFinder::EstimateDownwardTrend(const std::vector<double> &data, const std::vector<int32_t> &peaksPoint,
    const std::vector<int32_t> &lastPeaksPoint, std::vector<DownwardTrendInfo> &downwardTrends)
{
    if (peaksPoint.empty() || lastPeaksPoint.size() < peaksPoint.size()) {
        SEN_HILOGE("Invalid parameter");
        return Sensors::ERROR;
    }
    bool isRapidlyDecay = false;
    std::vector<double> partEnvelope;
    for (size_t i = 0; i < peaksPoint.size(); i++) {
        for (int32_t j = peaksPoint[i]; j < lastPeaksPoint[i]; j++) {
            if (data[j] > data.size()) {
                SEN_HILOGE("parameter is error");
                break;
            }
            partEnvelope.push_back(data[j]);
        }
        double dropHeight = 0.0;
        double ducyCycle = 0.0;
        if (EstimateDesentEnergy(partEnvelope, dropHeight, ducyCycle) != Sensors::SUCCESS) {
            SEN_HILOGD("EstimateDesentEnergy failed");
            continue;
        }
        dropHeight *= HUNDRED_POINT_DESCENT_HEIGHT ;
        if (IsLessNotEqual(ducyCycle, DOWN_TREND_MAX) && IsGreatNotEqual(dropHeight, DROP_HIGHT)) {
            isRapidlyDecay = true;
        } else {
            isRapidlyDecay = false;
        }
        downwardTrends.push_back(DownwardTrendInfo(isRapidlyDecay, dropHeight, ducyCycle));
    }
    return Sensors::SUCCESS;
}

// Merge continuous long envelopes and continuous pure instantaneous event intervals to avoid low energy
//   long event outputs.
// between two short events, such as CoinDrop.wav.
// Pre processing before peak point co envelope refinement
// a0[0],a2[0],a0[1],a2[1]... May be discontinuous
// Output list_ I is continuous
void PeakFinder::SplitLongShortEnvelope(int32_t dataSize, const std::vector<int32_t> &firstPos,
    const std::vector<int32_t> &lastPos, EnvelopeSegmentInfo &envelopeList)
{
    int32_t leastCount = 2 * hopLength_;
    std::vector<int32_t> countAssemble;
    for (size_t i = 0; i < firstPos.size(); i++) {
        countAssemble.push_back(lastPos[i] - firstPos[i]);
    }
    int32_t longestSampleCount = *max_element(countAssemble.begin(), countAssemble.end());
    if (longestSampleCount <= leastCount) {
        SEN_HILOGE("longestSampleCount must be less than or equal to leastCount");
        return;
    }
    bool preFlag = false;
    int32_t preIndex = 0;
    size_t countAssembleLen = countAssemble.size();
    if (countAssembleLen == 0) {
        SEN_HILOGE("countAssembleLen should not be 0");
        return;
    }
    for (size_t j = 0; j < countAssembleLen; j++) {
        bool flag = true;
        if (countAssemble[j] < hopLength_) {
            flag = false;
        }
        int32_t toIndex = lastPos[j];
        if (j == 0) {
            envelopeList.continuousEventFlag.push_back(flag);
            envelopeList.demarcPos.push_back(0);
            envelopeList.demarcPos.push_back(toIndex);
            preFlag = flag;
        } else if (j == (countAssembleLen - 1)) {
            if (flag != preFlag) {
                envelopeList.continuousEventFlag.push_back(flag);
                envelopeList.demarcPos.push_back(preIndex);
                preFlag = flag;
            }
            envelopeList.demarcPos[envelopeList.demarcPos.size() - 1] = dataSize;
        } else {
            if (flag == preFlag) {
                envelopeList.demarcPos[envelopeList.demarcPos.size() - 1] = toIndex;
            } else {
                envelopeList.continuousEventFlag.push_back(flag);
                envelopeList.demarcPos.push_back(preIndex);
                envelopeList.demarcPos.push_back(toIndex);
                preFlag = flag;
            }
        }
        preIndex = toIndex;
    }
}

// Calculate the starting and ending envelopes of isolated pure short events, searching from peak to both sides
// Do not merge common envelope peaks
// Missing trackback function similar to note when co enveloping
int32_t PeakFinder::GetIsolatedEnvelope(const std::vector<double> &data, const std::vector<int32_t> &peaks,
    double lowerAmp, IsolatedEnvelopeInfo &isolatedEnvelopeInfo)
{
    std::vector<double> triangularEnvelope(data.size(), 0.0);
    for (size_t i = 0; i < data.size(); i++) {
        triangularEnvelope[i] = fabs(data[i]);
        if (triangularEnvelope[i] < lowerAmp) {
            triangularEnvelope[i] = 0;
        }
    }
    size_t envelopeSize = triangularEnvelope.size();
    size_t j = 0;
    while ((j + MAX_N) <= envelopeSize) {
        double maxValue = *std::max_element(triangularEnvelope.begin() + j, triangularEnvelope.begin() + j + MAX_N);
        triangularEnvelope[j] = maxValue;
        ++j;
    }
    int32_t leastCount = 2 * hopLength_;
    MountainPosition mountainPosition;
    // Obtain the independent envelope of each peak point
    GetEachIndependentEnvelope(triangularEnvelope, peaks, lowerAmp, mountainPosition);
    if (mountainPosition.peakPos.empty()) {
        SEN_HILOGE("peakPos is empty");
        return Sensors::ERROR;
    }
    EnvelopeSegmentInfo envelopeList;
    // Merge continuous long envelopes and continuous pure instantaneous event intervals
    SplitLongShortEnvelope(envelopeSize, mountainPosition.firstPos, mountainPosition.lastPos, envelopeList);
    ValleyPoint valleyPoint;
    size_t envPeakLen = mountainPosition.peakPos.size() - 1;
    // The valleyes in both peaks to detection
    if (DetectValley(triangularEnvelope, 0, envPeakLen, mountainPosition, valleyPoint) != Sensors::SUCCESS) {
        SEN_HILOGE("DetectValley failed");
        return Sensors::ERROR;
    }
    std::vector<int32_t> countAssemble;
    for (size_t i = 0; i < mountainPosition.lastPos.size(); i++) {
        countAssemble.push_back(mountainPosition.lastPos[i] - mountainPosition.firstPos[i]);
    }
    isolatedEnvelopeInfo.isHaveContinuousEvent = false;
    isolatedEnvelopeInfo.mountainPosition = mountainPosition;
    isolatedEnvelopeInfo.longestSampleCount = *max_element(countAssemble.begin(), countAssemble.end());
    if (isolatedEnvelopeInfo.longestSampleCount > leastCount) {
        isolatedEnvelopeInfo.isHaveContinuousEvent = true;
    }
    for (size_t i = 0; i < countAssemble.size(); i++) {
        bool flag = false;
        if (countAssemble[i] < hopLength_) {
            flag = true;
        }
        isolatedEnvelopeInfo.transientEventFlags.push_back(flag);
    }
    return Sensors::SUCCESS;
}

// Find all isolated short events through the original amplitude
int32_t PeakFinder::ObtainTransientByAmplitude(const std::vector<double> &data,
    IsolatedEnvelopeInfo &isolatedEnvelopeInfo)
{
    if (data.empty()) {
        SEN_HILOGE("data is empty");
        return Sensors::ERROR;
    }
    PeaksInfo peakDetection;
    // Determine the peak position and parameters of short events through amplitude values
    GetPeakEnvelope(data, SAMPLE_RATE, AMPLITUDE_ENVELOPE_HOP_LENGTH, peakDetection);

    // Starting from the peak, find the first lowest point on both sides
    double ampLowerDalta = GetLowestPeakValue(peakDetection.ampPeakEnvelope, peakDetection.ampPeakAllIdx);

    // Calculate the starting and ending envelopes of isolated pure short events, searching from peak to both sides
    if (GetIsolatedEnvelope(data, peakDetection.ampPeakIdxs,
        ampLowerDalta, isolatedEnvelopeInfo) != Sensors::SUCCESS) {
        SEN_HILOGE("GetIsolatedEnvelope failed");
        return Sensors::ERROR;
    }
    // In order to reduce the impact of voiceless and noise on the frequency of voiced sounds,
    // the threshold is increased
    voiceSegmentFlag_ = GetVoiceFlag(data, peakDetection.ampPeakIdxs, ampLowerDalta);
    std::vector<DownwardTrendInfo> downwardTrends;
    // Estimating the downward trend of isolated short events
    int32_t ret = EstimateDownwardTrend(data, isolatedEnvelopeInfo.mountainPosition.peakPos,
        isolatedEnvelopeInfo.mountainPosition.lastPos, downwardTrends);
    if (downwardTrends.size() > 0) {
        SEN_HILOGD("isRapidlyDecay:%{public}d,dropHeight:%{public}lf,ducyCycle:%{public}lf",
            downwardTrends[0].isRapidlyDecay, downwardTrends[0].dropHeight, downwardTrends[0].ducyCycle);
    }
    return ret;
}

int32_t PeakFinder::EstimateDesentEnergy(const std::vector<double> &data, double &dropHeight, double &dutyCycle)
{
    if (data.empty()) {
        SEN_HILOGE("data is empty");
        return Sensors::ERROR;
    }
    std::vector<double> blockSum = ObtainAmplitudeEnvelop(data, DESCENT_WNDLEN, DESCENT_WNDLEN);
    if (blockSum.empty()) {
        SEN_HILOGE("blockSum is empty");
        return Sensors::ERROR;
    }
    // Number of consecutive drops.
    size_t n = 1;
    for (size_t i = 0; i < (blockSum.size() - 1); i++) {
        if (blockSum[i] < blockSum[i + 1]) {
            break;
        }
        ++n;
    }
    size_t dataSize = data.size();
    dropHeight = 0;
    if (n == 1) {
        dropHeight = blockSum[0] / dataSize;
    } else {
        dropHeight = (blockSum[0] - blockSum[1]) / DESCENT_WNDLEN;
    }

    // Estimating within 1024 sampling points (0.046ms).
    int32_t miniFrmN = ENERGY_HOP_LEN / DESCENT_WNDLEN;
    if (n > miniFrmN) {
        n = miniFrmN;
    }
    double totalEnergy = 0.0;
    for (size_t i = 0; i < n; i++) {
        totalEnergy += blockSum[i];
    }
    double virtualWholeEnergy = miniFrmN * blockSum[0];
    if (virtualWholeEnergy < EPS_MIN) {
        dutyCycle = 0.0;
        SEN_HILOGW("The virtualWholeEnergy value is too low");
    }
    dutyCycle = totalEnergy / virtualWholeEnergy;
    return Sensors::SUCCESS;
}
}  // namespace Sensors
}  // namespace OHOS
