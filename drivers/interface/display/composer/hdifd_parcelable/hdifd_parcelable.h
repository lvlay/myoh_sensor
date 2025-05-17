/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_HDI_HDIFD_PARCELABLE_H
#define OHOS_HDI_HDIFD_PARCELABLE_H

#include <string>
#include <vector>
#include <parcel.h>

namespace OHOS {
namespace HDI {
namespace Display {
using namespace OHOS;

class HdifdParcelable : public Parcelable {
public:
    HdifdParcelable();
    explicit HdifdParcelable(int32_t fd);
    virtual ~HdifdParcelable();

    bool Init(int32_t fd);
    bool Marshalling(Parcel& parcel) const override;
    static sptr<HdifdParcelable> Unmarshalling(Parcel& parcel);
    int32_t Move();
    int32_t GetFd();
    std::string Dump() const;

private:
    static bool WriteFileDescriptor(const int fd, Parcel& parcel);
    static int ReadFileDescriptor(Parcel& parcel);

private:
    bool isOwner_;
    int32_t hdiFd_;
};
} // Display
} // HDI
} // OHOS
#endif // OHOS_HDI_HDIFD_PARCELABLE_H
