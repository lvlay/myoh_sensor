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

#ifndef SENSORS_LOG_DOMAIN_H
#define SENSORS_LOG_DOMAIN_H

#include "hilog/log.h"

namespace OHOS {
namespace MedicalSensorLogDomain {
constexpr uint32_t COMMON = 0xD002700;
constexpr uint32_t MEDICAL_SENSOR_FRAMEWORK = 0xD002781;
constexpr uint32_t MEDICAL_SENSOR_SERVICE = 0xD002782;
constexpr uint32_t MEDICAL_SENSOR_UTILS = 0xD002783;
constexpr uint32_t MEDICAL_SENSOR_TEST = 0xD002784;
constexpr uint32_t MEDICAL_SENSOR_NATIVE = 0xD002785;
constexpr uint32_t MEDICAL_SENSOR_JSNAPI = 0xD002786;
constexpr uint32_t MEDICAL_SENSOR_HDI_ADAPTOR = 0xD002787;
constexpr uint32_t MEDICAL_SENSOR_HDI_HARDWARE = 0xD002788;
constexpr uint32_t MEDICAL_SENSOR_HDI_INTERFACE = 0xD002789;
}  // namespace MedicalSensorLogDomain
}  // namespace OHOS
#endif  // SENSORS_LOG_DOMAIN_H
