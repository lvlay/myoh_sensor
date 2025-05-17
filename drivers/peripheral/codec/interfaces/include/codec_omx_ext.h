/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef CODEC_OMX_EXT_H
#define CODEC_OMX_EXT_H
#include <OMX_IVCommon.h>
#include <stdbool.h>
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define PROCESS_NAME_LEN 50

/**
 * @brief Enumerates the extended AVC profile.
 */
enum CodecAVCProfileExt {
    OMX_VIDEO_AVC_LEVEL52  = 0x10000,  /**< Level 5.2 */
    OMX_VIDEO_AVC_LEVEL6   = 0x20000,  /**< Level 6 */
    OMX_VIDEO_AVC_LEVEL61  = 0x40000,  /**< Level 6.1 */
    OMX_VIDEO_AVC_LEVEL62  = 0x80000,  /**< Level 6.2 */
};

/**
 * @brief Enumerates the extended codec codingtyps.
 */
enum CodecVideoExType {
    CODEC_OMX_VIDEO_CodingVP9  = 10, /** VP9 Index in Codec HDI */
    CODEC_OMX_VIDEO_CodingHEVC = 11, /** HEVC Index in Codec HDI */
    CODEC_OMX_VIDEO_CodingVVC = 12, /** VVC Index in Codec HDI */
};

/**
 * @brief Enumerates the extended HEVC profile.
 */
enum CodecHevcProfile {
    CODEC_HEVC_PROFILE_INVALID = 0x0,
    CODEC_HEVC_PROFILE_MAIN = 0x1,
    CODEC_HEVC_PROFILE_MAIN10 = 0x2,
    CODEC_HEVC_PROFILE_MAIN_STILL = 0x3,
    // main_10 profile with HDR SEI support.
    CODEC_HEVC_PROFILE_MAIN10_HDR10 = 0x1000,
    CODEC_HEVC_PROFILE_MAIN10_HDR10_PLUS = 0x2000,
    CODEC_HEVC_PROFILE_MAX = 0x7FFFFFFF
};

/**
 * @brief Enumerates the extended HEVC level.
 */
enum CodecHevcLevel {
    CODEC_HEVC_LEVEL_INVALID = 0x0,
    CODEC_HEVC_MAIN_TIER_LEVEL1 = 0x1,
    CODEC_HEVC_HIGH_TIER_LEVEL1 = 0x2,
    CODEC_HEVC_MAIN_TIER_LEVEL2 = 0x4,
    CODEC_HEVC_HIGH_TIER_LEVEL2 = 0x8,
    CODEC_HEVC_MAIN_TIER_LEVEL21 = 0x10,
    CODEC_HEVC_HIGH_TIER_LEVEL21 = 0x20,
    CODEC_HEVC_MAIN_TIER_LEVEL3 = 0x40,
    CODEC_HEVC_HIGH_TIER_LEVEL3 = 0x80,
    CODEC_HEVC_MAIN_TIER_LEVEL31 = 0x100,
    CODEC_HEVC_HIGH_TIER_LEVEL31 = 0x200,
    CODEC_HEVC_MAIN_TIER_LEVEL4 = 0x400,
    CODEC_HEVC_HIGH_TIER_LEVEL4 = 0x800,
    CODEC_HEVC_MAIN_TIER_LEVEL41 = 0x1000,
    CODEC_HEVC_HIGH_TIER_LEVEL41 = 0x2000,
    CODEC_HEVC_MAIN_TIER_LEVEL5 = 0x4000,
    CODEC_HEVC_HIGH_TIER_LEVEL5 = 0x8000,
    CODEC_HEVC_MAIN_TIER_LEVEL51 = 0x10000,
    CODEC_HEVC_HIGH_TIER_LEVEL51 = 0x20000,
    CODEC_HEVC_MAIN_TIER_LEVEL52 = 0x40000,
    CODEC_HEVC_HIGH_TIER_LEVEL52 = 0x80000,
    CODEC_HEVC_MAIN_TIER_LEVEL6 = 0x100000,
    CODEC_HEVC_HIGH_TIER_LEVEL6 = 0x200000,
    CODEC_HEVC_MAIN_TIER_LEVEL61 = 0x400000,
    CODEC_HEVC_HIGH_TIER_LEVEL61 = 0x800000,
    CODEC_HEVC_MAIN_TIER_LEVEL62 = 0x1000000,
    CODEC_HEVC_HIGH_TIER_LEVEL62 = 0x2000000,
    CODEC_HEVC_HIGH_TIER_MAX = 0x7FFFFFFF
};

/**
 * @brief Enumerates the extended codec color format.
 */
enum CodecColorFormatExt {
    CODEC_COLOR_FORMAT_RGBA8888 = OMX_COLOR_FormatVendorStartUnused + 100,
};

/**
 * @brief Enumerates the buffer types.
 */
enum CodecBufferType {
    /** Invalid buffer type. */
    CODEC_BUFFER_TYPE_INVALID = 0,
    /** Virtual address type. */
    CODEC_BUFFER_TYPE_VIRTUAL_ADDR = 0x1,
    /** Shared memory. */
    CODEC_BUFFER_TYPE_AVSHARE_MEM_FD = 0x2,
    /** Handle. */
    CODEC_BUFFER_TYPE_HANDLE = 0x4,
    /** Dynamic handle. */
    CODEC_BUFFER_TYPE_DYNAMIC_HANDLE = 0x8,
    /** DMA memory. */
    CODEC_BUFFER_TYPE_DMA_MEM_FD = 0x10,
};

/**
 * @brief Defines the <b>SupportBuffer</b>.
 */
struct SupportBufferType {
    uint32_t size;                 /** Size of the structure */
    union OMX_VERSIONTYPE version; /** Component version */
    uint32_t portIndex;            /** Port index */
    uint32_t bufferTypes;          /** Supported buffer types */
};

/**
 * @brief Define the <b>UseBuffer</b>.
 */
struct UseBufferType {
    uint32_t size;                 /** Size of the structure */
    union OMX_VERSIONTYPE version; /** Component version */
    uint32_t portIndex;            /** Port index */
    uint32_t bufferType;           /** Buffer type */
};

/**
 * @brief Defines the <b>BufferHandleUsage</b>.
 */
struct GetBufferHandleUsageParams {
    uint32_t size;                 /** Size of the structure */
    union OMX_VERSIONTYPE version; /** Component version */
    uint32_t portIndex;            /** Port index */
    uint64_t usage;                /** Usage */
};

/**
 * @brief Defines the <b>CodecVideoPortFormatParam</b>.
 */
struct CodecVideoPortFormatParam {
    uint32_t size;                                         /** Size of the structure */
    union OMX_VERSIONTYPE version;                         /** Component version */
    uint32_t portIndex;                                    /** Port index */
    uint32_t codecColorIndex;                              /** Color format index */
    uint32_t codecColorFormat;                             /** Color format defined in Display */
    uint32_t codecCompressFormat;                          /** See  */
    uint32_t framerate;                                    /** Q16 format */
};

/**
 * @brief Defines the <b>ControlRateConstantQuality</b>.
 */
struct ControlRateConstantQuality {
    uint32_t size;                                         /** Size of the structure */
    union OMX_VERSIONTYPE version;                         /** Component version */
    uint32_t portIndex;                                    /** Port index */
    uint32_t qualityValue;                                 /** Control rate constant quality */
};

/**
 * @brief Defines the <b>PassthroughParam</b>.
 */
struct PassthroughParam {
    int32_t key;   /**< Parameter type index */
    void *val;     /**< Pointer to the parameter value */
    int size;      /**< Parameter value size */
};

/**
 * @brief Defines the <b>WorkingFrequencyParam</b>.
 */
struct WorkingFrequencyParam {
    uint32_t size;                    /** Size of the structure */
    union OMX_VERSIONTYPE version;    /** Component version */
    uint32_t level;                   /** Working Frequency level */
};

 /**
 * @brief Defines the <b>ProcessNameParam</b>.
 */
struct ProcessNameParam {
    uint32_t size;                         /** Size of the structure */
    union OMX_VERSIONTYPE version;         /** Component version */
    char processName[PROCESS_NAME_LEN];    /** Process name array */
};

/**
 * @brief Defines the <b>AudioCodecParam</b>.
 */
struct AudioCodecParam {
    uint32_t size;                 /** Size of the structure */
    union OMX_VERSIONTYPE version; /** Component version */
    uint32_t sampleRate;           /** Sample Rate */
    uint32_t sampleFormat;         /** Sample Format */
    uint32_t channels;             /** Channels */
    uint32_t bitRate;              /** Bit Rate */
    uint32_t reserved;             /** reserved word */
};

/**
 * @brief Enumerates the extended codec indexes.
 */
enum OmxIndexCodecExType {
    /** Extended BufferType index */
    OMX_IndexExtBufferTypeStartUnused = OMX_IndexKhronosExtensions + 0x00a00000,
    /** SupportBuffer */
    OMX_IndexParamSupportBufferType,
    /** UseBuffer */
    OMX_IndexParamUseBufferType,
    /** GetBufferHandleUsage */
    OMX_IndexParamGetBufferHandleUsage,
    /** CodecVideoPortFormatParam */
    OMX_IndexCodecVideoPortFormat,
    /** ControlRateConstantQuality */
    OMX_IndexParamControlRateConstantQuality,
    /** PassthroughParam */
    OMX_IndexParamPassthrough,
    /** OMX_IndexParamVideoHevc */
    OMX_IndexParamVideoHevc,
    /** range/primary/transfer/matrix */
    OMX_IndexColorAspects,
    /** WorkingFrequencyParam */
    OMX_IndexParamWorkingFrequency,
    /** ProcessNameParam */
    OMX_IndexParamProcessName,
    /** AudioCodecParam */
    OMX_IndexParamAudioCodec,
    /** CodecParamOverlayBuffer */
    OMX_IndexParamOverlayBuffer,
    /** CodecLTRParam/CodecLTRPerFrameParam */
    OMX_IndexParamLTR,
    /** CodecQPRangeParam */
    OMX_IndexParamQPRange,
    /** OMX_CONFIG_BOOLEANTYPE */
    OMX_IndexParamLowLatency,
    /** WirelessLowDelay */
    OMX_IndexParamWirelessLowDelay,
    /** OMX_S32 */
    OMX_IndexParamEncOutQp,
    /** double */
    OMX_IndexParamEncOutMse,
    /** CodecEncOutLTRParam */
    OMX_IndexParamEncOutLTR,
    /** OMX_CONFIG_BOOLEANTYPE */
    OMX_IndexParamEncParamsFeedback,
    /** OMX_S32 */
    OMX_IndexParamEncOutFrameLayer,
    /** OMX_S32 */
    OMX_IndexParamQPStsart,
    /** OMX_BOOL */
    OMX_IndexParamSkipFrame,
    /** CodecTemperalLayerParam */
    OMX_IndexParamTemperalLayer,
};

/**
 * @brief Enumerates the Other extended codec indexes.
 */
enum OmxIndexCodecOtherExtType {
    /** Extended Config AutoFramerate Conversion */
    OMX_IndexCodecExtConfigAutoFramerateConversion = OMX_IndexOtherStartUnused + 0x1,
    /** Extended Config Priority */
    OMX_IndexCodecExtConfigPriority,
    /** Extended Config OperatingRate index */
    OMX_IndexCodecExtConfigOperatingRate,
};

enum OmxIndexCodecVendorExtType {
    /** Extended Channel Attributes index */
    OMX_IndexCodecExtChannelAttributes = OMX_IndexVendorStartUnused + 0x1,
    /** CodecEnableNativeBufferParams */
    OMX_IndexCodecExtEnableNativeBuffer,
};

/**
 * @brief Structure for controlling HEVC video encoding
 */
struct CodecVideoParamHevc {
    uint32_t size;                        /** Size of the structure */
    union OMX_VERSIONTYPE version;        /** Component version */
    uint32_t portIndex;                   /** Port index */
    enum CodecHevcProfile profile;        /** Hevc profile. For details,  see {@link CodecHevcProfile}. */
    enum CodecHevcLevel level;            /** Hevc level. For details,  see {@link CodecHevcLevel}. */
    uint32_t keyFrameInterval;            /** Distance between consecutive I-frames (including one of the I frams).
                                              0 means interval is unspecified and can be freely chosen by the codec.
                                              1 means a stream of only I frams. other  means the real value. */
};

/**
 * @brief Defines the <b>CodecEnableNativeBufferParams</b>.
 */
struct CodecEnableNativeBufferParams {
    uint32_t size;                  /** Size of the structure */
    union OMX_VERSIONTYPE version;  /** Component version */
    uint32_t portIndex;             /** Port index */
    bool enable;                    /** Enable NativeBuffer */
};

enum RangeType {
    RANGE_UNSPECIFIED,
    RANGE_FULL,
    RANGE_LIMITED,
    RANGE_MAX = 0xff,
};

enum Primaries {
    PRIMARIES_UNSPECIFIED,
    PRIMARIES_BT709,                //Rec. ITU-R BT.709-6
    PRIMARIES_BT470_6M,             //Rec. ITU-R BT.470-6 System M
    PRIMARIES_BT601_625,            //Rec. ITU-R BT.601-7 625 or Rec. ITU-R BT.470-6 System B,G
    PRIMARIES_BT601_525,            //Rec. ITU-R BT.601-7 525 or SMPTE ST 170 or SMPTE ST 240
    PRIMARIES_GENERICFILM,          //Generic Film
    PRIMARIES_BT2020,               //Rec. ITU-R BT.2020-2 or Rec. ITU-R BT.2100-2
    PRIMARIES_MAX = 0xff,
};

enum Transfer {
    TRANSFER_UNSPECIFIED,
    TRANSFER_LINEAR,                //Linear transfer characteristics
    TRANSFER_SRGB,                  //IEC 61966-2-1 sRGB
    TRANSFER_SMPTE170,              //SMPTE ST 170 or Rec. ITU-R BT.709-6 or BT.601-7 or BT.2020-2
    TRANSFER_GAMMA22,               //Rec. ITU-R BT.470-6 System M
    TRANSFER_GAMMA28,               //Rec. ITU-R BT.470-6 System B,G
    TRANSFER_PQ,                    //Rec. ITU-R BT.2100-2 perceptual quantization (PQ) system
    TRANSFER_HLG,                   //Rec. ITU-R BT.2100-2 hybrid log gamma (HLG) system
    TRANSFER_SMPTE240 = 0x40,       //SMPTE ST 240
    TRANSFER_XVYCC,                 //IEC 61966-2-4
    TRANSFER_BT1361,                //Rec. ITU-R BT.1361-0 extended colour gamut system
    TRANSFER_ST428,                 //SMPTE ST 428-1
    TRANSFER_MAX = 0xff,
};

enum MatrixCoeffs {
    MATRIX_UNSPECIFED,
    MATRIX_BT709,                   //Rec. ITU-R BT.709-6
    MATRIX_FCC,                     //United States Federal Communications Commission
    MATRIX_BT601,                   //Rec. ITU-R BT.601-7 or Rec. ITU-R BT.470-6 System B,G
    MATRIX_SMPTE240,                //SMPTE ST 240
    MATRIX_BT2020,                  //Rec. ITU-R BT.2100-2 (non-constant luminance)
    MATRIX_BT2020CONSTANT,          //Rec. ITU-R BT.2100-2 (constant luminance)
    MATRIX_MAX = 0xff,
};

struct ColorAspects {
    enum RangeType range;
    enum Primaries primaries;
    enum Transfer transfer;
    enum MatrixCoeffs matrixCoeffs;
};

/**
 * @brief Structure for controlling color space
*/
struct CodecVideoColorspace {
    uint32_t size;                        /** Size of the structure */
    union OMX_VERSIONTYPE version;        /** Component version */
    uint32_t portIndex;                   /** Port index */
    uint32_t requestingDataSpace;
    uint32_t dataSpaceChanged;
    uint32_t pixeFormat;
    uint32_t dataSpace;
    struct ColorAspects aspects;
};

/**
 * @brief Structure for pAppPrivate data of OMX_BUFFERHEADERTYPE
*/
struct OMXBufferAppPrivateData {
    int32_t fd;                          /** dma fd or secure dma fd allocated by vender */
    uint32_t sizeOfParam;
    void *param;
};

struct CodecLTRParam {
    uint32_t size;                               /** Size of the structure */
    union OMX_VERSIONTYPE version;               /** Component version */
    uint32_t ltrFrameListLen;
};

struct CodecLTRPerFrameParam {
    bool markAsLTR;
    bool useLTR;
    uint32_t useLTRPoc;
};

struct CodecEncOutLTRParam {
    bool isLTR;
    uint32_t poc;
};

struct CodecQPRangeParam {
    uint32_t size;                               /** Size of the structure */
    union OMX_VERSIONTYPE version;               /** Component version */
    uint32_t minQp;
    uint32_t maxQp;
};

struct CodecTemperalLayerParam {
    uint32_t size;                               /** Size of the structure */
    union OMX_VERSIONTYPE version;               /** Component version */
    uint32_t layerCnt;
};

struct CodecParamOverlayBuffer {
    uint32_t size;                               /** Size of the structure */
    union OMX_VERSIONTYPE version;               /** Component version */
    bool enable;
    uint32_t dstX;
    uint32_t dstY;
    uint32_t dstW;
    uint32_t dstH;
    void* bufferHandle;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  // CODEC_OMX_EXT_H