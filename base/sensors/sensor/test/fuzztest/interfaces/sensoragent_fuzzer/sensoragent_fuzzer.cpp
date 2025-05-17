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

#include "sensoragent_fuzzer.h"

#include "sensor_agent.h"
#include "sensor_agent_type.h"
#include <unistd.h>
#include <thread>

void SensorDataCallbackImpl(SensorEvent *event)
{
    if (event == nullptr) {
        return;
    }
}

bool CheckSensorTypeId(int32_t sensorTypeId)
{
    int32_t count = -1;
    SensorInfo *sensorInfo = nullptr;
    int32_t ret = GetAllSensors(&sensorInfo, &count);
    if (ret != 0) {
        return false;
    }
    for (int32_t i = 0; i < count; i++) {
        if ((sensorInfo + i)->sensorTypeId == sensorTypeId) {
            return true;
        }
    }
    return false;
}

bool SensorAgentFuzzTest(const uint8_t *data, size_t size)
{
    intptr_t sensorTypeId = reinterpret_cast<intptr_t>(data);
    bool validSensorId = CheckSensorTypeId(sensorTypeId);
    SensorUser user;
    user.callback = SensorDataCallbackImpl;
    int32_t ret = SubscribeSensor(sensorTypeId, &user);
    if (ret != 0) {
        return validSensorId ? false : true;
    }
    ret = SetBatch(sensorTypeId, &user, 200000000, 0);
    if (ret != 0) {
        return validSensorId ? false : true;
    }
    ret = ActivateSensor(sensorTypeId, &user);
    if (ret != 0) {
        return validSensorId ? false : true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    ret = DeactivateSensor(sensorTypeId, &user);
    if (ret != 0) {
        return validSensorId ? false : true;
    }
    ret = UnsubscribeSensor(sensorTypeId, &user);
    if (ret != 0) {
        return validSensorId ? false : true;
    }
    return true;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    SensorAgentFuzzTest(data, size);
    return 0;
}

