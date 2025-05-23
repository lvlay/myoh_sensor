/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "jd9365_boe.h"
#include "gpio_if.h"
#include "hdf_bl.h"
#include "hdf_disp.h"
#include "osal.h"

/* panel on command payload */
static uint8_t g_payLoad0[] = { 0x11 };
static uint8_t g_payLoad1[] = { 0xff, 0x98, 0x81, 0x03 };
static uint8_t g_payLoad2[] = { 0x01, 0x00 };
static uint8_t g_payLoad3[] = { 0x02, 0x00 };
static uint8_t g_payLoad4[] = { 0x03, 0x72 };
static uint8_t g_payLoad5[] = { 0x04, 0x00 };
static uint8_t g_payLoad6[] = { 0x05, 0x00 };
static uint8_t g_payLoad7[] = { 0x06, 0x09 };
static uint8_t g_payLoad8[] = { 0x07, 0x00 };
static uint8_t g_payLoad9[] = { 0x08, 0x00 };
static uint8_t g_payLoad10[] = { 0x09, 0x01 };
static uint8_t g_payLoad11[] = { 0x0A, 0x00 };
static uint8_t g_payLoad12[] = { 0x0B, 0x00 };
static uint8_t g_payLoad13[] = { 0x0C, 0x01 };
static uint8_t g_payLoad14[] = { 0x0D, 0x00 };
static uint8_t g_payLoad15[] = { 0x0E, 0x00 };
static uint8_t g_payLoad16[] = { 0x0F, 0x00 };
static uint8_t g_payLoad17[] = { 0x10, 0x00 };
static uint8_t g_payLoad18[] = { 0x11, 0x00 };
static uint8_t g_payLoad19[] = { 0x12, 0x00 };
static uint8_t g_payLoad20[] = { 0x13, 0x00 };
static uint8_t g_payLoad21[] = { 0x14, 0x00 };
static uint8_t g_payLoad22[] = { 0x15, 0x00 };
static uint8_t g_payLoad23[] = { 0x16, 0x00 };
static uint8_t g_payLoad24[] = { 0x17, 0x00 };
static uint8_t g_payLoad25[] = { 0x18, 0x00 };
static uint8_t g_payLoad26[] = { 0x19, 0x00 };
static uint8_t g_payLoad27[] = { 0x1A, 0x00 };
static uint8_t g_payLoad28[] = { 0x1B, 0x00 };
static uint8_t g_payLoad29[] = { 0x1C, 0x00 };
static uint8_t g_payLoad30[] = { 0x1D, 0x00 };
static uint8_t g_payLoad31[] = { 0x1E, 0x40 };
static uint8_t g_payLoad32[] = { 0x1F, 0x80 };
static uint8_t g_payLoad33[] = { 0x20, 0x05 };
static uint8_t g_payLoad34[] = { 0x21, 0x02 };
static uint8_t g_payLoad35[] = { 0x22, 0x00 };
static uint8_t g_payLoad36[] = { 0x23, 0x00 };
static uint8_t g_payLoad37[] = { 0x24, 0x00 };
static uint8_t g_payLoad38[] = { 0x25, 0x00 };
static uint8_t g_payLoad39[] = { 0x26, 0x00 };
static uint8_t g_payLoad40[] = { 0x27, 0x00 };
static uint8_t g_payLoad41[] = { 0x28, 0x33 };
static uint8_t g_payLoad42[] = { 0x29, 0x02 };
static uint8_t g_payLoad43[] = { 0x2A, 0x00 };
static uint8_t g_payLoad44[] = { 0x2B, 0x00 };
static uint8_t g_payLoad45[] = { 0x2C, 0x00 };
static uint8_t g_payLoad46[] = { 0x2D, 0x00 };
static uint8_t g_payLoad47[] = { 0x2E, 0x00 };
static uint8_t g_payLoad48[] = { 0x2F, 0x00 };
static uint8_t g_payLoad49[] = { 0x30, 0x00 };
static uint8_t g_payLoad50[] = { 0x31, 0x00 };
static uint8_t g_payLoad51[] = { 0x32, 0x00 };
static uint8_t g_payLoad52[] = { 0x33, 0x00 };
static uint8_t g_payLoad53[] = { 0x34, 0x04 };
static uint8_t g_payLoad54[] = { 0x35, 0x00 };
static uint8_t g_payLoad55[] = { 0x36, 0x00 };
static uint8_t g_payLoad56[] = { 0x37, 0x00 };
static uint8_t g_payLoad57[] = { 0x38, 0x3C };
static uint8_t g_payLoad58[] = { 0x39, 0x00 };
static uint8_t g_payLoad59[] = { 0x3A, 0x40 };
static uint8_t g_payLoad60[] = { 0x3B, 0x40 };
static uint8_t g_payLoad61[] = { 0x3C, 0x00 };
static uint8_t g_payLoad62[] = { 0x3D, 0x00 };
static uint8_t g_payLoad63[] = { 0x3E, 0x00 };
static uint8_t g_payLoad64[] = { 0x3F, 0x00 };
static uint8_t g_payLoad65[] = { 0x40, 0x00 };
static uint8_t g_payLoad66[] = { 0x41, 0x00 };
static uint8_t g_payLoad67[] = { 0x42, 0x00 };
static uint8_t g_payLoad68[] = { 0x43, 0x00 };
static uint8_t g_payLoad69[] = { 0x44, 0x00 };
static uint8_t g_payLoad70[] = { 0x50, 0x01 };
static uint8_t g_payLoad71[] = { 0x51, 0x23 };
static uint8_t g_payLoad72[] = { 0x52, 0x45 };
static uint8_t g_payLoad73[] = { 0x53, 0x67 };
static uint8_t g_payLoad74[] = { 0x54, 0x89 };
static uint8_t g_payLoad75[] = { 0x55, 0xAB };
static uint8_t g_payLoad76[] = { 0x56, 0x01 };
static uint8_t g_payLoad77[] = { 0x57, 0x23 };
static uint8_t g_payLoad78[] = { 0x58, 0x45 };
static uint8_t g_payLoad79[] = { 0x59, 0x67 };
static uint8_t g_payLoad80[] = { 0x5A, 0x89 };
static uint8_t g_payLoad81[] = { 0x5B, 0xAB };
static uint8_t g_payLoad82[] = { 0x5C, 0xCD };
static uint8_t g_payLoad83[] = { 0x5D, 0xEF };
static uint8_t g_payLoad84[] = { 0x5E, 0x11 };
static uint8_t g_payLoad85[] = { 0x5F, 0x01 };
static uint8_t g_payLoad86[] = { 0x60, 0x00 };
static uint8_t g_payLoad87[] = { 0x61, 0x15 };
static uint8_t g_payLoad88[] = { 0x62, 0x14 };
static uint8_t g_payLoad89[] = { 0x63, 0x0E };
static uint8_t g_payLoad90[] = { 0x64, 0x0F };
static uint8_t g_payLoad91[] = { 0x65, 0x0C };
static uint8_t g_payLoad92[] = { 0x66, 0x0D };
static uint8_t g_payLoad93[] = { 0x67, 0x06 };
static uint8_t g_payLoad94[] = { 0x68, 0x02 };
static uint8_t g_payLoad95[] = { 0x69, 0x02 };
static uint8_t g_payLoad96[] = { 0x6A, 0x02 };
static uint8_t g_payLoad97[] = { 0x6B, 0x02 };
static uint8_t g_payLoad98[] = { 0x6C, 0x02 };
static uint8_t g_payLoad99[] = { 0x6D, 0x02 };
static uint8_t g_payLoad100[] = { 0x6E, 0x07 };
static uint8_t g_payLoad101[] = { 0x6F, 0x02 };
static uint8_t g_payLoad102[] = { 0x70, 0x02 };
static uint8_t g_payLoad103[] = { 0x71, 0x02 };
static uint8_t g_payLoad104[] = { 0x72, 0x02 };
static uint8_t g_payLoad105[] = { 0x73, 0x02 };
static uint8_t g_payLoad106[] = { 0x74, 0x02 };
static uint8_t g_payLoad107[] = { 0x75, 0x01 };
static uint8_t g_payLoad108[] = { 0x76, 0x00 };
static uint8_t g_payLoad109[] = { 0x77, 0x14 };
static uint8_t g_payLoad110[] = { 0x78, 0x15 };
static uint8_t g_payLoad111[] = { 0x79, 0x0E };
static uint8_t g_payLoad112[] = { 0x7A, 0x0F };
static uint8_t g_payLoad113[] = { 0x7B, 0x0C };
static uint8_t g_payLoad114[] = { 0x7C, 0x0D };
static uint8_t g_payLoad115[] = { 0x7D, 0x06 };
static uint8_t g_payLoad116[] = { 0x7E, 0x02 };
static uint8_t g_payLoad117[] = { 0x7F, 0x02 };
static uint8_t g_payLoad118[] = { 0x80, 0x02 };
static uint8_t g_payLoad119[] = { 0x81, 0x02 };
static uint8_t g_payLoad120[] = { 0x82, 0x02 };
static uint8_t g_payLoad121[] = { 0x83, 0x02 };
static uint8_t g_payLoad122[] = { 0x84, 0x07 };
static uint8_t g_payLoad123[] = { 0x85, 0x02 };
static uint8_t g_payLoad124[] = { 0x86, 0x02 };
static uint8_t g_payLoad125[] = { 0x87, 0x02 };
static uint8_t g_payLoad126[] = { 0x88, 0x02 };
static uint8_t g_payLoad127[] = { 0x89, 0x02 };
static uint8_t g_payLoad128[] = { 0x8A, 0x02 };
static uint8_t g_payLoad129[] = { 0xFF, 0x98, 0x81, 0x04 };
static uint8_t g_payLoad130[] = { 0x6C, 0x15 };
static uint8_t g_payLoad131[] = { 0x6E, 0x2A };
static uint8_t g_payLoad132[] = { 0x6F, 0x33 };
static uint8_t g_payLoad133[] = { 0x3A, 0x94 };
static uint8_t g_payLoad134[] = { 0x8D, 0x1A };
static uint8_t g_payLoad135[] = { 0x87, 0xBA };
static uint8_t g_payLoad136[] = { 0x26, 0x76 };
static uint8_t g_payLoad137[] = { 0xB2, 0xD1 };
static uint8_t g_payLoad138[] = { 0xB5, 0x06 };
static uint8_t g_payLoad139[] = { 0xFF, 0x98, 0x81, 0x01 };
static uint8_t g_payLoad140[] = { 0x22, 0x0A };
static uint8_t g_payLoad141[] = { 0x31, 0x00 };
static uint8_t g_payLoad142[] = { 0x40, 0x13 };
static uint8_t g_payLoad143[] = { 0x53, 0x84 };
static uint8_t g_payLoad144[] = { 0x55, 0x8F };
static uint8_t g_payLoad145[] = { 0x50, 0xAE };
static uint8_t g_payLoad146[] = { 0x51, 0xAE };
static uint8_t g_payLoad147[] = { 0x60, 0x28 };
static uint8_t g_payLoad148[] = { 0xA0, 0x0f };
static uint8_t g_payLoad149[] = { 0xA1, 0x1B };
static uint8_t g_payLoad150[] = { 0xA2, 0x28 };
static uint8_t g_payLoad151[] = { 0xA3, 0x12 };
static uint8_t g_payLoad152[] = { 0xA4, 0x15 };
static uint8_t g_payLoad153[] = { 0xA5, 0x28 };
static uint8_t g_payLoad154[] = { 0xA6, 0x1B };
static uint8_t g_payLoad155[] = { 0xA7, 0x1E };
static uint8_t g_payLoad156[] = { 0xA8, 0x79 };
static uint8_t g_payLoad157[] = { 0xA9, 0x1B };
static uint8_t g_payLoad158[] = { 0xAA, 0x27 };
static uint8_t g_payLoad159[] = { 0xAB, 0x69 };
static uint8_t g_payLoad160[] = { 0xAC, 0x19 };
static uint8_t g_payLoad161[] = { 0xAD, 0x18 };
static uint8_t g_payLoad162[] = { 0xAE, 0x4C };
static uint8_t g_payLoad163[] = { 0xAF, 0x21 };
static uint8_t g_payLoad164[] = { 0xB0, 0x28 };
static uint8_t g_payLoad165[] = { 0xB1, 0x52 };
static uint8_t g_payLoad166[] = { 0xB2, 0x65 };
static uint8_t g_payLoad167[] = { 0xC0, 0x04 };
static uint8_t g_payLoad168[] = { 0xC1, 0x1B };
static uint8_t g_payLoad169[] = { 0xC2, 0x27 };
static uint8_t g_payLoad170[] = { 0xC3, 0x13 };
static uint8_t g_payLoad171[] = { 0xC4, 0x15 };
static uint8_t g_payLoad172[] = { 0xC5, 0x28 };
static uint8_t g_payLoad173[] = { 0xC6, 0x1C };
static uint8_t g_payLoad174[] = { 0xC7, 0x1E };
static uint8_t g_payLoad175[] = { 0xC8, 0x79 };
static uint8_t g_payLoad176[] = { 0xC9, 0x1A };
static uint8_t g_payLoad177[] = { 0xCA, 0x27 };
static uint8_t g_payLoad178[] = { 0xCB, 0x69 };
static uint8_t g_payLoad179[] = { 0xCC, 0x1A };
static uint8_t g_payLoad180[] = { 0xCD, 0x18 };
static uint8_t g_payLoad181[] = { 0xCE, 0x4C };
static uint8_t g_payLoad182[] = { 0xCF, 0x21 };
static uint8_t g_payLoad183[] = { 0xD0, 0x27 };
static uint8_t g_payLoad184[] = { 0xD1, 0x52 };
static uint8_t g_payLoad185[] = { 0xD2, 0x65 };
static uint8_t g_payLoad186[] = { 0xD3, 0x3F };
static uint8_t g_payLoad187[] = { 0xFF, 0x98, 0x81, 0x00 };
static uint8_t g_payLoad188[] = { 0x29 };
static uint8_t g_payLoad189[] = { 0x35, 0x00 };

static struct DsiCmdDesc g_panelOnCode[] = {
    { 0x05, 0x64, sizeof(g_payLoad0), g_payLoad0 },
    { 0x39, 0x00, sizeof(g_payLoad1), g_payLoad1 },
    { 0x15, 0x00, sizeof(g_payLoad2), g_payLoad2 },
    { 0x15, 0x00, sizeof(g_payLoad3), g_payLoad3 },
    { 0x15, 0x00, sizeof(g_payLoad4), g_payLoad4 },
    { 0x15, 0x00, sizeof(g_payLoad5), g_payLoad5 },
    { 0x15, 0x00, sizeof(g_payLoad6), g_payLoad6 },
    { 0x15, 0x00, sizeof(g_payLoad7), g_payLoad7 },
    { 0x15, 0x00, sizeof(g_payLoad8), g_payLoad8 },
    { 0x15, 0x00, sizeof(g_payLoad9), g_payLoad9 },
    { 0x15, 0x00, sizeof(g_payLoad10), g_payLoad10 },
    { 0x15, 0x00, sizeof(g_payLoad11), g_payLoad11 },
    { 0x15, 0x00, sizeof(g_payLoad12), g_payLoad12 },
    { 0x15, 0x00, sizeof(g_payLoad13), g_payLoad13 },
    { 0x15, 0x00, sizeof(g_payLoad14), g_payLoad14 },
    { 0x15, 0x00, sizeof(g_payLoad15), g_payLoad15 },
    { 0x15, 0x00, sizeof(g_payLoad16), g_payLoad16 },
    { 0x15, 0x00, sizeof(g_payLoad17), g_payLoad17 },
    { 0x15, 0x00, sizeof(g_payLoad18), g_payLoad18 },
    { 0x15, 0x00, sizeof(g_payLoad19), g_payLoad19 },
    { 0x15, 0x00, sizeof(g_payLoad20), g_payLoad20 },
    { 0x15, 0x00, sizeof(g_payLoad21), g_payLoad21 },
    { 0x15, 0x00, sizeof(g_payLoad22), g_payLoad22 },
    { 0x15, 0x00, sizeof(g_payLoad23), g_payLoad23 },
    { 0x15, 0x00, sizeof(g_payLoad24), g_payLoad24 },
    { 0x15, 0x00, sizeof(g_payLoad25), g_payLoad25 },
    { 0x15, 0x00, sizeof(g_payLoad26), g_payLoad26 },
    { 0x15, 0x00, sizeof(g_payLoad27), g_payLoad27 },
    { 0x15, 0x00, sizeof(g_payLoad28), g_payLoad28 },
    { 0x15, 0x00, sizeof(g_payLoad29), g_payLoad29 },
    { 0x15, 0x00, sizeof(g_payLoad30), g_payLoad30 },
    { 0x15, 0x00, sizeof(g_payLoad31), g_payLoad31 },
    { 0x15, 0x00, sizeof(g_payLoad32), g_payLoad32 },
    { 0x15, 0x00, sizeof(g_payLoad33), g_payLoad33 },
    { 0x15, 0x00, sizeof(g_payLoad34), g_payLoad34 },
    { 0x15, 0x00, sizeof(g_payLoad35), g_payLoad35 },
    { 0x15, 0x00, sizeof(g_payLoad36), g_payLoad36 },
    { 0x15, 0x00, sizeof(g_payLoad37), g_payLoad37 },
    { 0x15, 0x00, sizeof(g_payLoad38), g_payLoad38 },
    { 0x15, 0x00, sizeof(g_payLoad39), g_payLoad39 },
    { 0x15, 0x00, sizeof(g_payLoad40), g_payLoad40 },
    { 0x15, 0x00, sizeof(g_payLoad41), g_payLoad41 },
    { 0x15, 0x00, sizeof(g_payLoad42), g_payLoad42 },
    { 0x15, 0x00, sizeof(g_payLoad43), g_payLoad43 },
    { 0x15, 0x00, sizeof(g_payLoad44), g_payLoad44 },
    { 0x15, 0x00, sizeof(g_payLoad45), g_payLoad45 },
    { 0x15, 0x00, sizeof(g_payLoad46), g_payLoad46 },
    { 0x15, 0x00, sizeof(g_payLoad47), g_payLoad47 },
    { 0x15, 0x00, sizeof(g_payLoad48), g_payLoad48 },
    { 0x15, 0x00, sizeof(g_payLoad49), g_payLoad49 },
    { 0x15, 0x00, sizeof(g_payLoad50), g_payLoad50 },
    { 0x15, 0x00, sizeof(g_payLoad51), g_payLoad51 },
    { 0x15, 0x00, sizeof(g_payLoad52), g_payLoad52 },
    { 0x15, 0x00, sizeof(g_payLoad53), g_payLoad53 },
    { 0x15, 0x00, sizeof(g_payLoad54), g_payLoad54 },
    { 0x15, 0x00, sizeof(g_payLoad55), g_payLoad55 },
    { 0x15, 0x00, sizeof(g_payLoad56), g_payLoad56 },
    { 0x15, 0x00, sizeof(g_payLoad57), g_payLoad57 },
    { 0x15, 0x00, sizeof(g_payLoad58), g_payLoad58 },
    { 0x15, 0x00, sizeof(g_payLoad59), g_payLoad59 },
    { 0x15, 0x00, sizeof(g_payLoad60), g_payLoad60 },
    { 0x15, 0x00, sizeof(g_payLoad61), g_payLoad61 },
    { 0x15, 0x00, sizeof(g_payLoad62), g_payLoad62 },
    { 0x15, 0x00, sizeof(g_payLoad63), g_payLoad63 },
    { 0x15, 0x00, sizeof(g_payLoad64), g_payLoad64 },
    { 0x15, 0x00, sizeof(g_payLoad65), g_payLoad65 },
    { 0x15, 0x00, sizeof(g_payLoad66), g_payLoad66 },
    { 0x15, 0x00, sizeof(g_payLoad67), g_payLoad67 },
    { 0x15, 0x00, sizeof(g_payLoad68), g_payLoad68 },
    { 0x15, 0x00, sizeof(g_payLoad69), g_payLoad69 },
    { 0x15, 0x00, sizeof(g_payLoad70), g_payLoad70 },
    { 0x15, 0x00, sizeof(g_payLoad71), g_payLoad71 },
    { 0x15, 0x00, sizeof(g_payLoad72), g_payLoad72 },
    { 0x15, 0x00, sizeof(g_payLoad73), g_payLoad73 },
    { 0x15, 0x00, sizeof(g_payLoad74), g_payLoad74 },
    { 0x15, 0x00, sizeof(g_payLoad75), g_payLoad75 },
    { 0x15, 0x00, sizeof(g_payLoad76), g_payLoad76 },
    { 0x15, 0x00, sizeof(g_payLoad77), g_payLoad77 },
    { 0x15, 0x00, sizeof(g_payLoad78), g_payLoad78 },
    { 0x15, 0x00, sizeof(g_payLoad79), g_payLoad79 },
    { 0x15, 0x00, sizeof(g_payLoad80), g_payLoad80 },
    { 0x15, 0x00, sizeof(g_payLoad81), g_payLoad81 },
    { 0x15, 0x00, sizeof(g_payLoad82), g_payLoad82 },
    { 0x15, 0x00, sizeof(g_payLoad83), g_payLoad83 },
    { 0x15, 0x00, sizeof(g_payLoad84), g_payLoad84 },
    { 0x15, 0x00, sizeof(g_payLoad85), g_payLoad85 },
    { 0x15, 0x00, sizeof(g_payLoad86), g_payLoad86 },
    { 0x15, 0x00, sizeof(g_payLoad87), g_payLoad87 },
    { 0x15, 0x00, sizeof(g_payLoad88), g_payLoad88 },
    { 0x15, 0x00, sizeof(g_payLoad89), g_payLoad89 },
    { 0x15, 0x00, sizeof(g_payLoad90), g_payLoad90 },
    { 0x15, 0x00, sizeof(g_payLoad91), g_payLoad91 },
    { 0x15, 0x00, sizeof(g_payLoad92), g_payLoad92 },
    { 0x15, 0x00, sizeof(g_payLoad93), g_payLoad93 },
    { 0x15, 0x00, sizeof(g_payLoad94), g_payLoad94 },
    { 0x15, 0x00, sizeof(g_payLoad95), g_payLoad95 },
    { 0x15, 0x00, sizeof(g_payLoad96), g_payLoad96 },
    { 0x15, 0x00, sizeof(g_payLoad97), g_payLoad97 },
    { 0x15, 0x00, sizeof(g_payLoad98), g_payLoad98 },
    { 0x15, 0x00, sizeof(g_payLoad99), g_payLoad99 },
    { 0x15, 0x00, sizeof(g_payLoad100), g_payLoad100 },
    { 0x15, 0x00, sizeof(g_payLoad101), g_payLoad101 },
    { 0x15, 0x00, sizeof(g_payLoad102), g_payLoad102 },
    { 0x15, 0x00, sizeof(g_payLoad103), g_payLoad103 },
    { 0x15, 0x00, sizeof(g_payLoad104), g_payLoad104 },
    { 0x15, 0x00, sizeof(g_payLoad105), g_payLoad105 },
    { 0x15, 0x00, sizeof(g_payLoad106), g_payLoad106 },
    { 0x15, 0x00, sizeof(g_payLoad107), g_payLoad107 },
    { 0x15, 0x00, sizeof(g_payLoad108), g_payLoad108 },
    { 0x15, 0x00, sizeof(g_payLoad109), g_payLoad109 },
    { 0x15, 0x00, sizeof(g_payLoad110), g_payLoad110 },
    { 0x15, 0x00, sizeof(g_payLoad111), g_payLoad111 },
    { 0x15, 0x00, sizeof(g_payLoad112), g_payLoad112 },
    { 0x15, 0x00, sizeof(g_payLoad113), g_payLoad113 },
    { 0x15, 0x00, sizeof(g_payLoad114), g_payLoad114 },
    { 0x15, 0x00, sizeof(g_payLoad115), g_payLoad115 },
    { 0x15, 0x00, sizeof(g_payLoad116), g_payLoad116 },
    { 0x15, 0x00, sizeof(g_payLoad117), g_payLoad117 },
    { 0x15, 0x00, sizeof(g_payLoad118), g_payLoad118 },
    { 0x15, 0x00, sizeof(g_payLoad119), g_payLoad119 },
    { 0x15, 0x00, sizeof(g_payLoad120), g_payLoad120 },
    { 0x15, 0x00, sizeof(g_payLoad121), g_payLoad121 },
    { 0x15, 0x00, sizeof(g_payLoad122), g_payLoad122 },
    { 0x15, 0x00, sizeof(g_payLoad123), g_payLoad123 },
    { 0x15, 0x00, sizeof(g_payLoad124), g_payLoad124 },
    { 0x15, 0x00, sizeof(g_payLoad125), g_payLoad125 },
    { 0x15, 0x00, sizeof(g_payLoad126), g_payLoad126 },
    { 0x15, 0x00, sizeof(g_payLoad127), g_payLoad127 },
    { 0x15, 0x00, sizeof(g_payLoad128), g_payLoad128 },
    { 0x39, 0x00, sizeof(g_payLoad129), g_payLoad129 },
    { 0x15, 0x00, sizeof(g_payLoad130), g_payLoad130 },
    { 0x15, 0x00, sizeof(g_payLoad131), g_payLoad131 },
    { 0x15, 0x00, sizeof(g_payLoad132), g_payLoad132 },
    { 0x15, 0x00, sizeof(g_payLoad133), g_payLoad133 },
    { 0x15, 0x00, sizeof(g_payLoad134), g_payLoad134 },
    { 0x15, 0x00, sizeof(g_payLoad135), g_payLoad135 },
    { 0x15, 0x00, sizeof(g_payLoad136), g_payLoad136 },
    { 0x15, 0x00, sizeof(g_payLoad137), g_payLoad137 },
    { 0x15, 0x00, sizeof(g_payLoad138), g_payLoad138 },
    { 0x39, 0x00, sizeof(g_payLoad139), g_payLoad139 },
    { 0x15, 0x00, sizeof(g_payLoad140), g_payLoad140 },
    { 0x15, 0x00, sizeof(g_payLoad141), g_payLoad141 },
    { 0x15, 0x00, sizeof(g_payLoad142), g_payLoad142 },
    { 0x15, 0x00, sizeof(g_payLoad143), g_payLoad143 },
    { 0x15, 0x00, sizeof(g_payLoad144), g_payLoad144 },
    { 0x15, 0x00, sizeof(g_payLoad145), g_payLoad145 },
    { 0x15, 0x00, sizeof(g_payLoad146), g_payLoad146 },
    { 0x15, 0x00, sizeof(g_payLoad147), g_payLoad147 },
    { 0x15, 0x00, sizeof(g_payLoad148), g_payLoad148 },
    { 0x15, 0x00, sizeof(g_payLoad149), g_payLoad149 },
    { 0x15, 0x00, sizeof(g_payLoad150), g_payLoad150 },
    { 0x15, 0x00, sizeof(g_payLoad151), g_payLoad151 },
    { 0x15, 0x00, sizeof(g_payLoad152), g_payLoad152 },
    { 0x15, 0x00, sizeof(g_payLoad153), g_payLoad153 },
    { 0x15, 0x00, sizeof(g_payLoad154), g_payLoad154 },
    { 0x15, 0x00, sizeof(g_payLoad155), g_payLoad155 },
    { 0x15, 0x00, sizeof(g_payLoad156), g_payLoad156 },
    { 0x15, 0x00, sizeof(g_payLoad157), g_payLoad157 },
    { 0x15, 0x00, sizeof(g_payLoad158), g_payLoad158 },
    { 0x15, 0x00, sizeof(g_payLoad159), g_payLoad159 },
    { 0x15, 0x00, sizeof(g_payLoad160), g_payLoad160 },
    { 0x15, 0x00, sizeof(g_payLoad161), g_payLoad161 },
    { 0x15, 0x00, sizeof(g_payLoad162), g_payLoad162 },
    { 0x15, 0x00, sizeof(g_payLoad163), g_payLoad163 },
    { 0x15, 0x00, sizeof(g_payLoad164), g_payLoad164 },
    { 0x15, 0x00, sizeof(g_payLoad165), g_payLoad165 },
    { 0x15, 0x00, sizeof(g_payLoad166), g_payLoad166 },
    { 0x15, 0x00, sizeof(g_payLoad167), g_payLoad167 },
    { 0x15, 0x00, sizeof(g_payLoad168), g_payLoad168 },
    { 0x15, 0x00, sizeof(g_payLoad169), g_payLoad169 },
    { 0x15, 0x00, sizeof(g_payLoad170), g_payLoad170 },
    { 0x15, 0x00, sizeof(g_payLoad171), g_payLoad171 },
    { 0x15, 0x00, sizeof(g_payLoad172), g_payLoad172 },
    { 0x15, 0x00, sizeof(g_payLoad173), g_payLoad173 },
    { 0x15, 0x00, sizeof(g_payLoad174), g_payLoad174 },
    { 0x15, 0x00, sizeof(g_payLoad175), g_payLoad175 },
    { 0x15, 0x00, sizeof(g_payLoad176), g_payLoad176 },
    { 0x15, 0x00, sizeof(g_payLoad177), g_payLoad177 },
    { 0x15, 0x00, sizeof(g_payLoad178), g_payLoad178 },
    { 0x15, 0x00, sizeof(g_payLoad179), g_payLoad179 },
    { 0x15, 0x00, sizeof(g_payLoad180), g_payLoad180 },
    { 0x15, 0x00, sizeof(g_payLoad181), g_payLoad181 },
    { 0x15, 0x00, sizeof(g_payLoad182), g_payLoad182 },
    { 0x15, 0x00, sizeof(g_payLoad183), g_payLoad183 },
    { 0x15, 0x00, sizeof(g_payLoad184), g_payLoad184 },
    { 0x15, 0x00, sizeof(g_payLoad185), g_payLoad185 },
    { 0x15, 0x00, sizeof(g_payLoad186), g_payLoad186 },
    { 0x39, 0x00, sizeof(g_payLoad187), g_payLoad187 },
    { 0x05, 0x32, sizeof(g_payLoad188), g_payLoad188 },
    { 0x15, 0x00, sizeof(g_payLoad189), g_payLoad189 },
};

/* panel off command payload */
static uint8_t g_offpayLoad0[] = { 0x11, 0x28 };
static uint8_t g_offpayLoad1[] = { 0x01, 0x10 };

static struct DsiCmdDesc g_panelOffCode[] = {
    { 0x05, 0x32, sizeof(g_offpayLoad0), g_offpayLoad0 },
    { 0x05, 0xC8, sizeof(g_offpayLoad1), g_offpayLoad1 },
};

struct panel_jd9365_dev *g_panel_dev = NULL;

static struct panel_jd9365_dev *ToPanelSimpleDev(const struct PanelData *panel)
{
    return (struct panel_jd9365_dev *)panel->object->priv;
}

static int32_t panel_simple_regulator_enable(void)
{
    int32_t err;
    if (g_panel_dev == NULL) {
        return -1;
    }
    err = regulator_enable(g_panel_dev->supply);
    if (err < 0) {
        HDF_LOGE("regulator_enable failed");
        return err;
    }
    return 0;
}
static int32_t panel_simple_regulator_disable(void)
{
    if (g_panel_dev == NULL) {
        return -1;
    }
    regulator_disable(g_panel_dev->supply);
    return 0;
}

int panel_simple_loader_protect(struct drm_panel *panel)
{
    int err;
    (void)panel;
    err = panel_simple_regulator_enable();
    if (err < 0) {
        HDF_LOGE("failed to enable supply: %d\n", err);
        return err;
    }
    return 0;
}
EXPORT_SYMBOL(panel_simple_loader_protect);

static int32_t PanelSendCmds(struct mipi_dsi_device *dsi,
    const struct DsiCmdDesc *cmds, int size)
{
    int32_t i = 0;

    HDF_LOGI("%s enter", __func__);
    if (dsi == NULL) {
        HDF_LOGE("dsi is NULL");
        return -EINVAL;
    }
    for (i = 0; i < size; i++) {
        mipi_dsi_dcs_write_buffer(dsi, cmds[i].payload, cmds[i].dataLen);
        if (cmds[i].delay) {
            OsalMSleep(cmds[i].delay);
        }
    }
    return HDF_SUCCESS;
}

static int32_t PanelOn(struct PanelData *panel)
{
    struct panel_jd9365_dev *panel_dev = NULL;

    panel_dev = ToPanelSimpleDev(panel);
    if (panel_dev->hw_delay.enable_delay) {
        OsalMSleep(panel_dev->hw_delay.enable_delay);
    }
    return HDF_SUCCESS;
}

static int32_t PanelOff(struct PanelData *panel)
{
    struct panel_jd9365_dev *panel_dev = NULL;

    panel_dev = ToPanelSimpleDev(panel);
    if (panel_dev->hw_delay.disable_delay) {
        OsalMSleep(panel_dev->hw_delay.disable_delay);
    }
    return HDF_SUCCESS;
}

static int32_t PanelPrepare(struct PanelData *panel)
{
    int32_t ret;
    struct panel_jd9365_dev *panel_dev = NULL;

    panel_dev = ToPanelSimpleDev(panel);
    ret = regulator_enable(panel_dev->supply);
    if (ret < 0) {
        HDF_LOGE("failed to enable supply: %d\n", ret);
        return ret;
    }
    gpiod_set_value_cansleep(panel_dev->enable_gpio, 1);

    if (panel_dev->hw_delay.reset_delay > 0) {
        OsalMSleep(panel_dev->hw_delay.reset_delay);
    }

    gpiod_set_value_cansleep(panel_dev->reset_gpio, 1);

    if (panel_dev->hw_delay.reset_delay > 0) {
        OsalMSleep(panel_dev->hw_delay.reset_delay);
    }

    gpiod_set_value_cansleep(panel_dev->reset_gpio, 0);

    if (panel_dev->hw_delay.prepare_delay > 0) {
        OsalMSleep(panel_dev->hw_delay.prepare_delay);
    }

    ret = PanelSendCmds(panel_dev->dsiDev, g_panelOnCode, \
        sizeof(g_panelOnCode) / sizeof(g_panelOnCode[0]));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s PanelSendCmds failed", __func__);
        return HDF_FAILURE;
    }
    if (panel_dev->hw_delay.init_delay > 0) {
        OsalMSleep(panel_dev->hw_delay.init_delay);
    }

    return HDF_SUCCESS;
}

static int32_t PanelUnprepare(struct PanelData *panel)
{
    int32_t ret;
    struct panel_jd9365_dev *panel_dev = NULL;

    panel_dev = ToPanelSimpleDev(panel);

    ret = PanelSendCmds(panel_dev->dsiDev, g_panelOffCode, \
        sizeof(g_panelOffCode) / sizeof(g_panelOffCode[0]));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s PanelSendCmds failed", __func__);
        return HDF_FAILURE;
    }
    gpiod_set_value_cansleep(panel_dev->reset_gpio, 1);
    gpiod_set_value_cansleep(panel_dev->enable_gpio, 1);
    regulator_disable(panel_dev->supply);

    if (panel_dev->hw_delay.unprepare_delay) {
        OsalMSleep(panel_dev->hw_delay.unprepare_delay);
    }

    return HDF_SUCCESS;
}

static int32_t PanelInit(struct PanelData *panel)
{
    return 0;
}

#define BLK_PWM_INDEX             2
#define PWM_MAX_PERIOD            40000
/* backlight setting */
#define MIN_LEVEL                 0
#define MAX_LEVEL                 255
#define DEFAULT_LEVEL             127

static struct PanelInfo g_panelInfo = {
    .width = 800,          /* width */
    .height = 1280,          /* height */
    .hbp = 20,             /* horizontal back porch */
    .hfp = 40,         /* horizontal front porch */
    .hsw = 20,              /* horizontal sync width */
    .vbp = 12,              /* vertical back porch */
    .vfp = 30,              /* vertical front porch */
    .vsw = 4,               /* vertical sync width */
    .clockFreq = 70000000,  /* clock */
    .pWidth = 68,           /* physical width */
    .pHeight = 121,         /* physical height */
    .connectorType = DRM_MODE_CONNECTOR_DPI,   /* DRM_MODE_CONNECTOR_DPI=17 */
    .blk = { BLK_PWM, MIN_LEVEL, MAX_LEVEL, DEFAULT_LEVEL },
};

static void PanelResInit(struct panel_jd9365_dev *panel_dev)
{
    panel_dev->dsiDev->lanes = 4;  /* 4: dsi,lanes ,number of active data lanes */
    panel_dev->dsiDev->format = MIPI_DSI_FMT_RGB888; // dsi,format pixel format for video mode MIPI_DSI_FMT_RGB888
    panel_dev->dsiDev->mode_flags = (MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST | MIPI_DSI_MODE_LPM \
                                          | MIPI_DSI_MODE_EOT_PACKET);
    panel_dev->panel.info = &g_panelInfo;
    panel_dev->panel.init = PanelInit;
    panel_dev->panel.on = PanelOn;
    panel_dev->panel.off = PanelOff;
    panel_dev->panel.prepare = PanelPrepare;
    panel_dev->panel.unprepare = PanelUnprepare;
    panel_dev->panel.priv = panel_dev->dsiDev;
    panel_dev->hw_delay.disable_delay = 50; /* 50:disable_delay time */
    panel_dev->hw_delay.enable_delay = 120; /* 120:enable_delay */
    panel_dev->hw_delay.init_delay = 20; /* 20:init_delay */
    panel_dev->hw_delay.prepare_delay = 2; /* 2:prepare_delay */
    panel_dev->hw_delay.reset_delay = 100;  /* 100:reset_delay */
    panel_dev->hw_delay.unprepare_delay = 20;  /* 20:unprepare_delay */
}

static int32_t PanelEntryInit(struct HdfDeviceObject *object)
{
    struct device_node *panelNode = NULL;
    struct panel_jd9365_dev *panel_dev = NULL;

    panel_dev = (struct panel_jd9365_dev *)OsalMemCalloc(sizeof(struct panel_jd9365_dev));
    if (panel_dev == NULL) {
        HDF_LOGE("%s panel_dev malloc fail", __func__);
        return HDF_FAILURE;
    }
    g_panel_dev = panel_dev;
    panelNode = of_find_compatible_node(NULL, NULL, "simple-panel-dsi");
    if (panelNode == NULL) {
        HDF_LOGE("%s of_find_compatible_node fail", __func__);
        goto FAIL;
    }
    panel_dev->dsiDev = of_find_mipi_dsi_device_by_node(panelNode);
    if (panel_dev->dsiDev == NULL) {
        HDF_LOGE("%s of_find_mipi_dsi_device_by_node fail", __func__);
        goto FAIL;
    }
    panel_dev->supply = devm_regulator_get(&panel_dev->dsiDev->dev, "power");
    if (panel_dev->supply == NULL) {
        HDF_LOGE("Get regulator fail");
        goto FAIL;
    }

    panel_dev->enable_gpio = devm_gpiod_get_optional(&panel_dev->dsiDev->dev, "enable", GPIOD_ASIS);
    if (IS_ERR(panel_dev->enable_gpio)) {
        HDF_LOGE("get enable_gpio fail");
        goto FAIL;
    }
    panel_dev->hpd_gpio = devm_gpiod_get_optional(&panel_dev->dsiDev->dev, "hpd", GPIOD_IN);
    if (IS_ERR(panel_dev->hpd_gpio)) {
        HDF_LOGE("get hpd_gpio fail");
        goto FAIL;
    }
    panel_dev->reset_gpio = devm_gpiod_get_optional(&panel_dev->dsiDev->dev, "reset", GPIOD_ASIS);
    if (IS_ERR(panel_dev->reset_gpio)) {
        HDF_LOGE("get reset_gpio fail");
        goto FAIL;
    }

    PanelResInit(panel_dev);
    panel_dev->panel.object = object;
    object->priv = panel_dev;

    if (RegisterPanel(&panel_dev->panel) != HDF_SUCCESS) {
        HDF_LOGE("RegisterPanel fail");
        goto FAIL;
    }
    HDF_LOGI("%s success", __func__);
    return HDF_SUCCESS;
FAIL:
    OsalMemFree(panel_dev);
    return HDF_FAILURE;
}

struct HdfDriverEntry PanelDevEntry = {
    .moduleVersion = 1,
    .moduleName = "LCD_JD9365_BOE",
    .Init = PanelEntryInit,
};

HDF_INIT(PanelDevEntry);
