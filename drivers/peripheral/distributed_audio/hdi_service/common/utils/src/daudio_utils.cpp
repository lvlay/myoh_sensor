/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "daudio_utils.h"

#include <ctime>

#include "daudio_constants.h"
#include "daudio_errcode.h"
#include "daudio_log.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "DAudioUtils"

namespace OHOS {
namespace DistributedHardware {
constexpr size_t INT32_SHORT_ID_LENGTH = 20;
constexpr size_t INT32_PLAINTEXT_LENGTH = 4;
constexpr size_t INT32_MIN_ID_LENGTH = 3;
constexpr uint8_t MAX_KEY_DH_ID_LEN = 20;

std::string GetAnonyString(const std::string &value)
{
    std::string res;
    std::string tmpStr("******");
    size_t strLen = value.length();
    if (strLen < INT32_MIN_ID_LENGTH) {
        return tmpStr;
    }

    if (strLen <= INT32_SHORT_ID_LENGTH) {
        res += value[0];
        res += tmpStr;
        res += value[strLen - 1];
    } else {
        res.append(value, 0, INT32_PLAINTEXT_LENGTH);
        res += tmpStr;
        res.append(value, strLen - INT32_PLAINTEXT_LENGTH, INT32_PLAINTEXT_LENGTH);
    }

    return res;
}

std::string GetChangeDevIdMap(int32_t devId)
{
    std::string result = PRINT_NONE;
    switch (devId) {
        case DEFAULT_CAPTURE_ID:
            result = PRINT_MIC;
            break;
        case LOW_LATENCY_RENDER_ID:
            result = PRINT_SPK;
            break;
        case DEFAULT_RENDER_ID:
            result = PRINT_SPK;
            break;
        default:
            break;
    }
    return result;
}

int32_t GetAudioParamStr(const std::string &params, const std::string &key, std::string &value)
{
    size_t step = key.size();
    if (step >= params.size()) {
        return ERR_DH_AUDIO_HDF_FAIL;
    }
    size_t pos = params.find(key);
    if (pos == params.npos || params.at(pos + step) != '=') {
        return ERR_DH_AUDIO_COMMON_NOT_FOUND_KEY;
    }
    size_t splitPosEnd = params.find(';', pos);
    if (splitPosEnd != params.npos) {
        if (pos + step + 1 > splitPosEnd) {
            return ERR_DH_AUDIO_HDF_FAIL;
        }
        value = params.substr(pos + step + 1, splitPosEnd - pos - step - 1);
    } else {
        value = params.substr(pos + step + 1);
    }
    return DH_SUCCESS;
}

int32_t GetAudioParamInt(const std::string &params, const std::string &key, int32_t &value)
{
    std::string val = "-1";
    int32_t ret = GetAudioParamStr(params, key, val);
    if (!CheckIsNum(val)) {
        DHLOGE("String is not number. str:%{public}s.", val.c_str());
        return -1;
    }
    value = std::stoi(val);
    return ret;
}

bool CheckIsNum(const std::string &jsonString)
{
    if (jsonString.empty() || jsonString.size() > MAX_KEY_DH_ID_LEN) {
        DHLOGE("Json string size is zero or too long.");
        return false;
    }
    for (char const &c : jsonString) {
        if (!std::isdigit(c)) {
            DHLOGE("Json string is not number.");
            return false;
        }
    }
    return true;
}

int32_t GetAudioParamUInt(const std::string &params, const std::string &key, uint32_t &value)
{
    value = 0;
    return DH_SUCCESS;
}

int32_t GetAudioParamBool(const std::string &params, const std::string &key, bool &value)
{
    std::string val;
    GetAudioParamStr(params, key, val);
    value = (val != "0");
    return DH_SUCCESS;
}

int32_t SetAudioParamStr(std::string &params, const std::string &key, const std::string &value)
{
    params = params + key + '=' + value + ';';
    return DH_SUCCESS;
}

int32_t GetDevTypeByDHId(int32_t dhId)
{
    if (static_cast<uint32_t>(dhId) & 0x8000000) {
        return AUDIO_DEVICE_TYPE_MIC;
    } else if (static_cast<uint32_t>(dhId) & 0x7ffffff) {
        return AUDIO_DEVICE_TYPE_SPEAKER;
    }
    return AUDIO_DEVICE_TYPE_UNKNOWN;
}

int64_t GetNowTimeUs()
{
    std::chrono::microseconds nowUs =
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());
    return nowUs.count();
}

uint32_t CalculateFrameSize(uint32_t sampleRate, uint32_t channelCount,
    int32_t format, uint32_t timeInterval, bool isMMAP)
{
    return isMMAP ? (sampleRate * channelCount * static_cast<uint32_t>(format) * timeInterval) /
                     AUDIO_MS_PER_SECOND : DEFAULT_AUDIO_DATA_SIZE;
}

int32_t CalculateSampleNum(uint32_t sampleRate, uint32_t timems)
{
    return (sampleRate * timems) / AUDIO_MS_PER_SECOND;
}

int64_t GetCurNano()
{
    int64_t result = -1;
    struct timespec time;
    clockid_t clockId = CLOCK_MONOTONIC;
    int ret = clock_gettime(clockId, &time);
    if (ret < 0) {
        return result;
    }
    result = (time.tv_sec * AUDIO_NS_PER_SECOND) + time.tv_nsec;
    return result;
}

int32_t AbsoluteSleep(int64_t nanoTime)
{
    int32_t ret = -1;
    if (nanoTime <= 0) {
        return ret;
    }
    struct timespec time;
    time.tv_sec = nanoTime / AUDIO_NS_PER_SECOND;
    time.tv_nsec = nanoTime - (time.tv_sec * AUDIO_NS_PER_SECOND);

    clockid_t clockId = CLOCK_MONOTONIC;
    ret = clock_nanosleep(clockId, TIMER_ABSTIME, &time, nullptr);
    return ret;
}

int64_t CalculateOffset(const int64_t frameIndex, const int64_t framePeriodNs, const int64_t startTime)
{
    int64_t totalOffset = GetCurNano() - startTime;
    return totalOffset - frameIndex * framePeriodNs;
}

int64_t UpdateTimeOffset(const int64_t frameIndex, const int64_t framePeriodNs, int64_t &startTime)
{
    int64_t timeOffset = 0;
    if (frameIndex == 0) {
        startTime = GetCurNano();
    } else if (frameIndex % AUDIO_OFFSET_FRAME_NUM == 0) {
        timeOffset = CalculateOffset(frameIndex, framePeriodNs, startTime);
    }
    return timeOffset;
}

bool IsOutDurationRange(int64_t startTime, int64_t endTime, int64_t lastStartTime)
{
    int64_t currentInterval = endTime - startTime;
    int64_t twiceInterval = startTime - lastStartTime;
    return (currentInterval > MAX_TIME_INTERVAL_US || twiceInterval > MAX_TIME_INTERVAL_US) ? true : false;
}

void SaveFile(const std::string fileName, uint8_t *audioData, int32_t size)
{
    char path[PATH_MAX + 1] = {0x00};
    if (fileName.length() > PATH_MAX || realpath(fileName.c_str(), path) == nullptr) {
        DHLOGE("The file path is invalid.");
        return;
    }
    std::ofstream ofs(path, std::ios::binary | std::ios::out | std::ios::app);
    if (!ofs.is_open()) {
        return;
    }
    ofs.write(reinterpret_cast<char *>(audioData), size);
    ofs.close();
}

int32_t WrapCJsonItem(const std::initializer_list<std::pair<std::string, std::string>> &keys, std::string &content)
{
    cJSON *jParam = cJSON_CreateObject();
    if (jParam == nullptr) {
        return ERR_DH_AUDIO_HDF_FAIL;
    }
    for (auto item : keys) {
        cJSON_AddStringToObject(jParam, item.first.c_str(), item.second.c_str());
    }
    char *jsonData = cJSON_PrintUnformatted(jParam);
    if (jsonData == nullptr) {
        cJSON_Delete(jParam);
        return ERR_DH_AUDIO_HDF_FAIL;
    }
    content = std::string(jsonData);
    cJSON_Delete(jParam);
    cJSON_free(jsonData);
    return DH_SUCCESS;
}

static bool IsString(const cJSON *jsonObj, const std::string &key)
{
    if (jsonObj == nullptr || !cJSON_IsObject(jsonObj)) {
        DHLOGE("JSON parameter is invalid.");
        return false;
    }
    cJSON *paramValue = cJSON_GetObjectItemCaseSensitive(jsonObj, key.c_str());
    if (paramValue == nullptr) {
        DHLOGE("paramValue is null");
        return false;
    }

    if (cJSON_IsString(paramValue)) {
        return true;
    }
    return false;
}

bool CJsonParamCheck(const cJSON *jsonObj, const std::initializer_list<std::string> &keys)
{
    if (jsonObj == nullptr || !cJSON_IsObject(jsonObj)) {
        DHLOGE("JSON parameter is invalid.");
        return false;
    }

    for (auto it = keys.begin(); it != keys.end(); it++) {
        cJSON *paramValue = cJSON_GetObjectItemCaseSensitive(jsonObj, (*it).c_str());
        if (paramValue == nullptr) {
            DHLOGE("JSON parameter does not contain key: %{public}s", (*it).c_str());
            return false;
        }
        bool res = IsString(jsonObj, *it);
        if (!res) {
            DHLOGE("The key %{public}s value format in JSON is illegal.", (*it).c_str());
            return false;
        }
    }
    return true;
}

std::string ParseStringFromArgs(const std::string &args, const char *key)
{
    DHLOGD("ParseStringFrom Args : %{public}s", args.c_str());
    cJSON *jParam = cJSON_Parse(args.c_str());
    if (jParam == nullptr) {
        DHLOGE("Failed to parse JSON: %{public}s", cJSON_GetErrorPtr());
        cJSON_Delete(jParam);
        return "Failed to parse JSON";
    }
    if (!CJsonParamCheck(jParam, { key })) {
        DHLOGE("Not found the key : %{public}s.", key);
        cJSON_Delete(jParam);
        return "Not found the key.";
    }
    cJSON *dhIdItem = cJSON_GetObjectItem(jParam, key);
    if (dhIdItem == NULL || !cJSON_IsString(dhIdItem)) {
        DHLOGE("Not found the value of the key : %{public}s.", key);
        cJSON_Delete(jParam);
        return "Not found the value.";
    }
    std::string content(dhIdItem->valuestring);
    cJSON_Delete(jParam);
    DHLOGD("Parsed string is: %{public}s.", content.c_str());
    return content;
}
} // namespace DistributedHardware
} // namespace OHOS