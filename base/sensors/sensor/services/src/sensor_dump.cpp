/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "sensor_dump.h"

#include <getopt.h>

#include <cinttypes>
#include <cstring>
#include <ctime>
#include <queue>

#include "securec.h"
#include "sensor_agent_type.h"
#include "sensor_errors.h"

#undef LOG_TAG
#define LOG_TAG "SensorDump"

namespace OHOS {
namespace Sensors {
using namespace OHOS::HiviewDFX;
namespace {
constexpr int32_t MAX_DUMP_PARAMETERS = 32;
#ifdef BUILD_VARIANT_ENG
constexpr uint32_t MAX_DUMP_DATA_SIZE = 10;
#endif // BUILD_VARIANT_ENG
constexpr uint32_t MS_NS = 1000000;

enum {
    SOLITARIES_DIMENSION = 1,
    TWO_DIMENSION = 2,
    COMMON_DIMENSION = 3,
    VECTOR_DIMENSION = 4,
    UNCALIBRATED_DIMENSION = 6,
    SEVEN_DIMENSION = 7,
    POSE_6DOF_DIMENSION = 15,
    DEFAULT_DIMENSION = 16,
};
} // namespace

std::unordered_map<int32_t, std::string> SensorDump::sensorMap_ = {
    { SENSOR_TYPE_ID_ACCELEROMETER, "ACCELEROMETER" },
    { SENSOR_TYPE_ID_ACCELEROMETER_UNCALIBRATED, "ACCELEROMETER UNCALIBRATED" },
    { SENSOR_TYPE_ID_LINEAR_ACCELERATION, "LINEAR ACCELERATION" },
    { SENSOR_TYPE_ID_GRAVITY, "GRAVITY" },
    { SENSOR_TYPE_ID_GYROSCOPE, "GYROSCOPE" },
    { SENSOR_TYPE_ID_CAPACITIVE, "CAPACITIVE"},
    { SENSOR_TYPE_ID_TEMPERATURE, "TEMPERATURE"},
    { SENSOR_TYPE_ID_GESTURE, "GESTURE"},
    { SENSOR_TYPE_ID_GYROSCOPE_UNCALIBRATED, "GYROSCOPE UNCALIBRATED" },
    { SENSOR_TYPE_ID_SIGNIFICANT_MOTION, "SIGNIFICANT MOTION" },
    { SENSOR_TYPE_ID_PEDOMETER_DETECTION, "PEDOMETER DETECTION" },
    { SENSOR_TYPE_ID_PEDOMETER, "PEDOMETER" },
    { SENSOR_TYPE_ID_AMBIENT_TEMPERATURE, "AMBIENT TEMPERATURE" },
    { SENSOR_TYPE_ID_MAGNETIC_FIELD, "MAGNETIC FIELD" },
    { SENSOR_TYPE_ID_MAGNETIC_FIELD_UNCALIBRATED, "MAGNETIC FIELD UNCALIBRATED" },
    { SENSOR_TYPE_ID_HUMIDITY, "HUMIDITY" },
    { SENSOR_TYPE_ID_BAROMETER, "BAROMETER" },
    { SENSOR_TYPE_ID_DEVICE_ORIENTATION, "DEVICE ORIENTATION" },
    { SENSOR_TYPE_ID_ORIENTATION, "ORIENTATION" },
    { SENSOR_TYPE_ID_ROTATION_VECTOR, "ROTATION VECTOR" },
    { SENSOR_TYPE_ID_GAME_ROTATION_VECTOR, "GAME ROTATION VECTOR" },
    { SENSOR_TYPE_ID_GEOMAGNETIC_ROTATION_VECTOR, "GEOMAGNETIC ROTATION VECTOR" },
    { SENSOR_TYPE_ID_PROXIMITY, "PROXIMITY" },
    { SENSOR_TYPE_ID_PROXIMITY1, "SECONDARY PROXIMITY" },
    { SENSOR_TYPE_ID_AMBIENT_LIGHT, "AMBIENT LIGHT" },
    { SENSOR_TYPE_ID_AMBIENT_LIGHT1, "SECONDARY AMBIENT LIGHT" },
    { SENSOR_TYPE_ID_HALL, "HALL" },
    { SENSOR_TYPE_ID_HALL_EXT, "EXTENDED HALL" },
    { SENSOR_TYPE_ID_HEART_RATE, "HEART RATE" },
    { SENSOR_TYPE_ID_WEAR_DETECTION, "WEAR DETECTION" },
    { SENSOR_TYPE_ID_COLOR, "COLOR" },
    { SENSOR_TYPE_ID_SAR, "SAR" },
    { SENSOR_TYPE_ID_POSTURE, "POSTURE" },
    { SENSOR_TYPE_ID_HEADPOSTURE, "HEAD POSTURE" },
    { SENSOR_TYPE_ID_DROP_DETECTION, "DROP DETECTION" },
    { SENSOR_TYPE_ID_RPC, "RPC" },
};

void SensorDump::RunSensorDump(int32_t fd, int32_t optionIndex, const std::vector<std::string> &args, char **argv)
{
    struct option dumpOptions[] = {
        {"channel", no_argument, 0, 'c'},
#ifdef BUILD_VARIANT_ENG
        {"data", no_argument, 0, 'd'},
#endif // BUILD_VARIANT_ENG
        {"open", no_argument, 0, 'o'},
        {"help", no_argument, 0, 'h'},
        {"list", no_argument, 0, 'l'},
        {NULL, 0, 0, 0}
    };
    optind = 1;
    int32_t c;
    while ((c = getopt_long(args.size(), argv, "cdohl", dumpOptions, &optionIndex)) != -1) {
        switch (c) {
            case 'c': {
                DumpSensorChannel(fd, clientInfo_);
                break;
            }
#ifdef BUILD_VARIANT_ENG
            case 'd': {
                DumpSensorData(fd, clientInfo_);
                break;
            }
#endif // BUILD_VARIANT_ENG
            case 'o': {
                DumpOpeningSensor(fd, sensors_, clientInfo_);
                break;
            }
            case 'h': {
                DumpHelp(fd);
                break;
            }
            case 'l': {
                DumpSensorList(fd, sensors_);
                break;
            }
            default: {
                dprintf(fd, "Unrecognized option, More info with: \"hidumper -s 3601 -a -h\"\n");
                break;
            }
        }
    }
}

void SensorDump::ParseCommand(int32_t fd, const std::vector<std::string> &args, const std::vector<Sensor> &sensors,
    ClientInfo &clientInfo)
{
    int32_t count = 0;
    for (const auto &str : args) {
        if (str.find("--") == 0) {
            ++count;
            continue;
        }
        if (str.find("-") == 0) {
            count += static_cast<int32_t>(str.size()) - 1;
            continue;
        }
    }
    if (count > MAX_DUMP_PARAMETERS) {
        SEN_HILOGE("Cmd param number not more than 32");
        dprintf(fd, "Cmd param number not more than 32\n");
        return;
    }
    int32_t optionIndex = 0;
    char **argv = new (std::nothrow) char *[args.size()];
    CHKPV(argv);
    if (memset_s(argv, args.size() * sizeof(char *), 0, args.size() * sizeof(char *)) != EOK) {
        SEN_HILOGE("memset_s failed");
        delete[] argv;
        return;
    }
    for (size_t i = 0; i < args.size(); ++i) {
        argv[i] = new (std::nothrow) char[args[i].size() + 1];
        if (argv[i] == nullptr) {
            SEN_HILOGE("Alloc failure");
            goto RELEASE_RES;
        }
        if (strcpy_s(argv[i], args[i].size() + 1, args[i].c_str()) != EOK) {
            SEN_HILOGE("strcpy_s error");
            goto RELEASE_RES;
        }
    }
    sensors_ = sensors;
    RunSensorDump(fd, optionIndex, args, argv);
    RELEASE_RES:
    for (size_t i = 0; i < args.size(); ++i) {
        if (argv[i] != nullptr) {
            delete[] argv[i];
        }
    }
    delete[] argv;
}

void SensorDump::DumpHelp(int32_t fd)
{
    dprintf(fd, "Usage:\n");
    dprintf(fd, "      -h, --help: dump help\n");
    dprintf(fd, "      -l, --list: dump the sensor list\n");
    dprintf(fd, "      -c, --channel: dump the sensor data channel info\n");
    dprintf(fd, "      -o, --open: dump the opening sensors\n");
#ifdef BUILD_VARIANT_ENG 
    dprintf(fd, "      -d, --data: dump the last 10 packages sensor data\n");
#endif // BUILD_VARIANT_ENG
}

bool SensorDump::DumpSensorList(int32_t fd, const std::vector<Sensor> &sensors)
{
    DumpCurrentTime(fd);
    dprintf(fd, "Total sensor:%d, Sensor list:\n", int32_t { sensors.size() });
    for (const auto &sensor : sensors) {
        auto sensorId = sensor.GetSensorId();
        if (sensorMap_.find(sensorId) == sensorMap_.end()) {
            continue;
        }
        dprintf(fd,
                "sensorId:%8u | sensorType:%s | sensorName:%s | vendorName:%s | maxRange:%f"
                "| fifoMaxEventCount:%d | minSamplePeriodNs:%" PRId64 " | maxSamplePeriodNs:%" PRId64 "\n",
                sensorId, sensorMap_[sensorId].c_str(), sensor.GetSensorName().c_str(), sensor.GetVendorName().c_str(),
                sensor.GetMaxRange(), sensor.GetFifoMaxEventCount(), sensor.GetMinSamplePeriodNs(),
                sensor.GetMaxSamplePeriodNs());
    }
    return true;
}

bool SensorDump::DumpSensorChannel(int32_t fd, ClientInfo &clientInfo)
{
    DumpCurrentTime(fd);
    dprintf(fd, "Sensor channel info:\n");
    std::vector<SensorChannelInfo> channelInfo;
    clientInfo.GetSensorChannelInfo(channelInfo);
    for (const auto &channel : channelInfo) {
        auto sensorId = channel.GetSensorId();
        if (sensorMap_.find(sensorId) == sensorMap_.end()) {
            continue;
        }
        dprintf(fd,
                "uid:%d | packageName:%s | sensorId:%8u | sensorType:%s | samplingPeriodNs:%" PRId64 ""
                "| fifoCount:%u\n",
                channel.GetUid(), channel.GetPackageName().c_str(), sensorId, sensorMap_[sensorId].c_str(),
                channel.GetSamplingPeriodNs(), channel.GetFifoCount());
    }
    return true;
}

bool SensorDump::DumpOpeningSensor(int32_t fd, const std::vector<Sensor> &sensors, ClientInfo &clientInfo)
{
    DumpCurrentTime(fd);
    dprintf(fd, "Opening sensors:\n");
    for (const auto &sensor : sensors) {
        int32_t sensorId = sensor.GetSensorId();
        if (sensorMap_.find(sensorId) == sensorMap_.end()) {
            continue;
        }
        if (clientInfo.GetSensorState(sensorId)) {
            dprintf(fd, "sensorId: %8u | sensorType: %s | channelSize: %lu\n",
                sensorId, sensorMap_[sensorId].c_str(), clientInfo.GetSensorChannel(sensorId).size());
        }
    }
    return true;
}

#ifdef BUILD_VARIANT_ENG 
bool SensorDump::DumpSensorData(int32_t fd, ClientInfo &clientInfo)
{
    dprintf(fd, "Last 10 packages sensor data:\n");
    auto dataMap = clientInfo.GetDumpQueue();
    int32_t j = 0;
    for (auto &sensorData : dataMap) {
        int32_t sensorId = sensorData.first;
        if (sensorMap_.find(sensorId) == sensorMap_.end()) {
            continue;
        }
        dprintf(fd, "sensorId: %8u | sensorType: %s:\n", sensorId, sensorMap_[sensorId].c_str());
        for (uint32_t i = 0; i < MAX_DUMP_DATA_SIZE && (!sensorData.second.empty()); i++) {
            auto data = sensorData.second.front();
            sensorData.second.pop();
            timespec time = { 0, 0 };
            clock_gettime(CLOCK_REALTIME, &time);
            struct tm *timeinfo = localtime(&(time.tv_sec));
            CHKPF(timeinfo);
            dprintf(fd, "      %2d (ts=%.9f, time=%02d:%02d:%02d.%03d) | data:%s", ++j, data.timestamp / 1e9,
                    timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, int32_t { (time.tv_nsec / MS_NS) },
                    GetDataBySensorId(sensorId, data).c_str());
        }
    }
    return true;
}
#endif // BUILD_VARIANT_ENG

void SensorDump::DumpCurrentTime(int32_t fd)
{
    timespec curTime = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &curTime);
    struct tm *timeinfo = localtime(&(curTime.tv_sec));
    CHKPV(timeinfo);
    dprintf(fd, "Current time: %02d:%02d:%02d.%03d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
            int32_t { (curTime.tv_nsec / MS_NS) });
}

int32_t SensorDump::GetDataDimension(int32_t sensorId)
{
    switch (sensorId) {
        case SENSOR_TYPE_ID_BAROMETER:
        case SENSOR_TYPE_ID_HALL:
        case SENSOR_TYPE_ID_HALL_EXT:
        case SENSOR_TYPE_ID_TEMPERATURE:
        case SENSOR_TYPE_ID_PROXIMITY:
        case SENSOR_TYPE_ID_PROXIMITY1:
        case SENSOR_TYPE_ID_HUMIDITY:
        case SENSOR_TYPE_ID_AMBIENT_TEMPERATURE:
        case SENSOR_TYPE_ID_SIGNIFICANT_MOTION:
        case SENSOR_TYPE_ID_PEDOMETER_DETECTION:
        case SENSOR_TYPE_ID_PEDOMETER:
        case SENSOR_TYPE_ID_HEART_RATE:
        case SENSOR_TYPE_ID_WEAR_DETECTION:
        case SENSOR_TYPE_ID_SAR:
            return SOLITARIES_DIMENSION;
        case SENSOR_TYPE_ID_COLOR:
            return TWO_DIMENSION;
        case SENSOR_TYPE_ID_ROTATION_VECTOR:
        case SENSOR_TYPE_ID_HEADPOSTURE:
            return VECTOR_DIMENSION;
        case SENSOR_TYPE_ID_MAGNETIC_FIELD_UNCALIBRATED:
        case SENSOR_TYPE_ID_GYROSCOPE_UNCALIBRATED:
        case SENSOR_TYPE_ID_ACCELEROMETER_UNCALIBRATED:
            return UNCALIBRATED_DIMENSION;
        case SENSOR_TYPE_ID_POSTURE:
            return SEVEN_DIMENSION;
        default:
            SEN_HILOGW("Unknown sensorId:%{public}d, size:%{public}d", sensorId, COMMON_DIMENSION);
            return COMMON_DIMENSION;
    }
}

std::string SensorDump::GetDataBySensorId(int32_t sensorId, SensorData &sensorData)
{
    SEN_HILOGD("sensorId:%{public}u", sensorId);
    std::string str;
    int32_t dataLen = GetDataDimension(sensorId);
    if (sensorData.dataLen < sizeof(float)) {
        SEN_HILOGE("SensorData dataLen less than float size.");
        return str;
    }
    auto data = reinterpret_cast<float *>(sensorData.data);
    for (int32_t i = 0; i < dataLen; ++i) {
        str.append(std::to_string(*data));
        if (i != dataLen - 1) {
            str.append(",");
        }
        ++data;
    }
    str.append("\n");
    return str;
}
} // namespace Sensors
} // namespace OHOS
