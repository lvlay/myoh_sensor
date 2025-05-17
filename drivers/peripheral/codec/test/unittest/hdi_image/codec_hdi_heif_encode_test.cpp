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

#include <gtest/gtest.h>
#include "test_helper.h"
#include "v1_2/display_composer_type.h"
#include "v1_2/display_buffer_type.h"
#include "v1_2/include/idisplay_buffer.h"

#define HDF_LOG_TAG codec_heif_encode_test

namespace {
using namespace std;
using namespace testing::ext;
using namespace OHOS::HDI::Codec::Image::V2_0;
using namespace OHOS::HDI::Display::Buffer::V1_2;
using namespace OHOS::HDI::Display::Composer::V1_2;

static constexpr size_t EXTERNAL_BUFFER_SIZE = 18 * 1024 * 1024;
static constexpr size_t META_BUFFER_SIZE = 1024;

static uint32_t id_ = 0;
static OHOS::sptr<ICodecImage> hdiHeifEncoder_ = nullptr;
static OHOS::HDI::Display::Buffer::V1_2::IDisplayBuffer* bufferMgr_ = nullptr;
static uint32_t pixelFmtNv12_ = OHOS::HDI::Display::Composer::V1_2::PIXEL_FMT_YCBCR_420_SP;
static uint32_t pixelFmtRgba_ = OHOS::HDI::Display::Composer::V1_2::PIXEL_FMT_RGBA_8888;
static uint32_t unsupportedPixelFmt_ = OHOS::HDI::Display::Composer::V1_2::PIXEL_FMT_YUV_422_I;

class CodecHdiHeifEncodeTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        hdiHeifEncoder_ = ICodecImage::Get();
        bufferMgr_ = OHOS::HDI::Display::Buffer::V1_2::IDisplayBuffer::Get();
    }
    static void TearDownTestCase()
    {
        hdiHeifEncoder_ = nullptr;
        bufferMgr_ = nullptr;
    }
    void SetUp()
    {
        inputImgs_.clear();
        inputMetas_.clear();
        refs_.clear();
        output_ = AllocateSharedBuffer(EXTERNAL_BUFFER_SIZE);
        filledLen_ = 0;
    }
    void TearDown()
    {
        for (auto item : inputImgs_) {
            FreeSharedBuffer(item.sharedProperties);
        }
        for (auto item : inputMetas_) {
            FreeSharedBuffer(item.data);
        }
        FreeSharedBuffer(output_);
    }
public:
    enum ImgType {
        PRIMARY_IMG,
        AUXILIARY_IMG,
        THUMBNAIL_IMG,
        GAIN_MAP,
        T_MAP
    };
    enum MetaType {
        EXIF_DATA,
        USER_DATA
    };
public:
    static uint32_t GetNextId()
    {
        return id_++;
    }
    static SharedBuffer AllocateSharedBuffer(uint32_t size)
    {
        SharedBuffer sb = {
            .fd = -1,
            .filledLen = 0,
            .capacity = 0
        };
        IF_TRUE_RETURN_VAL(size <= 0, sb);
        int fd = AshmemCreate("ForHeifEncodeUT", size);
        if (fd > 0) {
            sb.fd = fd;
            sb.filledLen = static_cast<uint32_t>(size);
            sb.capacity = static_cast<uint32_t>(AshmemGetSize(fd));
        } else {
            HDF_LOGE("cannot create ashmem");
        }
        return sb;
    }

    static void FreeSharedBuffer(SharedBuffer& sb)
    {
        if (sb.fd > 0) {
            close(sb.fd);
        }
    }

    static sptr<NativeBuffer> AllocateNativeBuffer(uint32_t width, uint32_t height, uint32_t pixelFmt)
    {
        uint64_t usage = OHOS::HDI::Display::Composer::V1_2::HBM_USE_CPU_READ |
                         OHOS::HDI::Display::Composer::V1_2::HBM_USE_CPU_WRITE |
                         OHOS::HDI::Display::Composer::V1_2::HBM_USE_MEM_DMA;
        AllocInfo alloc = {
            .width = width,
            .height = height,
            .usage =  usage,
            .format = pixelFmt
        };
        BufferHandle *handle = nullptr;
        int32_t ret = bufferMgr_->AllocMem(alloc, handle);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("failed to alloc buffer, err [%{public}d] !", ret);
            return nullptr;
        }
        sptr<NativeBuffer> imgBuffer = new NativeBuffer(handle);
        return imgBuffer;
    }
    static ImageItem CreateImageItem(ImgType imageType, uint32_t pixelFmt, bool needAllocPixelBuffer = true)
    {
        static constexpr uint32_t ENCODE_QUALITY = 85;
        static constexpr uint32_t PIXEL_WIDTH = 1920;
        static constexpr uint32_t PIXEL_HEIGHT = 1080;
        ImageItem item = {
            .itemName = "",
            .id = GetNextId(),
            .pixelBuffer = nullptr,
            .isPrimary = (imageType == PRIMARY_IMG),
            .isHidden = (imageType != PRIMARY_IMG),
            .compressType = "hevc",
            .quality = ENCODE_QUALITY,
            .liteProperties = {},
            .sharedProperties = AllocateSharedBuffer(0)
        };
        if (needAllocPixelBuffer && (imageType != T_MAP)) {
            item.pixelBuffer = AllocateNativeBuffer(PIXEL_WIDTH, PIXEL_HEIGHT, pixelFmt);
        }
        return item;
    }
    static MetaItem CreateMetaItem(MetaType type)
    {
        MetaItem item = {
            .itemName = (type == USER_DATA) ? "userdata" : "exif",
            .id = GetNextId(),
            .data = AllocateSharedBuffer(META_BUFFER_SIZE),
            .properties = {}
        };
        if (type == USER_DATA) {
            bool useCompress = true;
            PropWriter pw;
            (void)pw.AddData<bool>(USER_DATA_DO_COMPRESS, useCompress);
            (void)pw.Finalize(item.properties);
        }
        return item;
    }
    static bool SetValidNclxColor(ImageItem& item)
    {
        PropWriter pw;
        ColorType clrType = NCLX;
        IF_TRUE_RETURN_VAL(!pw.AddData<ColorType>(COLOR_TYPE, clrType), false);
        ColourInfo clrInfo = {
            .colourPrimaries = 2,
            .transferCharacteristics = 2,
            .matrixCoefficients = 2,
            .fullRangeFlag = false
        };
        IF_TRUE_RETURN_VAL(!pw.AddData<ColourInfo>(COLOR_INFO, clrInfo), false);
        return pw.Finalize(item.liteProperties);
    }
    static bool SetColorTypeOnly(ImageItem& item, ColorType clrType)
    {
        PropWriter pw;
        IF_TRUE_RETURN_VAL(!pw.AddData<ColorType>(COLOR_TYPE, clrType), false);
        return pw.Finalize(item.liteProperties);
    }
    static bool SetInvalidRiccColor(ImageItem& item)
    {
        PropWriter pw;
        ColorType clrType = RICC;
        IF_TRUE_RETURN_VAL(!pw.AddData<ColorType>(COLOR_TYPE, clrType), false);
        IF_TRUE_RETURN_VAL(!pw.Finalize(item.liteProperties), false);
        PropertyType propType = ICC_PROFILE;
        size_t bufferSize = sizeof(propType);
        item.sharedProperties = AllocateSharedBuffer(bufferSize);
        IF_TRUE_RETURN_VAL(item.sharedProperties.fd < 0, false);
        void *addr = mmap(nullptr, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, item.sharedProperties.fd, 0);
        if (addr == nullptr) {
            close(item.sharedProperties.fd);
            item.sharedProperties.fd = -1;
            return false;
        }
        errno_t ret = memcpy_s(addr, bufferSize, reinterpret_cast<uint8_t*>(&propType), bufferSize);
        (void)munmap(addr, bufferSize);
        return (ret == EOK);
    }
    static bool SetInvalidNclxColor(ImageItem& item)
    {
        PropWriter pw;
        ColourInfo clrInfo = {
            .colourPrimaries = 2,
            .transferCharacteristics = 2,
            .matrixCoefficients = 2,
            .fullRangeFlag = false
        };
        IF_TRUE_RETURN_VAL(!pw.AddData<ColourInfo>(COLOR_INFO, clrInfo), false);
        return pw.Finalize(item.liteProperties);
    }
    static bool SetPropeForTmap(ImageItem& item)
    {
        PropWriter pw;
        ColorType clrType = NCLX;
        IF_TRUE_RETURN_VAL(!pw.AddData<ColorType>(COLOR_TYPE, clrType), false);
        ColourInfo clrInfo = {
            .colourPrimaries = 2,
            .transferCharacteristics = 2,
            .matrixCoefficients = 2,
            .fullRangeFlag = false
        };
        IF_TRUE_RETURN_VAL(!pw.AddData<ColourInfo>(COLOR_INFO, clrInfo), false);
        ToneMapMetadata tmapMeta;
        static constexpr uint8_t MULTI_CHANNEL = 3;
        tmapMeta.channelCnt = MULTI_CHANNEL;
        tmapMeta.useBaseColorSpace = true;
        tmapMeta.baseHdrHeadroom = {12, 23};
        tmapMeta.alternateHdrHeadroom = {36, 62};
        tmapMeta.channels1 = {
            .gainMapMin = {5, 21},
            .gainMapMax = {5, 7},
            .gamma = {2, 7},
            .baseOffset = {1, 3},
            .alternateOffset = {1, 7}
        };
        tmapMeta.channels2 = {
            .gainMapMin = {5, 21},
            .gainMapMax = {5, 7},
            .gamma = {2, 7},
            .baseOffset = {1, 3},
            .alternateOffset = {1, 7}
        };
        tmapMeta.channels3 = {
            .gainMapMin = {5, 21},
            .gainMapMax = {5, 7},
            .gamma = {2, 7},
            .baseOffset = {1, 3},
            .alternateOffset = {1, 7}
        };
        IF_TRUE_RETURN_VAL(!pw.AddData<ToneMapMetadata>(TONE_MAP_METADATA, tmapMeta), false);
        return pw.Finalize(item.liteProperties);
    }
public:
    vector<ImageItem> inputImgs_;
    vector<MetaItem> inputMetas_;
    vector<ItemRef> refs_;
    SharedBuffer output_;
    uint32_t filledLen_;
};

// [FAIL] auxl image only
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_003, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem auxlImageItem = CreateImageItem(AUXILIARY_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(auxlImageItem));
    inputImgs_.emplace_back(auxlImageItem);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] primary image + (image.pixelBuffer == nullptr)
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_004, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, pixelFmtRgba_, false);
    ASSERT_TRUE(SetValidNclxColor(primaryImageItem));
    inputImgs_.emplace_back(primaryImageItem);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] primary image + unsupported pixelFmt in image.pixelBuffer
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_005, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, unsupportedPixelFmt_, false);
    ASSERT_TRUE(SetValidNclxColor(primaryImageItem));
    inputImgs_.emplace_back(primaryImageItem);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] primary image + (COLOR_TYPE == NCLX) + COLOR_INFO not configured
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_006, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, unsupportedPixelFmt_, false);
    ASSERT_TRUE(SetColorTypeOnly(primaryImageItem, NCLX));
    inputImgs_.emplace_back(primaryImageItem);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] primary image + (COLOR_TYPE == RICC) + ICC_PROFILE not configured
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_007, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, unsupportedPixelFmt_, false);
    ASSERT_TRUE(SetColorTypeOnly(primaryImageItem, RICC));
    inputImgs_.emplace_back(primaryImageItem);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] primary image + (COLOR_TYPE == RICC) + SharedBuffer that store ICC_PROFILE is not properly filled
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_008, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, unsupportedPixelFmt_, false);
    ASSERT_TRUE(SetInvalidRiccColor(primaryImageItem));
    inputImgs_.emplace_back(primaryImageItem);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] primary image + COLOR_INFO is configured + COLOR_TYPE not configured
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_009, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, unsupportedPixelFmt_, false);
    ASSERT_TRUE(SetInvalidNclxColor(primaryImageItem));
    inputImgs_.emplace_back(primaryImageItem);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] auxl image is not related to primary image
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_010, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(primaryImageItem));
    ImageItem auxlImageItem = CreateImageItem(AUXILIARY_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(auxlImageItem));
    ImageItem thumImageItem = CreateImageItem(THUMBNAIL_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(thumImageItem));
    refs_.emplace_back(ItemRef {
        .type = AUXL,
        .auxType = "",
        .from = auxlImageItem.id,
        .to = { thumImageItem.id }
    });
    refs_.emplace_back(ItemRef {
        .type = THMB,
        .auxType = "",
        .from = thumImageItem.id,
        .to = { primaryImageItem.id }
    });
    inputImgs_.emplace_back(primaryImageItem);
    inputImgs_.emplace_back(auxlImageItem);
    inputImgs_.emplace_back(thumImageItem);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] auxl image is configured in refs, but not included in inputImgs
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_011, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(primaryImageItem));
    ImageItem auxlImageItem = CreateImageItem(AUXILIARY_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(auxlImageItem));
    refs_.emplace_back(ItemRef {
        .type = AUXL,
        .auxType = "",
        .from = 100,
        .to = { primaryImageItem.id }
    });
    inputImgs_.emplace_back(primaryImageItem);
    inputImgs_.emplace_back(auxlImageItem);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] auxl image is related to an image that is not included in inputImgs
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_012, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(primaryImageItem));
    ImageItem auxlImageItem = CreateImageItem(AUXILIARY_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(auxlImageItem));
    refs_.emplace_back(ItemRef {
        .type = AUXL,
        .auxType = "",
        .from = auxlImageItem.id,
        .to = { 100 }
    });
    inputImgs_.emplace_back(primaryImageItem);
    inputImgs_.emplace_back(auxlImageItem);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] auxl image is related to more than one images
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_013, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(primaryImageItem));
    ImageItem auxlImageItem = CreateImageItem(AUXILIARY_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(auxlImageItem));
    ImageItem thumImageItem = CreateImageItem(THUMBNAIL_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(thumImageItem));
    refs_.emplace_back(ItemRef {
        .type = AUXL,
        .auxType = "",
        .from = auxlImageItem.id,
        .to = { primaryImageItem.id, thumImageItem.id }
    });
    refs_.emplace_back(ItemRef {
        .type = THMB,
        .auxType = "",
        .from = thumImageItem.id,
        .to = { primaryImageItem.id }
    });
    inputImgs_.emplace_back(primaryImageItem);
    inputImgs_.emplace_back(auxlImageItem);
    inputImgs_.emplace_back(thumImageItem);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] userdata meta is not related to primary image
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_014, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(primaryImageItem));
    ImageItem auxlImageItem = CreateImageItem(AUXILIARY_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(auxlImageItem));
    refs_.emplace_back(ItemRef {
        .type = AUXL,
        .auxType = "",
        .from = auxlImageItem.id,
        .to = { primaryImageItem.id }
    });
    inputImgs_.emplace_back(primaryImageItem);
    inputImgs_.emplace_back(auxlImageItem);
    MetaItem metaUserData = CreateMetaItem(USER_DATA);
    refs_.emplace_back(ItemRef {
        .type = CDSC,
        .auxType = "",
        .from = metaUserData.id,
        .to = { auxlImageItem.id }
    });
    inputMetas_.emplace_back(metaUserData);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] userdata meta is configured in refs, but not included in inputMetas
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_015, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(primaryImageItem));
    inputImgs_.emplace_back(primaryImageItem);
    MetaItem metaUserData = CreateMetaItem(USER_DATA);
    refs_.emplace_back(ItemRef {
        .type = CDSC,
        .auxType = "",
        .from = metaUserData.id,
        .to = { primaryImageItem.id }
    });
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] userdata meta is related to an image that is not included in inputImgs
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_016, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(primaryImageItem));
    inputImgs_.emplace_back(primaryImageItem);
    MetaItem metaUserData = CreateMetaItem(USER_DATA);
    refs_.emplace_back(ItemRef {
        .type = CDSC,
        .auxType = "",
        .from = metaUserData.id,
        .to = { 100 }
    });
    inputMetas_.emplace_back(metaUserData);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] userdata meta is related to more than one images
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_017, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(primaryImageItem));
    inputImgs_.emplace_back(primaryImageItem);
    MetaItem metaUserData = CreateMetaItem(USER_DATA);
    refs_.emplace_back(ItemRef {
        .type = CDSC,
        .auxType = "",
        .from = metaUserData.id,
        .to = { primaryImageItem.id, 100 }
    });
    inputMetas_.emplace_back(metaUserData);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] output buffer is not properly configured, fd or capacity is invalid
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_018, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(primaryImageItem));
    inputImgs_.emplace_back(primaryImageItem);
    SharedBuffer errOurput = AllocateSharedBuffer(0);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, errOurput, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] output buffer is not properly configured, capacity is too small
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_019, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, pixelFmtRgba_);
    ASSERT_TRUE(SetValidNclxColor(primaryImageItem));
    inputImgs_.emplace_back(primaryImageItem);
    size_t smallSize = 128;
    SharedBuffer errOurput = AllocateSharedBuffer(smallSize);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, errOurput, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] Tmap image is configured in refs, but not included in inputImgs
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_021, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem tmapImageItem = CreateImageItem(T_MAP, pixelFmtNv12_);
    ASSERT_TRUE(SetPropeForTmap(tmapImageItem));
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, pixelFmtNv12_);
    ASSERT_TRUE(SetValidNclxColor(primaryImageItem));
    ImageItem gainMapImageItem = CreateImageItem(GAIN_MAP, pixelFmtNv12_);
    ASSERT_TRUE(SetValidNclxColor(gainMapImageItem));
    refs_.emplace_back(ItemRef {
        .type = DIMG,
        .auxType = "",
        .from = tmapImageItem.id,
        .to = { primaryImageItem.id, gainMapImageItem.id }
    });
    inputImgs_.emplace_back(primaryImageItem);
    inputImgs_.emplace_back(gainMapImageItem);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] Tmap is not related to any gainmap
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_022, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem tmapImageItem = CreateImageItem(T_MAP, pixelFmtNv12_);
    ASSERT_TRUE(SetPropeForTmap(tmapImageItem));
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, pixelFmtNv12_);
    ASSERT_TRUE(SetValidNclxColor(primaryImageItem));
    ImageItem gainMapImageItem = CreateImageItem(GAIN_MAP, pixelFmtNv12_);
    ASSERT_TRUE(SetValidNclxColor(gainMapImageItem));
    refs_.emplace_back(ItemRef {
        .type = DIMG,
        .auxType = "",
        .from = tmapImageItem.id,
        .to = { primaryImageItem.id }
    });
    inputImgs_.emplace_back(tmapImageItem);
    inputImgs_.emplace_back(primaryImageItem);
    inputImgs_.emplace_back(gainMapImageItem);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] Tmap is related to two identical images
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_023, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem tmapImageItem = CreateImageItem(T_MAP, pixelFmtNv12_);
    ASSERT_TRUE(SetPropeForTmap(tmapImageItem));
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, pixelFmtNv12_);
    ASSERT_TRUE(SetValidNclxColor(primaryImageItem));
    ImageItem gainMapImageItem = CreateImageItem(GAIN_MAP, pixelFmtNv12_);
    ASSERT_TRUE(SetValidNclxColor(gainMapImageItem));
    refs_.emplace_back(ItemRef {
        .type = DIMG,
        .auxType = "",
        .from = tmapImageItem.id,
        .to = { gainMapImageItem.id, gainMapImageItem.id }
    });
    inputImgs_.emplace_back(tmapImageItem);
    inputImgs_.emplace_back(primaryImageItem);
    inputImgs_.emplace_back(gainMapImageItem);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}

// [FAIL] Tmap is related to images that are not included in inputImgs
HWTEST_F(CodecHdiHeifEncodeTest, HdfCodecHdiDoHeifEncodeTest_024, TestSize.Level1)
{
    ASSERT_TRUE(hdiHeifEncoder_ != nullptr);
    ASSERT_TRUE(bufferMgr_ != nullptr);
    ImageItem tmapImageItem = CreateImageItem(T_MAP, pixelFmtNv12_);
    ASSERT_TRUE(SetPropeForTmap(tmapImageItem));
    ImageItem primaryImageItem = CreateImageItem(PRIMARY_IMG, pixelFmtNv12_);
    ASSERT_TRUE(SetValidNclxColor(primaryImageItem));
    ImageItem gainMapImageItem = CreateImageItem(GAIN_MAP, pixelFmtNv12_);
    ASSERT_TRUE(SetValidNclxColor(gainMapImageItem));
    refs_.emplace_back(ItemRef {
        .type = DIMG,
        .auxType = "",
        .from = tmapImageItem.id,
        .to = { gainMapImageItem.id, 100 }
    });
    inputImgs_.emplace_back(tmapImageItem);
    inputImgs_.emplace_back(primaryImageItem);
    inputImgs_.emplace_back(gainMapImageItem);
    int32_t ret = hdiHeifEncoder_->DoHeifEncode(inputImgs_, inputMetas_, refs_, output_, filledLen_);
    ASSERT_NE(ret, HDF_SUCCESS);
    ASSERT_EQ(filledLen_, 0);
}
}