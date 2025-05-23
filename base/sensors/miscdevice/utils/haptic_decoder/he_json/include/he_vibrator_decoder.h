/*
 * Copyright (c) 2023 Shanghai Ruisheng Kaitai Acoustic Science Co., Ltd.
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

#ifndef HE_VIBRATOR_DECODER_H
#define HE_VIBRATOR_DECODER_H

#include <cstdint>
#include <vector>

#include "i_vibrator_decoder.h"
#include "json_parser.h"

namespace OHOS {
namespace Sensors {
class HEVibratorDecoder : public IVibratorDecoder {
public:
    HEVibratorDecoder() = default;
    ~HEVibratorDecoder() = default;
    int32_t DecodeEffect(const RawFileDescriptor &rawFd, const JsonParser &parser,
        VibratePackage &patternPackage) override;
private:
    int32_t ParseVersion(const JsonParser &parser);
    int32_t ParsePatternList(const JsonParser& parser, cJSON* patternListJSON, VibratePackage& pkg);
    int32_t ParsePattern(const JsonParser &parser, cJSON *patternJSON, VibratePattern &pattern);
    int32_t ParseEvent(const JsonParser &parser, cJSON *eventJSON, VibrateEvent &event);
    int32_t ParseCurve(const JsonParser &parser, cJSON *curveJSON, VibrateEvent &event);
    bool CheckEventParameters(const VibrateEvent& event);
    bool CheckCommonParameters(const VibrateEvent& event);
    bool CheckTransientParameters(const VibrateEvent& event);
    bool CheckContinuousParameters(const VibrateEvent& event);
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // HE_VIBRATOR_DECODER_H