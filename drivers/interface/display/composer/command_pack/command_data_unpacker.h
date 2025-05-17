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

#ifndef DISPLAY_COMMAND_DATA_UNPACKER_H
#define DISPLAY_COMMAND_DATA_UNPACKER_H

#include <memory>
#include "common/include/display_interface_utils.h"
#include "hilog/log.h"

namespace OHOS {
namespace HDI {
namespace Display {
class CommandDataUnpacker {
public:
    CommandDataUnpacker() : packSize_(0), readPos_(0), curSecOffset_(0), curSecLen_(0), data_(nullptr) {}

    void Init(char* unpackData, size_t size)
    {
        packSize_ = size;
        data_ = unpackData;
        return;
    }

    bool ReadUint64(uint64_t& value)
    {
        return Read<uint64_t>(value);
    }

    bool ReadUint32(uint32_t& value)
    {
        return Read<uint32_t>(value);
    }

    bool ReadUint8(uint8_t& value)
    {
        uint32_t intVal = 0;
        bool ret = Read<uint32_t>(intVal);
        if (ret == true) {
            value = static_cast<uint8_t>(intVal & 0xFF);
        }

        return ret;
    }

    bool ReadInt32(int32_t& value)
    {
        return Read<int32_t>(value);
    }

    bool ReadBool(bool& value)
    {
        int32_t intVal = 0;
        bool ret = Read<int32_t>(intVal);
        if (ret == true) {
            value = (intVal == 0 ? false : true);
        }

        return ret;
    }

    char *GetDataPtr()
    {
        return data_;
    }

    bool PackBegin(int32_t& beginCmd)
    {
        readPos_ = 0;
        curSecLen_ = sizeof(int32_t);
        curSecOffset_ = readPos_;

        DISPLAY_CHK_RETURN(ReadInt32(beginCmd) == false, false,
            HDF_LOGE("%{public}s, read beginCmd error", __func__));
        return true;
    }

    bool BeginSection(int32_t& cmdId)
    {
        uint32_t magic;
        curSecOffset_ = readPos_;

        DISPLAY_CHK_RETURN(ReadUint32(magic) == false, false,
            HDF_LOGE("%{public}s, read magic error", __func__));
        DISPLAY_CHK_RETURN(magic != SECTION_END_MAGIC, false,
            HDF_LOGE("%{public}s, err: magic number is corrupted", __func__));
        DISPLAY_CHK_RETURN(ReadInt32(cmdId) == false, false,
            HDF_LOGE("%{public}s, read cmdId error", __func__));
        DISPLAY_CHK_RETURN(ReadUint32(curSecLen_) == false, false,
            HDF_LOGE("%{public}s, read curSecLen_ error", __func__));
        return true;
    }

    bool NextSection()
    {
        readPos_ = curSecOffset_ + curSecLen_;
        if (readPos_ >= (packSize_ - COMMAND_ID_SIZE)) {
            return false;
        }
        return true;
    }

    bool PackEnd(int32_t& endCmd)
    {
        DISPLAY_CHK_RETURN(ReadInt32(endCmd) == false, false,
            HDF_LOGE("%{public}s, endCmd error", __func__));
        DISPLAY_CHK_RETURN(readPos_ != packSize_, false,
            HDF_LOGE("%{public}s, error: eadPos_ != packSize_", __func__));
        return true;
    }

    void Dump()
    {
        HDF_LOGI("---------------------------------------------\n");
        HDF_LOGI("SECTION_END_MAGIC =0x%{public}x\n", SECTION_END_MAGIC);
        HDF_LOGI("COMMAND_ID_SIZE   =%{public}d\n", COMMAND_ID_SIZE);
        HDF_LOGI("packSize_         =%{public}zu\n", packSize_);
        HDF_LOGI("readPos_          =%{public}zu\n", readPos_);
        HDF_LOGI("curSecOffset_     =%{public}zu\n", curSecOffset_);
        HDF_LOGI("curSecLen_        =%{public}d\n", curSecLen_);
        uint32_t i = 0;
        for (; sizeof(int32_t) * i < packSize_;) {
            HDF_LOGI("%{public}08x ", *reinterpret_cast<uint32_t *>(data_ + sizeof(int32_t) * i));
            i++;
            if (i % DUMP_LINE_LEN == 0) {
                HDF_LOGI("\n");
            } else if (i % SECTION_LEN_ALIGN == 0) {
                HDF_LOGI(" ");
            } else {
            }
        }
        HDF_LOGI("\n");
    }

private:
    template <typename T>
    bool Read(T& value)
    {
        size_t dataSize = sizeof(T);

        if (readPos_ + dataSize > packSize_) {
            HDF_LOGE("Read overflow, readPos=%{public}zu + %{public}zu}, packSize=%{public}zu.",
                readPos_, dataSize, packSize_);
            return false;
        }

        value = *reinterpret_cast<T *>(data_ + readPos_);
        readPos_ += dataSize;

        return true;
    }

private:
    static constexpr uint32_t SECTION_END_MAGIC = 0xB5B5B5B5;
    static constexpr uint32_t COMMAND_ID_SIZE = sizeof(int32_t);
    static constexpr int32_t SECTION_LEN_ALIGN = 4;
    static constexpr uint32_t DUMP_HALF_LINE_SPACE = 4;
    static constexpr uint32_t DUMP_LINE_LEN = 8;

private:
    size_t packSize_;
    size_t readPos_;
    size_t curSecOffset_;
    uint32_t curSecLen_;
    char *data_;
};
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // DISPLAY_COMMAND_DATA_UNPACKER_H
