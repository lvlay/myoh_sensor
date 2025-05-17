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

#ifndef DISPLAY_COMMAND_DATA_PACKER_H
#define DISPLAY_COMMAND_DATA_PACKER_H

#include <memory>
#include <securec.h>
#include "common/include/display_interface_utils.h"
#include "hilog/log.h"

namespace OHOS {
namespace HDI {
namespace Display {
class CommandDataPacker {
public:
    CommandDataPacker()
        : packSize_(0),
        writePos_(0),
        curSecOffset_(0),
        settingSecLen_(0),
        curSecLenPos_(0),
        data_(nullptr),
        isAdaptiveGrowth_(false)
    {
    }

    ~CommandDataPacker()
    {
        if (data_ != nullptr) {
            delete[] data_;
        }
    }

    bool Init(size_t size = INIT_DATA_SIZE, bool isAdaptiveGrowth = false)
    {
        writePos_ = 0;
        curSecOffset_ = 0;
        settingSecLen_ = 0;
        curSecLenPos_ = 0;
        isAdaptiveGrowth_ = false;
        packSize_ = size;
        uint32_t alignedSize = (packSize_ + ALLOC_PAGE_SIZE - 1) & (~(ALLOC_PAGE_SIZE - 1));
        if (data_ != nullptr) {
            delete[] data_;
        }
        data_ = new char[alignedSize];
        DISPLAY_CHK_RETURN(data_ == nullptr, false,
            HDF_LOGE("%{public}s, alloc memory failed", __func__));
        packSize_ = alignedSize;
        isAdaptiveGrowth_ = isAdaptiveGrowth;
        return true;
    }

    bool WriteUint64(uint64_t value)
    {
        return Write<uint64_t>(value);
    }

    bool WriteUint32(uint32_t value)
    {
        return Write<uint32_t>(value);
    }

    bool WriteUint8(uint8_t value)
    {
        return Write<uint32_t>(value);
    }

    bool WriteInt32(int32_t value)
    {
        return Write<int32_t>(value);
    }

    bool WriteBool(bool value)
    {
        return Write<int32_t>(value);
    }

    size_t ValidSize()
    {
        return writePos_;
    }

    size_t PackSize()
    {
        return packSize_;
    }

    char *GetDataPtr()
    {
        return data_;
    }

    bool PackBegin(int32_t beginCmd)
    {
        writePos_ = 0;
        settingSecLen_ = sizeof(int32_t);
        curSecLenPos_ = 0;
        curSecOffset_ = writePos_;
        DISPLAY_CHK_RETURN(WriteInt32(beginCmd) == false, false,
            HDF_LOGE("%{public}s, write beginCmd error", __func__));
        return true;
    }

    bool BeginSection(int32_t cmdId, uint32_t len = 0)
    {
        // len must be 4 byte alignment.
        DISPLAY_CHK_RETURN((len & SECTION_LEN_ALIGN - 1) != 0, false,
            HDF_LOGE("%{public}s, length is not aligned by 4 bytes", __func__));
        // Update current data before next section.
        curSecOffset_ = writePos_;
        DISPLAY_CHK_RETURN(WriteUint32(SECTION_END_MAGIC) == false, false,
            HDF_LOGE("%{public}s, write SECTION_END_MAGIC error", __func__));
        DISPLAY_CHK_RETURN(WriteInt32(cmdId) == false, false,
            HDF_LOGE("%{public}s, write cmdID error", __func__));
        curSecLenPos_ = writePos_;
        settingSecLen_ = len;
        // Now we don't write section len here,
        // but write it on EndSection.
        writePos_ += sizeof(uint32_t);
        return true;
    }

    bool EndSection()
    {
        // Write cmd len before data related is updated.
        DISPLAY_CHK_RETURN(writePos_ < curSecOffset_, false,
            HDF_LOGE("%{public}s, error: writePos_ after curSecOffset_", __func__));
        // Write cmd len before data related is updated.
        uint32_t actualLen = writePos_ - curSecOffset_;
        uint32_t updatedLen = actualLen > settingSecLen_ ? actualLen : settingSecLen_;
        // We must check "writePos" < packSize_ Before write
        if ((curSecLenPos_ + sizeof(updatedLen)) > packSize_) {
            HDF_LOGE("%{public}s, error: current pos > packSize", __func__);
            return false;
        } else {
            *reinterpret_cast<uint32_t *>(data_ + curSecLenPos_) = updatedLen;
        }
        writePos_ = curSecOffset_ + updatedLen;

        DISPLAY_CHK_RETURN(writePos_  >= packSize_, false,
            HDF_LOGE("%{public}s, error: writePos_ > packSize", __func__));
        return true;
    }

    bool PackEnd(int32_t endCmd)
    {
        DISPLAY_CHK_RETURN(WriteInt32(endCmd) == false, false,
            HDF_LOGE("%{public}s, write endCmd error", __func__));
        DISPLAY_CHK_RETURN(writePos_  >= packSize_, false,
            HDF_LOGE("%{public}s, error: writePos_ > packSize", __func__));
        return true;
    }

    void Dump()
    {
        HDF_LOGI("---------------------------------------------\n");
        HDF_LOGI("ALLOC_PAGE_SIZE       =%{public}d\n", ALLOC_PAGE_SIZE);
        HDF_LOGI("INIT_DATA_SIZE        =%{public}d\n", INIT_DATA_SIZE);
        HDF_LOGI("SECTION_END_MAGIC     =0x%{public}x\n", SECTION_END_MAGIC);
        HDF_LOGI("packSize_             =%{public}zu\n", packSize_);
        HDF_LOGI("writePos_             =%{public}zu\n", writePos_);
        HDF_LOGI("curSecOffset_         =%{public}zu\n", curSecOffset_);
        HDF_LOGI("settingSecLen_        =%{public}d\n", settingSecLen_);
        HDF_LOGI("curSecLenPos_         =%{public}zu\n", curSecLenPos_);
        uint32_t i = 0;
        for (; sizeof(int32_t) * i < writePos_;) {
            HDF_LOGI("%{public}08x ", *reinterpret_cast<uint32_t *>(data_ + sizeof(int32_t) * i));
            i++;
            if ((i % DUMP_LINE_LEN) == 0) {
                HDF_LOGI("\n");
            } else if ((i % DUMP_HALF_LINE_SPACE) == 0) {
                HDF_LOGI(" ");
            } else {
            }
        }
        HDF_LOGI("\n");
    }

private:
    template <typename T>
    bool Write(T value)
    {
        size_t writeSize = sizeof(T);
        size_t newSize = writePos_ + writeSize;
        if (newSize > packSize_) {
            if (!isAdaptiveGrowth_) {
                HDF_LOGE("%{public}s: command packer is overflow", __func__);
                return false;
            }
            newSize = (newSize + ALLOC_PAGE_SIZE - 1) & (~(ALLOC_PAGE_SIZE - 1));
            char *newData = new char[newSize];
            if (newData == nullptr) {
                HDF_LOGE("%{public}s: mem alloc failed", __func__);
                return false;
            }
            if (memcpy_s(newData, newSize, data_, packSize_) != EOK) {
                HDF_LOGE("%{public}s: memcpy_s failed", __func__);
                delete[] newData;
                newData = nullptr;
                return false;
            }
            delete[] data_;
            data_ = newData;
            packSize_ = newSize;
        }
        *reinterpret_cast<T *>(data_ + writePos_) = value;
        writePos_ += writeSize;

        return true;
    }

private:
    static constexpr uint32_t ALLOC_PAGE_SIZE = 1024;
    static constexpr uint32_t INIT_DATA_SIZE = 32 * ALLOC_PAGE_SIZE;
    static constexpr uint32_t SECTION_END_MAGIC = 0xB5B5B5B5;
    static constexpr uint32_t SECTION_LEN_ALIGN = 4;
    static constexpr uint32_t DUMP_HALF_LINE_SPACE = 4;
    static constexpr uint32_t DUMP_LINE_LEN = 8;

private:
    size_t packSize_;
    size_t writePos_;
    size_t curSecOffset_;
    uint32_t settingSecLen_;
    size_t curSecLenPos_;
    char *data_;
    bool isAdaptiveGrowth_;
};
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // DISPLAY_COMMAND_DATA_PACKER_H
