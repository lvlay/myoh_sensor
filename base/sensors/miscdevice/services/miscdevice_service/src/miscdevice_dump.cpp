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

#include "miscdevice_dump.h"

#include <getopt.h>

#include <cinttypes>
#include <cstring>
#include <ctime>
#include <map>

#include "securec.h"
#include "sensors_errors.h"

#undef LOG_TAG
#define LOG_TAG "MiscdeviceDump"

namespace OHOS {
namespace Sensors {
namespace {
constexpr uint32_t MAX_DUMP_RECORD_SIZE = 30;
constexpr uint32_t BASE_YEAR = 1900;
constexpr uint32_t BASE_MON = 1;
constexpr int32_t MAX_DUMP_PARAMETERS = 32;
constexpr int32_t CONVERSION_RATE = 1000;
}  // namespace

static std::map<int32_t, std::string> usageMap_ = {
    {USAGE_UNKNOWN, "unknown"},
    {USAGE_ALARM, "alarm"},
    {USAGE_RING, "ring"},
    {USAGE_NOTIFICATION, "notification"},
    {USAGE_COMMUNICATION, "communication"},
    {USAGE_TOUCH, "touch"},
    {USAGE_MEDIA, "media"},
    {USAGE_PHYSICAL_FEEDBACK, "physicalFeedback"},
    {USAGE_SIMULATE_REALITY, "simulateReality"},
};

MiscdeviceDump::MiscdeviceDump() {}

MiscdeviceDump::~MiscdeviceDump() {}

void MiscdeviceDump::ParseCommand(int32_t fd, const std::vector<std::string> &args)
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
        MISC_HILOGE("Cmd param number not more than 32");
        dprintf(fd, "Cmd param number not more than 32\n");
        return;
    }
    int32_t optionIndex = 0;
    char **argv = new (std::nothrow) char *[args.size()];
    CHKPV(argv);
    if (memset_s(argv, args.size() * sizeof(char *), 0, args.size() * sizeof(char *)) != EOK) {
        MISC_HILOGE("Call memset_s failed");
        delete[] argv;
        return;
    }
    for (size_t i = 0; i < args.size(); ++i) {
        argv[i] = new (std::nothrow) char[args[i].size() + 1];
        if (argv[i] == nullptr) {
            MISC_HILOGE("Alloc failure");
            goto RELEASE_RES;
        }
        if (strcpy_s(argv[i], args[i].size() + 1, args[i].c_str()) != EOK) {
            MISC_HILOGE("strcpy_s error");
            goto RELEASE_RES;
        }
    }
    RunVibratorDump(fd, optionIndex, args, argv);
    RELEASE_RES:
    for (size_t i = 0; i < args.size(); ++i) {
        if (argv[i] != nullptr) {
            delete[] argv[i];
        }
    }
    delete[] argv;
}

void MiscdeviceDump::RunVibratorDump(int32_t fd, int32_t optionIndex, const std::vector<std::string> &args, char **argv)
{
    struct option dumpOptions[] = {
        {"record", no_argument, 0, 'r'},
        {"help", no_argument, 0, 'h'},
        {NULL, 0, 0, 0}
    };
    optind = 1;
    int32_t c;
    while ((c = getopt_long(args.size(), argv, "rh", dumpOptions, &optionIndex)) != -1) {
        switch (c) {
            case 'r': {
                DumpMiscdeviceRecord(fd);
                break;
            }
            case 'h': {
                DumpHelp(fd);
                break;
            }
            default: {
                dprintf(fd, "Unrecognized option, More info with: \"hidumper -s 3602 -a -h\"\n");
                break;
            }
        }
    }
}

void MiscdeviceDump::DumpHelp(int32_t fd)
{
    dprintf(fd, "Usage:\n");
    dprintf(fd, "      -h, --help: dump help\n");
    dprintf(fd, "      -r, --record: dump the list of vibrate recorded\n");
}

void MiscdeviceDump::DumpMiscdeviceRecord(int32_t fd)
{
    std::lock_guard<std::mutex> queueLock(recordQueueMutex_);
    if (dumpQueue_.empty()) {
        MISC_HILOGW("dumpQueue_ is empty");
        return;
    }
    size_t length = dumpQueue_.size() > MAX_DUMP_RECORD_SIZE ? MAX_DUMP_RECORD_SIZE : dumpQueue_.size();
    for (size_t i = 0; i < length; ++i) {
        auto record = dumpQueue_.front();
        dumpQueue_.push(record);
        dumpQueue_.pop();
        VibrateInfo info = record.info;
        if (info.mode == "time") {
            dprintf(fd, "startTime:%s | uid:%d | pid:%d | packageName:%s | duration:%d | usage:%s\n",
                record.startTime.c_str(), info.uid, info.pid, info.packageName.c_str(),
                info.duration, GetUsageName(info.usage).c_str());
        } else if (info.mode == "preset") {
            dprintf(fd, "startTime:%s | uid:%d | pid:%d | packageName:%s | effect:%s | count:%d | usage:%s\n",
                record.startTime.c_str(), info.uid, info.pid, info.packageName.c_str(),
                info.effect.c_str(), info.count, GetUsageName(info.usage).c_str());
        } else {
            dprintf(fd, "startTime:%s | uid:%d | pid:%d | packageName:%s | usage:%s\n",
                record.startTime.c_str(), info.uid, info.pid, info.packageName.c_str(),
                GetUsageName(info.usage).c_str());
        }
    }
}

void MiscdeviceDump::DumpCurrentTime(std::string &startTime)
{
    timespec curTime;
    clock_gettime(CLOCK_REALTIME, &curTime);
    struct tm *timeinfo = localtime(&(curTime.tv_sec));
    CHKPV(timeinfo);
    startTime.append(std::to_string(timeinfo->tm_year + BASE_YEAR)).append("-")
        .append(std::to_string(timeinfo->tm_mon + BASE_MON)).append("-").append(std::to_string(timeinfo->tm_mday))
        .append(" ").append(std::to_string(timeinfo->tm_hour)).append(":").append(std::to_string(timeinfo->tm_min))
        .append(":").append(std::to_string(timeinfo->tm_sec)).append(".")
        .append(std::to_string(curTime.tv_nsec / (CONVERSION_RATE * CONVERSION_RATE)));
}

void MiscdeviceDump::UpdateRecordQueue(const VibrateRecord &record)
{
    std::lock_guard<std::mutex> queueLock(recordQueueMutex_);
    dumpQueue_.push(record);
    if (dumpQueue_.size() > MAX_DUMP_RECORD_SIZE) {
        dumpQueue_.pop();
    }
}

void MiscdeviceDump::SaveVibrateRecord(const VibrateInfo &vibrateInfo)
{
    VibrateRecord record;
    record.info = vibrateInfo;
    DumpCurrentTime(record.startTime);
    UpdateRecordQueue(record);
}

std::string MiscdeviceDump::GetUsageName(int32_t usage)
{
    auto it = usageMap_.find(usage);
    if (it == usageMap_.end()) {
        MISC_HILOGE("Usage:%{public}d is invalid", usage);
        return {};
    }
    return it->second;
}
}  // namespace Sensors
}  // namespace OHOS
