/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_CAMERA_METADATA_UTILS_H
#define OHOS_CAMERA_METADATA_UTILS_H

#include <iostream>
#include <list>
#include <map>
#include <vector>

#include "camera_metadata_info.h"
#include "message_parcel.h"

namespace OHOS::Camera {
class MetadataUtils {
public:
    static bool EncodeCameraMetadata(const std::shared_ptr<CameraMetadata> &metadata,
                                     MessageParcel &data);
    static void DecodeCameraMetadata(MessageParcel &data, std::shared_ptr<CameraMetadata> &metadata);

    static std::string EncodeToString(std::shared_ptr<CameraMetadata> metadata);
    static std::shared_ptr<CameraMetadata> DecodeFromString(std::string setting);
    static bool ConvertMetadataToVec(const std::shared_ptr<CameraMetadata> &metadata,
        std::vector<uint8_t>& cameraAbility);
    static void ConvertVecToMetadata(const std::vector<uint8_t>& cameraAbility,
        std::shared_ptr<CameraMetadata> &metadata);
    static void FreeMetadataBuffer(camera_metadata_item_t &entry);

    template <class T> static void WriteData(T data, std::vector<uint8_t>& cameraAbility);
    template <class T> static void ReadData(T &data, int32_t &index, const std::vector<uint8_t>& cameraAbility);
private:
    static bool WriteMetadata(const camera_metadata_item_t &item, MessageParcel &data);
    static bool ReadMetadata(camera_metadata_item_t &item, MessageParcel &data);
    static void ItemDataToBuffer(const camera_metadata_item_t &item, void **buffer);
    static void WriteMetadataDataToVec(const camera_metadata_item_t &entry, std::vector<uint8_t>& cameraAbility);
    static void ReadMetadataDataFromVec(int32_t &index, camera_metadata_item_t &entry,
        const std::vector<uint8_t>& cameraAbility);
    static int copyEncodeToStringMem(common_metadata_header_t *meta, char *encodeData, int32_t encodeDataLen);
    static int copyDecodeFromStringMem(common_metadata_header_t *meta, char *decodeData,
        char *decodeMetadataData, uint32_t totalLen);
};

template <class T>
void MetadataUtils::WriteData(T data, std::vector<uint8_t>& cameraAbility)
{
    T dataTemp = data;
    uint8_t *dataPtr = reinterpret_cast<uint8_t *>(&dataTemp);
    for (size_t j = 0; j < sizeof(T); j++) {
        cameraAbility.push_back(*(dataPtr + j));
    }
}

template <class T>
void MetadataUtils::ReadData(T &data, int32_t &index, const std::vector<uint8_t>& cameraAbility)
{
    constexpr uint32_t typeLen = sizeof(T);
    uint8_t array[typeLen] = {0};
    T *ptr = nullptr;
    for (size_t j = 0; j < sizeof(T); j++) {
        array[j] = cameraAbility.at(index++);
    }
    ptr = reinterpret_cast<T *>(array);
    data = *ptr;
}
} // namespace Camera
#endif // OHOS_CAMERA_METADATA_UTILS_H
