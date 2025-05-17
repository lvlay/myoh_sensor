/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef LIGHT_AGENT_TYPE_H
#define LIGHT_AGENT_TYPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum length of the light name */
#ifndef NAME_MAX_LEN
#define NAME_MAX_LEN 32
#endif /* NAME_MAX_LEN */
/**
 * @brief Enumerates the light types.
 *
 * @since 10
 */
enum LightType {
    LIGHT_TYPE_SINGLE_COLOR = 1,
    LIGHT_TYPE_RGB_COLOR = 2,
    LIGHT_TYPE_WRGB_COLOR = 3,
};

/**
 * @brief Defines the basic light information.
 *
 * @since 10
 */
typedef struct {
    char lightName[NAME_MAX_LEN]; /**< light name */
    int32_t lightId; /**< Light id */
    int32_t lightNumber; /**< Number of physical lights in the logic controller */
    int32_t lightType; /** Light type. For details, see {@link LightType}. */
} LightInfo;

/**
 * @brief Enumerates the lighting modes.
 *
 * @since 10
 */
typedef enum {
    LIGHT_MODE_DEFAULT = 0, /**< No flicker mode. */
    LIGHT_MODE_BLINK = 1, /**< Timed flashing mode. */
    LIGHT_MODE_GRADIENT = 2, /**< Gradient mode. */
    LIGHT_MODE_BUTT = 3, /**< Invalid mode. */
} FlashMode;

/**
 * @brief Defines the blinking parameters.
 *
 * @since 10
 */
struct LightAnimation {
    int32_t mode = LIGHT_MODE_DEFAULT; /**< Blinking mode. For details, see {@link FlashMode}. */
    int32_t onTime; /**<  Duration (in ms) for which the light is on in a blinking period. */
    int32_t offTime; /**< Duration (in ms) for which the light is off in a blinking period. */
};

/**
 * @brief Defines the color and custom extended information of the light.
 *
 * The parameters include the These parameters include red, green, blue values and custom extended information.
 *
 * @since 10
 */
struct RGBColor {
    uint8_t r;          /** Red value range 0-255. */
    uint8_t g;          /** Green value range 0-255. */
    uint8_t b;          /** Blue value range 0-255. */
    uint8_t reserved;   /** Custom extended information */
};

/**
 * @brief Defines the color of the light.
 *
 * The parameters include the These parameters include red, green, blue and white values.
 ght idNumber of physical lights in the logic controllerint reservedCustom extended information.
 * @since 10
 */
struct WRGBColor {
    uint8_t w;    /** White value range 0-255. */
    uint8_t r;    /** Red value range 0-255. */
    uint8_t g;    /** Green value range 0-255. */
    uint8_t b;    /** Blue value range 0-255. */
};

/**
 * @brief Defines the color of the light。
 *
 * @since 10
 */
union LightColor {
    int32_t singleColor; /** Single color mode. */
    RGBColor rgbColor; /** RGB mode, see {@link RGBColor}. */
    WRGBColor wrgbColor; /** WRGB mode, see {@link WRGBColor}. */
};

/** @} */
#ifdef __cplusplus
};
#endif
#endif  // endif LIGHT_AGENT_TYPE_H
