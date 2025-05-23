/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef CAMERA_HOST_METADATA_ENUM_MAP_H
#define CAMERA_HOST_METADATA_ENUM_MAP_H

#include <map>
#include <string>
#include "camera_device_ability_items.h"

std::map<std::string, camera_device_metadata_tag_t> MetadataTagMap = {
    {"OHOS_ABILITY_CAMERA_POSITION",                       OHOS_ABILITY_CAMERA_POSITION                      },
    {"OHOS_ABILITY_CAMERA_TYPE",                           OHOS_ABILITY_CAMERA_TYPE                          },
    {"OHOS_ABILITY_CAMERA_CONNECTION_TYPE",                OHOS_ABILITY_CAMERA_CONNECTION_TYPE               },
    {"OHOS_ABILITY_MEMORY_TYPE",                           OHOS_ABILITY_MEMORY_TYPE                          },
    {"OHOS_SENSOR_INFO_ACTIVE_ARRAY_SIZE",                 OHOS_SENSOR_INFO_ACTIVE_ARRAY_SIZE                },
    {"OHOS_SENSOR_INFO_SENSITIVITY_RANGE",                 OHOS_SENSOR_INFO_SENSITIVITY_RANGE                },
    {"OHOS_SENSOR_INFO_MAX_FRAME_DURATION",                OHOS_SENSOR_INFO_MAX_FRAME_DURATION               },
    {"OHOS_SENSOR_INFO_PHYSICAL_SIZE",                     OHOS_SENSOR_INFO_PHYSICAL_SIZE                    },
    {"OHOS_SENSOR_INFO_PIXEL_ARRAY_SIZE",                  OHOS_SENSOR_INFO_PIXEL_ARRAY_SIZE                 },
    {"OHOS_STATISTICS_FACE_DETECT_MODE",                   OHOS_STATISTICS_FACE_DETECT_MODE                  },
    {"OHOS_STATISTICS_HISTOGRAM_MODE",                     OHOS_STATISTICS_HISTOGRAM_MODE                    },
    {"OHOS_STATISTICS_FACE_IDS",                           OHOS_STATISTICS_FACE_IDS                          },
    {"OHOS_STATISTICS_FACE_LANDMARKS",                     OHOS_STATISTICS_FACE_LANDMARKS                    },
    {"OHOS_STATISTICS_FACE_RECTANGLES",                    OHOS_STATISTICS_FACE_RECTANGLES                   },
    {"OHOS_STATISTICS_FACE_SCORES",                        OHOS_STATISTICS_FACE_SCORES                       },
    {"OHOS_CONTROL_AE_ANTIBANDING_MODE",                   OHOS_CONTROL_AE_ANTIBANDING_MODE                  },
    {"OHOS_CONTROL_AE_EXPOSURE_COMPENSATION",              OHOS_CONTROL_AE_EXPOSURE_COMPENSATION             },
    {"OHOS_CONTROL_AE_LOCK",                               OHOS_CONTROL_AE_LOCK                              },
    {"OHOS_CONTROL_AE_MODE",                               OHOS_CONTROL_AE_MODE                              },
    {"OHOS_CONTROL_AE_REGIONS",                            OHOS_CONTROL_AE_REGIONS                           },
    {"OHOS_CONTROL_AE_TARGET_FPS_RANGE",                   OHOS_CONTROL_AE_TARGET_FPS_RANGE                  },
    {"OHOS_CONTROL_AF_MODE",                               OHOS_CONTROL_AF_MODE                              },
    {"OHOS_CONTROL_AF_REGIONS",                            OHOS_CONTROL_AF_REGIONS                           },
    {"OHOS_CONTROL_AWB_LOCK",                              OHOS_CONTROL_AWB_LOCK                             },
    {"OHOS_CONTROL_AWB_MODE",                              OHOS_CONTROL_AWB_MODE                             },
    {"OHOS_CONTROL_AWB_REGIONS",                           OHOS_CONTROL_AWB_REGIONS                          },
    {"OHOS_CONTROL_AE_AVAILABLE_ANTIBANDING_MODES",        OHOS_CONTROL_AE_AVAILABLE_ANTIBANDING_MODES       },
    {"OHOS_CONTROL_AE_AVAILABLE_MODES",                    OHOS_CONTROL_AE_AVAILABLE_MODES                   },
    {"OHOS_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES",        OHOS_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES       },
    {"OHOS_CONTROL_AE_COMPENSATION_RANGE",                 OHOS_ABILITY_AE_COMPENSATION_RANGE                },
    {"OHOS_CONTROL_AE_COMPENSATION_STEP",                  OHOS_ABILITY_AE_COMPENSATION_STEP                 },
    {"OHOS_CONTROL_AF_AVAILABLE_MODES",                    OHOS_CONTROL_AF_AVAILABLE_MODES                   },
    {"OHOS_CONTROL_AWB_AVAILABLE_MODES",                   OHOS_CONTROL_AWB_AVAILABLE_MODES                  },
    {"OHOS_ABILITY_DEVICE_AVAILABLE_EXPOSUREMODES",        OHOS_ABILITY_DEVICE_AVAILABLE_EXPOSUREMODES       },
    {"OHOS_CONTROL_EXPOSUREMODE",                          OHOS_CONTROL_EXPOSUREMODE                         },
    {"OHOS_ABILITY_DEVICE_AVAILABLE_FOCUSMODES",           OHOS_ABILITY_DEVICE_AVAILABLE_FOCUSMODES          },
    {"OHOS_CONTROL_FOCUSMODE",                             OHOS_CONTROL_FOCUSMODE                            },
    {"OHOS_ABILITY_DEVICE_AVAILABLE_FLASHMODES",           OHOS_ABILITY_DEVICE_AVAILABLE_FLASHMODES          },
    {"OHOS_CONTROL_FLASHMODE",                             OHOS_CONTROL_FLASHMODE                            },
    {"OHOS_ABILITY_ZOOM_RATIO_RANGE",                      OHOS_ABILITY_ZOOM_RATIO_RANGE                     },
    {"OHOS_CONTROL_ZOOM_RATIO",                            OHOS_CONTROL_ZOOM_RATIO                           },
    {"OHOS_ABILITY_STREAM_AVAILABLE_BASIC_CONFIGURATIONS", OHOS_ABILITY_STREAM_AVAILABLE_BASIC_CONFIGURATIONS},
    {"OHOS_JPEG_GPS_COORDINATES",                          OHOS_JPEG_GPS_COORDINATES                         },
    {"OHOS_JPEG_GPS_PROCESSING_METHOD",                    OHOS_JPEG_GPS_PROCESSING_METHOD                   },
    {"OHOS_JPEG_GPS_TIMESTAMP",                            OHOS_JPEG_GPS_TIMESTAMP                           },
    {"OHOS_JPEG_ORIENTATION",                              OHOS_JPEG_ORIENTATION                             },
    {"OHOS_JPEG_QUALITY",                                  OHOS_JPEG_QUALITY                                 },
    {"OHOS_JPEG_THUMBNAIL_QUALITY",                        OHOS_JPEG_THUMBNAIL_QUALITY                       },
    {"OHOS_JPEG_THUMBNAIL_SIZE",                           OHOS_JPEG_THUMBNAIL_SIZE                          },
    {"OHOS_JPEG_AVAILABLE_THUMBNAIL_SIZES",                OHOS_JPEG_AVAILABLE_THUMBNAIL_SIZES               },
    {"OHOS_JPEG_MAX_SIZE",                                 OHOS_JPEG_MAX_SIZE                                },
    {"OHOS_JPEG_SIZE",                                     OHOS_JPEG_SIZE                                    },
};

std::map<std::string, camera_position_enum_t> CameraPositionMap = {
    {"OHOS_CAMERA_POSITION_FRONT", OHOS_CAMERA_POSITION_FRONT},
    {"OHOS_CAMERA_POSITION_BACK",  OHOS_CAMERA_POSITION_BACK },
    {"OHOS_CAMERA_POSITION_OTHER", OHOS_CAMERA_POSITION_OTHER},
};

std::map<std::string, camera_type_enum_t> CameraTypeMap = {
    {"OHOS_CAMERA_TYPE_WIDE_ANGLE",  OHOS_CAMERA_TYPE_WIDE_ANGLE },
    {"OHOS_CAMERA_TYPE_ULTRA_WIDE",  OHOS_CAMERA_TYPE_ULTRA_WIDE },
    {"OHOS_CAMERA_TYPE_TELTPHOTO",   OHOS_CAMERA_TYPE_TELTPHOTO  },
    {"OHOS_CAMERA_TYPE_TRUE_DEAPTH", OHOS_CAMERA_TYPE_TRUE_DEAPTH},
    {"OHOS_CAMERA_TYPE_LOGICAL",     OHOS_CAMERA_TYPE_LOGICAL    },
    {"OHOS_CAMERA_TYPE_UNSPECIFIED", OHOS_CAMERA_TYPE_UNSPECIFIED},
};

std::map<std::string, camera_connection_type_t> cameraConnectionTypeMap = {
    {"OHOS_CAMERA_CONNECTION_TYPE_BUILTIN",    OHOS_CAMERA_CONNECTION_TYPE_BUILTIN   },
    {"OHOS_CAMERA_CONNECTION_TYPE_USB_PLUGIN", OHOS_CAMERA_CONNECTION_TYPE_USB_PLUGIN},
    {"OHOS_CAMERA_CONNECTION_TYPE_REMOTE",     OHOS_CAMERA_CONNECTION_TYPE_REMOTE    },
};

std::map<std::string, camera_exposure_mode_enum_t> ExposureModeMap = {
    {"OHOS_CAMERA_EXPOSURE_MODE_MANUAL",          OHOS_CAMERA_EXPOSURE_MODE_MANUAL         },
    {"OHOS_CAMERA_EXPOSURE_MODE_CONTINUOUS_AUTO", OHOS_CAMERA_EXPOSURE_MODE_CONTINUOUS_AUTO},
    {"OHOS_CAMERA_EXPOSURE_MODE_LOCKED",          OHOS_CAMERA_EXPOSURE_MODE_LOCKED         },
    {"OHOS_CAMERA_EXPOSURE_MODE_AUTO",            OHOS_CAMERA_EXPOSURE_MODE_AUTO           },
};

std::map<std::string, camera_focus_mode_enum_t> FocusModeMap = {
    {"OHOS_CAMERA_FOCUS_MODE_MANUAL",          OHOS_CAMERA_FOCUS_MODE_MANUAL         },
    {"OHOS_CAMERA_FOCUS_MODE_CONTINUOUS_AUTO", OHOS_CAMERA_FOCUS_MODE_CONTINUOUS_AUTO},
    {"OHOS_CAMERA_FOCUS_MODE_AUTO",            OHOS_CAMERA_FOCUS_MODE_AUTO           },
    {"OHOS_CAMERA_FOCUS_MODE_LOCKED",          OHOS_CAMERA_FOCUS_MODE_LOCKED         },
};

std::map<std::string, camera_flash_mode_enum_t> FlashModeMap = {
    {"OHOS_CAMERA_FLASH_MODE_CLOSE",       OHOS_CAMERA_FLASH_MODE_CLOSE      },
    {"OHOS_CAMERA_FLASH_MODE_OPEN",        OHOS_CAMERA_FLASH_MODE_OPEN       },
    {"OHOS_CAMERA_FLASH_MODE_AUTO",        OHOS_CAMERA_FLASH_MODE_AUTO       },
    {"OHOS_CAMERA_FLASH_MODE_ALWAYS_OPEN", OHOS_CAMERA_FLASH_MODE_ALWAYS_OPEN},
};

std::map<std::string, camera_meter_mode_t> meterModeMap = {
    {"OHOS_CAMERA_SPOT_METERING",    OHOS_CAMERA_SPOT_METERING   },
    {"OHOS_CAMERA_REGION_METERING",  OHOS_CAMERA_REGION_METERING },
    {"OHOS_CAMERA_OVERALL_METERING", OHOS_CAMERA_OVERALL_METERING},
};

std::map<std::string, camera_mirror_t> mirrorMap = {
    {"OHOS_CAMERA_MIRROR_OFF", OHOS_CAMERA_MIRROR_OFF},
    {"OHOS_CAMERA_MIRROR_ON",  OHOS_CAMERA_MIRROR_ON }
};

std::map<std::string, CameraVideoStabilizationMode> videoStabilizationMap = {
    {"OHOS_CAMERA_VIDEO_STABILIZATION_OFF",    OHOS_CAMERA_VIDEO_STABILIZATION_OFF   },
    {"OHOS_CAMERA_VIDEO_STABILIZATION_LOW",    OHOS_CAMERA_VIDEO_STABILIZATION_LOW   },
    {"OHOS_CAMERA_VIDEO_STABILIZATION_MIDDLE", OHOS_CAMERA_VIDEO_STABILIZATION_MIDDLE},
    {"OHOS_CAMERA_VIDEO_STABILIZATION_HIGH",   OHOS_CAMERA_VIDEO_STABILIZATION_HIGH  },
    {"OHOS_CAMERA_VIDEO_STABILIZATION_AUTO",   OHOS_CAMERA_VIDEO_STABILIZATION_AUTO  },
};

std::map<std::string, CameraFlashAvailable> flashAvailableMap = {
    {"OHOS_CAMERA_FLASH_FALSE", OHOS_CAMERA_FLASH_FALSE},
    {"OHOS_CAMERA_FLASH_TRUE",  OHOS_CAMERA_FLASH_TRUE }
};

std::vector<camera_format_t> formatArray = {
    OHOS_CAMERA_FORMAT_RGBA_8888, OHOS_CAMERA_FORMAT_YCBCR_420_888,
    OHOS_CAMERA_FORMAT_YCRCB_420_SP, OHOS_CAMERA_FORMAT_JPEG
};

std::map<std::string, camera_ae_antibanding_mode_t> AeAntibandingModeMap = {
    {"OHOS_CAMERA_AE_ANTIBANDING_MODE_OFF",  OHOS_CAMERA_AE_ANTIBANDING_MODE_OFF },
    {"OHOS_CAMERA_AE_ANTIBANDING_MODE_50HZ", OHOS_CAMERA_AE_ANTIBANDING_MODE_50HZ},
    {"OHOS_CAMERA_AE_ANTIBANDING_MODE_60HZ", OHOS_CAMERA_AE_ANTIBANDING_MODE_60HZ},
    {"OHOS_CAMERA_AE_ANTIBANDING_MODE_AUTO", OHOS_CAMERA_AE_ANTIBANDING_MODE_AUTO},
};

std::map<std::string, camera_ae_lock_t> AeLockMap = {
    {"OHOS_CAMERA_AE_LOCK_OFF", OHOS_CAMERA_AE_LOCK_OFF},
    {"OHOS_CAMERA_AE_LOCK_ON",  OHOS_CAMERA_AE_LOCK_ON },
};

std::map<std::string, camera_ae_mode_t> AeModeMap = {
    {"OHOS_CAMERA_AE_MODE_OFF",                  OHOS_CAMERA_AE_MODE_OFF                 },
    {"OHOS_CAMERA_AE_MODE_ON",                   OHOS_CAMERA_AE_MODE_ON                  },
    {"OHOS_CAMERA_AE_MODE_ON_AUTO_FLASH",        OHOS_CAMERA_AE_MODE_ON_AUTO_FLASH       },
    {"OHOS_CAMERA_AE_MODE_ON_ALWAYS_FLASH",      OHOS_CAMERA_AE_MODE_ON_ALWAYS_FLASH     },
    {"OHOS_CAMERA_AE_MODE_ON_AUTO_FLASH_REDEYE", OHOS_CAMERA_AE_MODE_ON_AUTO_FLASH_REDEYE},
    {"OHOS_CAMERA_AE_MODE_ON_EXTERNAL_FLASH",    OHOS_CAMERA_AE_MODE_ON_EXTERNAL_FLASH   },
};

std::map<std::string, camera_af_mode_t> AfModeMap = {
    {"OHOS_CAMERA_AF_MODE_OFF",                OHOS_CAMERA_AF_MODE_OFF               },
    {"OHOS_CAMERA_AF_MODE_AUTO",               OHOS_CAMERA_AF_MODE_AUTO              },
    {"OHOS_CAMERA_AF_MODE_MACRO",              OHOS_CAMERA_AF_MODE_MACRO             },
    {"OHOS_CAMERA_AF_MODE_CONTINUOUS_VIDEO",   OHOS_CAMERA_AF_MODE_CONTINUOUS_VIDEO  },
    {"OHOS_CAMERA_AF_MODE_CONTINUOUS_PICTURE", OHOS_CAMERA_AF_MODE_CONTINUOUS_PICTURE},
    {"OHOS_CAMERA_AF_MODE_EDOF",               OHOS_CAMERA_AF_MODE_EDOF              },
};

std::map<std::string, camera_awb_lock_t> AwbLockMap = {
    {"OHOS_CAMERA_AWB_LOCK_OFF", OHOS_CAMERA_AWB_LOCK_OFF},
    {"OHOS_CAMERA_AWB_LOCK_ON",  OHOS_CAMERA_AWB_LOCK_ON },
};

std::map<std::string, camera_awb_mode_t> AwbModeMap = {
    {"OHOS_CAMERA_AWB_MODE_OFF",              OHOS_CAMERA_AWB_MODE_OFF             },
    {"OHOS_CAMERA_AWB_MODE_AUTO",             OHOS_CAMERA_AWB_MODE_AUTO            },
    {"OHOS_CAMERA_AWB_MODE_INCANDESCENT",     OHOS_CAMERA_AWB_MODE_INCANDESCENT    },
    {"OHOS_CAMERA_AWB_MODE_FLUORESCENT",      OHOS_CAMERA_AWB_MODE_FLUORESCENT     },
    {"OHOS_CAMERA_AWB_MODE_WARM_FLUORESCENT", OHOS_CAMERA_AWB_MODE_WARM_FLUORESCENT},
    {"OHOS_CAMERA_AWB_MODE_DAYLIGHT",         OHOS_CAMERA_AWB_MODE_DAYLIGHT        },
    {"OHOS_CAMERA_AWB_MODE_CLOUDY_DAYLIGHT",  OHOS_CAMERA_AWB_MODE_CLOUDY_DAYLIGHT },
    {"OHOS_CAMERA_AWB_MODE_TWILIGHT",         OHOS_CAMERA_AWB_MODE_TWILIGHT        },
    {"OHOS_CAMERA_AWB_MODE_SHADE",            OHOS_CAMERA_AWB_MODE_SHADE           },
};

std::map<std::string, camera_face_detect_mode_t> FaceDetectModeMap = {
    {"OHOS_CAMERA_FACE_DETECT_MODE_OFF",    OHOS_CAMERA_FACE_DETECT_MODE_OFF   },
    {"OHOS_CAMERA_FACE_DETECT_MODE_SIMPLE", OHOS_CAMERA_FACE_DETECT_MODE_SIMPLE},
};

std::map<std::string, camera_histogram_mode_t> HistogramModeMap = {
    {"OHOS_CAMERA_HISTOGRAM_MODE_OFF", OHOS_CAMERA_HISTOGRAM_MODE_OFF},
    {"OHOS_CAMERA_HISTOGRAM_MODE_ON",  OHOS_CAMERA_HISTOGRAM_MODE_ON },
};

std::map<std::string, camera_memory_type_enum_t> CameraMemoryTypeMap = {
    {"OHOS_CAMERA_MEMORY_MMAP", OHOS_CAMERA_MEMORY_MMAP},
    {"OHOS_CAMERA_MEMORY_USERPTR", OHOS_CAMERA_MEMORY_USERPTR},
    {"OHOS_CAMERA_MEMORY_OVERLAY", OHOS_CAMERA_MEMORY_OVERLAY},
    {"OHOS_CAMERA_MEMORY_DMABUF", OHOS_CAMERA_MEMORY_DMABUF},
};

#endif /* CAMERA_HOST_METADATA_ENUM_MAP_H */
