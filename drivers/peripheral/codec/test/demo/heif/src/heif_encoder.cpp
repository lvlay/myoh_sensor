/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "codec_heif_helper.h"

int main(int argc, char *argv[])
{
    OHOS::VDI::HEIF::CommandOpt opt = OHOS::VDI::HEIF::Parse(argc, argv);
    opt.Print();
    OHOS::VDI::HEIF::HeifEncoderHelper obj(opt);
    obj.DoEncode();
    return 0;
}