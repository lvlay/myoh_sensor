/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef IMAGE_AUTO_INITER
#define IMAGE_AUTO_INITER
#include "v2_0/icodec_image.h"
namespace OHOS {
namespace Codec {
namespace Image {
class ImageAutoIniter {
public:
    ImageAutoIniter(OHOS::sptr<OHOS::HDI::Codec::Image::V2_0::ICodecImage> imageClient,
        OHOS::HDI::Codec::Image::V2_0::CodecImageRole role) : client_(imageClient), role_(role)
    {
        if (client_) {
            client_->Init(role_);
        }
    }

    ~ImageAutoIniter()
    {
        if (client_) {
            client_->DeInit(role_);
            client_ = nullptr;
        }
    }

private:
    OHOS::sptr<OHOS::HDI::Codec::Image::V2_0::ICodecImage> client_;
    OHOS::HDI::Codec::Image::V2_0::CodecImageRole role_;
};
}  // namespace Image
}  // namespace Codec
}  // namespace OHOS

#endif