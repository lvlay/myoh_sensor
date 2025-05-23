/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PLATFORM_DEVICE_TEST_H
#define PLATFORM_DEVICE_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

enum PlatformDeviceTestCmd {
    PLAT_DEVICE_TEST_SET_NAME = 0,
    PLAT_DEVICE_TEST_GET_DEVICE = 1,
    PLAT_DEVICE_TEST_ADD_DEVICE = 2,
    PLAT_DEVICE_TEST_CREATE_SERVICE = 3,
    PLAT_DEVICE_TEST_BIND_DEVICE = 4,
    PLAT_DEVICE_TEST_RELIABILITY = 5,
    PLAT_DEVICE_TEST_CMD_MAX,
};

int PlatformDeviceTestExecute(int cmd);
void PlatformDeviceTestExecuteAll(void);
void PlatformDeviceTestSetUpAll(void);
void PlatformDeviceTestTearDownAll(void);

#ifdef __cplusplus
}
#endif
#endif /* PLATFORM_DEVICE_TEST_H */
