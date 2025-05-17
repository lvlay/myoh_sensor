/*
 * Copyright (c) 2022 Chipsea Technologies (Shenzhen) Corp., Ltd.
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

#include "medical_sensor_dump.h"

#include <cinttypes>
#include <ctime>
#include <queue>

#include "medical_log_domain.h"
#include "medical_errors.h"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;

enum class DumpDataSizeType {
    PPG_DATA_SIZE = 400,
    COMMON_DATA_SIZE = 1,
};

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, MedicalSensorLogDomain::MEDICAL_SENSOR_SERVICE, "MedicalSensorDump" };
constexpr uint32_t MAX_DUMP_DATA_SIZE = 10;
constexpr uint32_t MS_NS = 1000000;
constexpr uint32_t PPG = 256;
constexpr int32_t MAX_DMUP_PARAM = 2;
}  // namespace

std::unordered_map<uint32_t, std::string> MedicalSensorDump::sensorMap_ = {
    { PPG, "PPG" },
};

bool MedicalSensorDump::DumpSensorHelp(int32_t fd, const std::vector<std::u16string> &args)
{
    if ((args.empty()) || (args[0].compare(u"-h") != 0)) {
        HiLog::Error(LABEL, "%{public}s args cannot be empty or invalid", __func__);
        return false;
    }
    DumpHelp(fd);
    return true;
}

void MedicalSensorDump::DumpHelp(int32_t fd)
{
    dprintf(fd, "Usage:\n");
    dprintf(fd, "      -h: dump help\n");
    dprintf(fd, "      -l: dump the sensor list\n");
    dprintf(fd, "      -c: dump the sensor data channel info\n");
    dprintf(fd, "      -o: dump the opening sensors\n");
    dprintf(fd, "      -d: dump the last 10 packages sensor data\n");
}

bool MedicalSensorDump::DumpSensorList(int32_t fd,
    const std::vector<MedicalSensor> &sensors, const std::vector<std::u16string> &args)
{
    if ((args.empty()) || (args[0].compare(u"-l") != 0)) {
        HiLog::Error(LABEL, "%{public}s args cannot be empty or invalid", __func__);
        return false;
    }
    DumpCurrentTime(fd);
    dprintf(fd, "Total sensor:%d, MedicalSensor list:\n", int32_t { sensors.size() });
    for (const auto &sensor : sensors) {
        auto sensorId = sensor.GetSensorId();
        dprintf(fd,
                "sensorId:%8u | sensorType:%s | name:%s | vendor:%s | maxRange:%f | fifoMaxEventCount:%d "
                "| minSamplePeriodNs:%lld | maxSamplePeriodNs:%lld\n",
                sensorId, sensorMap_[sensorId].c_str(), sensor.GetName().c_str(), sensor.GetVendor().c_str(),
                sensor.GetMaxRange(), sensor.GetFifoMaxEventCount(), (long long) { sensor.GetMinSamplePeriodNs() },
                (long long) { sensor.GetMaxSamplePeriodNs() });
    }
    return true;
}

bool MedicalSensorDump::DumpSensorChannel(int32_t fd, ClientInfo &clientInfo, const std::vector<std::u16string> &args)
{
    if ((args.empty()) || (args[0].compare(u"-c") != 0)) {
        HiLog::Error(LABEL, "%{public}s args cannot be empty or invalid", __func__);
        return false;
    }
    DumpCurrentTime(fd);
    dprintf(fd, "MedicalSensor channel info:\n");
    std::vector<MedicalSensorChannelInfo> channelInfo;
    clientInfo.GetSensorChannelInfo(channelInfo);
    for (const auto &channel : channelInfo) {
        auto sensorId = channel.GetSensorId();
        std::string cmds("");
        auto cmdList = channel.GetCmdType();
        for (auto cmd : cmdList) {
            cmds += (std::to_string(cmd) + " ");
        }
        dprintf(fd,
                "uid:%d | packageName:%s | sensorId:%8u | sensorType:%s | samplingPeriodNs:%d "
                "| fifoCount:%u | cmdType:%s\n",
                channel.GetUid(), channel.GetPackageName().c_str(), sensorId, sensorMap_[sensorId].c_str(),
                int32_t { channel.GetSamplingPeriodNs() }, channel.GetFifoCount(), cmds.c_str());
    }
    return true;
}

bool MedicalSensorDump::DumpOpeningSensor(int32_t fd, const std::vector<MedicalSensor> &sensors,
    ClientInfo &clientInfo, const std::vector<std::u16string> &args)
{
    if ((args.empty()) || (args[0].compare(u"-o") != 0)) {
        HiLog::Error(LABEL, "%{public}s args cannot be empty or invalid", __func__);
        return false;
    }
    DumpCurrentTime(fd);
    dprintf(fd, "Opening sensors:\n");
    for (const auto &sensor : sensors) {
        uint32_t sensorId = sensor.GetSensorId();
        if (clientInfo.GetSensorState(sensorId) == SENSOR_ENABLED) {
            dprintf(fd, "sensorId: %8u | sensorType: %s\n", sensorId, sensorMap_[sensorId].c_str());
        }
    }
    return true;
}

bool MedicalSensorDump::DumpSensorData(int32_t fd, ClientInfo &clientInfo, const std::vector<std::u16string> &args)
{
    if ((args.empty()) || (args[0].compare(u"-d") != 0)) {
        HiLog::Error(LABEL, "%{public}s args cannot be empty or invalid", __func__);
        return false;
    }
    dprintf(fd, "Last 10 packages sensor data:\n");
    auto dataMap = clientInfo.GetDataQueue();
    int32_t j = 0;
    for (auto &sensorData : dataMap) {
        uint32_t sensorId = sensorData.first;
        dprintf(fd, "sensorId: %8u | sensorType: %s:\n", sensorId, sensorMap_[sensorId].c_str());
        for (uint32_t i = 0; i < MAX_DUMP_DATA_SIZE && (!sensorData.second.empty()); i++) {
            auto data = sensorData.second.front();
            sensorData.second.pop();
            timespec time = { 0, 0 };
            tm *timeinfo = localtime(&(time.tv_sec));
            if (timeinfo == nullptr) {
                HiLog::Error(LABEL, "%{public}s timeinfo cannot be null", __func__);
                return false;
            }
            dprintf(fd, "      %2d (ts=%.9f, time=%02d:%02d:%02d.%03d) | data:%s", ++j, data.timestamp / 1e9,
                    timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, int32_t { (time.tv_nsec / MS_NS) },
                    GetDataBySensorId(sensorId, data).c_str());
        }
    }
    return true;
}

void MedicalSensorDump::DumpCurrentTime(int32_t fd)
{
    timespec curTime = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &curTime);
    tm *timeinfo = localtime(&(curTime.tv_sec));
    if (timeinfo == nullptr) {
        HiLog::Error(LABEL, "%{public}s timeinfo cannot be null", __func__);
        return;
    }
    dprintf(fd, "Current time: %02d:%02d:%02d.%03d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
            int32_t { (curTime.tv_nsec / MS_NS) });
}

int32_t MedicalSensorDump::DataSizeBySensorId(uint32_t sensorId)
{
    switch (sensorId) {
        case PPG:
            return static_cast<int32_t>(DumpDataSizeType::PPG_DATA_SIZE);
        default:
            return static_cast<int32_t>(DumpDataSizeType::COMMON_DATA_SIZE);
    }
}

std::string MedicalSensorDump::GetDataBySensorId(uint32_t sensorId, struct SensorEvent &sensorData)
{
    HiLog::Debug(LABEL, "%{public}s sensorId: %{public}u", __func__, sensorId);
    std::string buffer;
    int32_t dataLen = DataSizeBySensorId(sensorId);
    for (int32_t i = 0; i < dataLen; ++i) {
        buffer.append(std::to_string(sensorData.data[i])).append(",");
    }
    buffer.append("\n");
    return buffer;
}

int32_t MedicalSensorDump::Dump(int32_t fd, const std::vector<std::u16string> &args,
    std::vector<MedicalSensor> &sensors, ClientInfo &clientInfo)
{
    HiLog::Info(LABEL, "%{public}s begin", __func__);

    if ((args.empty()) || (args[0].size() != MAX_DMUP_PARAM)) {
        HiLog::Error(LABEL, "%{public}s param cannot be empty or the length is not 2", __func__);
        dprintf(fd, "cmd param number is not equal to 2\n");
        DumpHelp(fd);
        return DUMP_PARAM_ERR;
    }
    bool helpRet = DumpSensorHelp(fd, args);
    bool listRet = DumpSensorList(fd, sensors, args);
    bool channelRet = DumpSensorChannel(fd, clientInfo, args);
    bool openRet = DumpOpeningSensor(fd, sensors, clientInfo, args);
    bool dataRet = DumpSensorData(fd, clientInfo, args);
    bool total = helpRet + listRet + channelRet + openRet + dataRet;
    if (!total) {
        dprintf(fd, "cmd param is error\n");
        DumpHelp(fd);
        return DUMP_PARAM_ERR;
    }
    HiLog::Info(LABEL, "%{public}s end", __func__);
    return ERR_OK;
}
}  // namespace Sensors
}  // namespace OHOS
