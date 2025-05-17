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

#ifndef MISCDEVICE_DUMP_H
#define MISCDEVICE_DUMP_H

#include <queue>

#include "singleton.h"

#include "nocopyable.h"
#include "vibrator_infos.h"

namespace OHOS {
namespace Sensors {
struct VibrateRecord {
    std::string startTime;
    VibrateInfo info;
};

class MiscdeviceDump {
    DECLARE_DELAYED_SINGLETON(MiscdeviceDump);
public:
    DISALLOW_COPY_AND_MOVE(MiscdeviceDump);
    void DumpHelp(int32_t fd);
    void DumpMiscdeviceRecord(int32_t fd);
    void ParseCommand(int32_t fd, const std::vector<std::string> &args);
    void SaveVibrateRecord(const VibrateInfo &vibrateInfo);

private:
    std::queue<VibrateRecord> dumpQueue_;
    std::mutex recordQueueMutex_;
    void DumpCurrentTime(std::string &startTime);
    void UpdateRecordQueue(const VibrateRecord &record);
    std::string GetUsageName(int32_t usage);
    void RunVibratorDump(int32_t fd, int32_t optionIndex, const std::vector<std::string> &args, char **argv);
};
#define DumpHelper DelayedSingleton<MiscdeviceDump>::GetInstance()
}  // namespace Sensors
}  // namespace OHOS
#endif  // MISCDEVICE_DUMP_H
