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

#ifndef PEAK_FINDER_H
#define PEAK_FINDER_H

#include <vector>

namespace OHOS {
namespace Sensors {
/**
 * @brief Three points on the left, middle, and right of a mountainPosition peak, aka low + high + low.
 */
struct MountainPosition {
    /** Index of the first low point on the left */
    std::vector<int32_t> firstPos;
    /** Index of the middle highest point on the peak */
    std::vector<int32_t> peakPos;
    /** Index of the last low point on the right */
    std::vector<int32_t> lastPos;
};

/** Points on both sides of the mountainPosition peak */
struct BothSidesOfPeak {
    /** Index of the left low postion */
    int32_t leftPos { 0 };
    /** Index of the right low postion */
    int32_t rightPos { 0 };
};

/** The position and value of points on the envelope appear in pairs */
struct ValleyPoint {
    std::vector<int32_t> pos;
    std::vector<double> values;
};

/**
 * @brief Peak information.
 */
struct PeaksInfo {
    /** amplitude envelope */
    std::vector<double> ampPeakEnvelope;
    /** All peak point IDs */
    std::vector<int32_t> ampPeakAllIdx;
    /** Peak IDs, after filtered. */
    std::vector<int32_t> ampPeakIdxs;
    /** Filtered, corresponding time for each peak point */
    std::vector<double> ampPeakTimes;
};

/**
 * @brief Continuous merging of several long events. Continuous merging of several short events.
 */
struct EnvelopeSegmentInfo {
    /** The starting and ending points of a paragraph. the endpoint is the starting point of the next paragraph */
    std::vector<int32_t> demarcPos;
    /** True, all events within this segment are long events, otherwise they are all short events */
    std::vector<bool> continuousEventFlag;
};

/** Outline Description of Peak Detection */
struct IsolatedEnvelopeInfo {
    /** Whether the envelope contains long events */
    bool isHaveContinuousEvent { false };
    /** Number of points for the longest continuous signal */
    int32_t longestSampleCount { 0 };
    /** The flag of peak is a transient event. */
    std::vector<bool> transientEventFlags;
    MountainPosition mountainPosition;
};

/**
 * @brief Estimation of the downward trend from the peak to the lowest point on the right.
 */
struct DownwardTrendInfo {
    /** Is it a rapidly decaying peak. */
    bool isRapidlyDecay { false };
    /** 100 point descent height */
    double dropHeight { 0.0 };
    /** The proportion of descent interval in a rms-hop length. */
    double ducyCycle { 0.0 };

    DownwardTrendInfo() = default;
    DownwardTrendInfo(bool isDecay, double drop, double cycle)
    {
        isRapidlyDecay = isDecay;
        dropHeight = drop;
        ducyCycle = cycle;
    }
};

class PeakFinder {
public:
    /**
     * @brief Calculate peak value by difference, method: low+high+low
     * 1. When equal to 0, all peak points are selected
     * 2. Like heartbeat. wav, the secondary peak point is not from the lowest point and must be removed
     *
     * @param envelope Envelope curve of the peak point to be searched.
     * @param peakThreshold If the peak drop is less than 'peakThreshold',
     * it is considered not the peak. Value range 0~1.0.
     * @return Return the index of peaks.
     */
    std::vector<int32_t> DetectPeak(const std::vector<double> &envelope, double peakThreshold);

    /**
     * @brief Get the Transient By Amplitude object
     * Find all isolated short events through the original amplitude
     * @param data Audio time series.
     * @param isolatedEnvelopeInfo A series of parameters for independent envelopes.
     *
     * @return Returns <b>0</b> if the operation is successful; returns a negative value otherwise.
     */
    int32_t ObtainTransientByAmplitude(const std::vector<double> &data, IsolatedEnvelopeInfo &isolatedEnvelopeInfo);

    /**
     * @brief Get the Voice Segment Flag
     * Must first call 'ObtainTransientByAmplitude'
     *
     * @return Return the Voice Segment Flags.
     */
    std::vector<bool> GetVoiceSegmentFlag() const;

private:
    bool GetDeleteFlagOfPeak(const std::vector<double> &envelope, int32_t peakIndex, int32_t valleyIndex,
        const std::vector<double> &valleysValue, const MountainPosition &mountainPosition);
    std::vector<double> ExtractValues(const std::vector<double> &envelope, const std::vector<int32_t> &idxs);
    std::vector<bool> GetVoiceFlag(const std::vector<double> &data,
        const std::vector<int32_t> &peaks, double lowerAmp);
    void GetEachIndependentEnvelope(const std::vector<double> &data, const std::vector<int32_t> &peaks,
        double lowerAmp, MountainPosition &mountainPosition);
    bool FindPeakBoundary(const std::vector<double> &data, int32_t peakPlace, double threshold,
        BothSidesOfPeak &bothSides);
    std::vector<int32_t> PeakFilterMinRange(const std::vector<double> &data,
        std::vector<int32_t> &peaks, int32_t minSampleCount);
    std::vector<int32_t> FilterLowPeak(const std::vector<double> &envelope,
        const std::vector<int32_t> &peaks, double removeRatio);
    std::vector<bool> SplitVoiceSlienceRange(int32_t dataSize, const std::vector<int32_t> &envelopeStart,
        const std::vector<int32_t> &envelopeLast);
    std::vector<int32_t> FilterSecondaryPeak(const std::vector<double> &envelope,
        const std::vector<int32_t> &peaks, double lowerAmp);
    int32_t DeletePeaks(const std::vector<double> &envelope, int32_t startPos, int32_t endPos,
        MountainPosition &mountainPosition, int32_t &index);
    int32_t DetectValley(const std::vector<double> &envelope, int32_t startPos, int32_t endPos,
        const MountainPosition &mountainPosition, ValleyPoint &valleyPoint);

    double GetLowestPeakValue(const std::vector<double> &envelope, const std::vector<int32_t> &peaks);
    int32_t GetPeakEnvelope(const std::vector<double> &data, int32_t samplingRate, int32_t hopLength,
        PeaksInfo &peakDetection);
    int32_t EstimateDownwardTrend(const std::vector<double> &data, const std::vector<int32_t> &peaksPoint,
        const std::vector<int32_t> &lastPeaksPoint, std::vector<DownwardTrendInfo> &downwardTrends);

    /**
     * @brief Merge continuous long envelopes and continuous pure instantaneous event intervals to avoid low energy
     *  long event outputs.
     *
     * between two short events, such as CoinDrop.wav.
     * Pre processing before peak point co envelope refinement
     * a0[0],a2[0],a0[1],a2[1]... May be discontinuous
     * Output list_I is continuous
     *
     * @param dataSize Length of envelope points
     * @param firstPos Start positions.
     * @param lastPos Last positions.
     * @param envelopeList Output continuous Long or Short Envelope Results.
     */
    void SplitLongShortEnvelope(int32_t dataSize, const std::vector<int32_t> &firstPos,
        const std::vector<int32_t> &lastPos, EnvelopeSegmentInfo &envelopeList);
    int32_t GetIsolatedEnvelope(const std::vector<double> &data, const std::vector<int32_t> &peaks,
        double lowerAmp, IsolatedEnvelopeInfo &isolatedEnvelopeInfo);
    // Descending energy
    // 1.Energy occupation ratio(area occupation ratio)
    // 2.Lowering energy difference
    // Returns < b>0 < / b > if the operation is successful; returns a negative value otherwise.
    int32_t EstimateDesentEnergy(const std::vector<double> &data, double &dropHeight, double &dutyCycle);

private:
    std::vector<bool> voiceSegmentFlag_;
    int32_t hopLength_ { 1024 };
};
}  // namespace Sensors
}  // namespace OHOS
#endif // PEAK_FINDER_H