/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "system_info.h"

#include <dirent.h>
#include <unistd.h>

#include <fstream>
#include <sstream>
#include <string>
#include <thread>

#include "securec.h"
#include "sensor_errors.h"

#undef LOG_TAG
#define LOG_TAG "SYSTEM_INFO"

namespace OHOS {
namespace Sensors {
namespace SYSTEM_INFO {
using namespace OHOS::HiviewDFX;
namespace {
constexpr int32_t LOCATION = 14;
constexpr int32_t TIME_WAIT_FOR_OP = 1000;
constexpr int32_t DEFAULT_PID = -1;
} // namespace

inline double CHK_RATE(double rate)
{
    return (rate > CPU_USAGE_MAX ? CPU_USAGE_MAX : rate);
}

int32_t CpuInfo::GetTaskPidFile(const std::string &process_name)
{
    int32_t pid = DEFAULT_PID;
    static const std::string procPath = "/proc";
    DIR *dir = ::opendir(procPath.c_str());
    if (dir == nullptr) {
        SEN_HILOGE("Failed to open path:%{public}s", procPath.c_str());
        return DEFAULT_PID;
    }
    struct dirent *pidFile;
    while ((pidFile = ::readdir(dir)) != nullptr) {
        if ((::strcmp(pidFile->d_name, ".") == 0) || (::strcmp(pidFile->d_name, "..") == 0)) {
            continue;
        }
        if (pidFile->d_type != DT_DIR) {
            continue;
        }
        const std::string path = procPath + "/" + pidFile->d_name + "/status";
        std::ifstream filePath(path);
        if (!filePath.is_open()) {
            continue;
        }
        std::string strLine;
        if (!std::getline(filePath, strLine)) {
            SEN_HILOGE("getline fail");
            filePath.close();
            continue;
        }
        if (strLine.empty()) {
            filePath.close();
            continue;
        }
        if ((strLine.find(process_name)) == std::string::npos) {
            filePath.close();
            continue;
        }
        while (std::getline(filePath, strLine)) {
            if ((strLine.find("Pid")) != std::string::npos) {
                if (::sscanf_s(strLine.c_str(), "%*s%d", &pid, sizeof(pid)) != 1) {
                    SEN_HILOGE("sscanf_s failed");
                }
                break;
            }
        }
        filePath.close();
        break;
    }
    ::closedir(dir);

    return pid;
}

int32_t CpuInfo::GetProcOccupy(int32_t pid)
{
    Proc_Cpu_Occupy info;
    static const std::string procPath = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream file(procPath);
    if (!file.is_open()) {
        SEN_HILOGE("Failed to open path:%{public}s", procPath.c_str());
        return OHOS::Sensors::ERROR;
    }

    std::string strLine;
    if (!std::getline(file, strLine)) {
        SEN_HILOGE("getline fail");
        file.close();
        return OHOS::Sensors::ERROR;
    }
    if (strLine.empty()) {
        SEN_HILOGE("Failed to read content");
        file.close();
        return OHOS::Sensors::ERROR;
    }
    file.close();

    int pos = 1;
    std::istringstream ss(strLine);
    while (ss >> strLine) {
        pos++;
        if (pos >= LOCATION) {
            break;
        }
    }
    ss >> info.utime >> info.stime >> info.cutime >> info.cstime;
    return (info.utime + info.stime + info.cutime + info.cstime);
}

double CpuInfo::GetCpuUsage(const Total_Cpu_Occupy &first, const Total_Cpu_Occupy &second)
{
    unsigned long cpuTime2 = static_cast<unsigned long>(second.user + second.nice + second.system +
                                                        second.idle + second.lowait + second.irq + second.softirq);
    unsigned long cpuTime1 = static_cast<unsigned long>(first.user + first.nice + first.system +
                                                        first.idle + first.lowait + first.irq + first.softirq);

    double cpu_use = (second.user - first.user) * CPU_USAGE_MAX / (cpuTime2 - cpuTime1);
    double cpu_sys = (second.system - first.system) * CPU_USAGE_MAX / (cpuTime2 - cpuTime1);

    return CHK_RATE(cpu_use + cpu_sys);
}

int32_t CpuInfo::GetSystemCpuStatInfo(Total_Cpu_Occupy &info)
{
    std::ifstream statFile("/proc/stat");
    if (!statFile.is_open()) {
        SEN_HILOGE("Failed to open config file");
        return FILE_OPEN_FAIL;
    }
    std::string strLine;
    if (!std::getline(statFile, strLine)) {
        SEN_HILOGE("getline fail");
        statFile.close();
        return STREAM_BUF_READ_FAIL;
    }
    if (strLine.empty()) {
        SEN_HILOGE("No valid content was read");
        statFile.close();
        return STREAM_BUF_READ_FAIL;
    }
    if ((strLine.find("cpu")) == std::string::npos) {
        SEN_HILOGE("The keyword was not matched. Procedure");
        statFile.close();
        return OHOS::Sensors::ERROR;
    }
    std::istringstream ss(strLine);
    ss >> info.name >> info.user >> info.nice >> info.system >> info.idle >> info.lowait \
        >> info.irq >> info.softirq >> info.steal >> info.guest >> info.guest_nice;

    statFile.close();
    return OHOS::Sensors::SUCCESS;
}

double CpuInfo::GetSystemCpuUsage()
{
    Total_Cpu_Occupy first {};
    int32_t ret = GetSystemCpuStatInfo(first);
    if (ret != OHOS::Sensors::SUCCESS) {
        SEN_HILOGE("Failed to obtain CPU information, errcode:%{public}d", ret);
        return CPU_USAGE_UNKNOWN;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP));
    Total_Cpu_Occupy second {};
    ret = GetSystemCpuStatInfo(second);
    if (ret != OHOS::Sensors::SUCCESS) {
        SEN_HILOGE("Failed to obtain CPU information, errcode:%{public}d", ret);
        return CPU_USAGE_UNKNOWN;
    }

    return GetCpuUsage(first, second);
}

int64_t CpuInfo::GetSystemTotalOccupy()
{
    int ret = -1;
    Total_Cpu_Occupy occupy {};
    if ((ret = GetSystemCpuStatInfo(occupy)) != OHOS::Sensors::SUCCESS) {
        SEN_HILOGE("Failed to obtain CPU information, errcode:%{public}d", ret);
        return OHOS::Sensors::ERROR;
    }
    return (occupy.user + occupy.nice + occupy.system + occupy.idle);
}

double CpuInfo::GetProcCpuUsage(const std::string &process_name)
{
    int64_t totalTime1 = 0;
    int64_t totalTime2 = 0;
    int64_t procTime1 = 0;
    int64_t procTime2 = 0;
    int32_t pid = GetTaskPidFile(process_name);

    if ((totalTime1 = GetSystemTotalOccupy()) == OHOS::Sensors::ERROR) {
        SEN_HILOGE("Failed to obtain CPU occupy");
        return CPU_USAGE_UNKNOWN;
    }
    if ((procTime1 = GetProcOccupy(pid)) == OHOS::Sensors::ERROR) {
        SEN_HILOGE("Failed to obtain process CPU information");
        return CPU_USAGE_UNKNOWN;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP));

    if ((totalTime2 = GetSystemTotalOccupy()) == OHOS::Sensors::ERROR) {
        SEN_HILOGE("Failed to obtain CPU occupy");
        return CPU_USAGE_UNKNOWN;
    }
    if ((procTime2 = GetProcOccupy(pid)) == OHOS::Sensors::ERROR) {
        SEN_HILOGE("Failed to obtain process CPU information");
        return CPU_USAGE_UNKNOWN;
    }

    return CHK_RATE(CPU_USAGE_MAX * (procTime2 - procTime1) / (totalTime2 - totalTime1));
}
} // namespace SYSTEM_INFO
} // namespace Sensors
} // namespace OHOS