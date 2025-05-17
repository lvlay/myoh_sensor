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

#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "ddk_api.h"
#include "ddk_types.h"
#include "gtest/gtest.h"

using namespace testing::ext;

#define PARAM_0 0
#define BUFF_LENTH 10
#define PORT_READ 0x01
#define PORT_WRITE 0x02
#define PORT_ILLEGAL 0x08

namespace OHOS {
namespace ExternalDeviceManager {

class DdkBaseTest : public testing::Test {
public:
    void SetUp() override
    {
        EDM_LOGD(MODULE_BASE_DDK, "DdkBaseTest SetUp");
    }
    void TearDown() override
    {
        EDM_LOGD(MODULE_BASE_DDK, "DdkBaseTest TearDown");
    }
};

HWTEST_F(DdkBaseTest, OH_DDK_CreateAshmem_001, TestSize.Level1)
{
    DDK_Ashmem *ashmem = nullptr;
    const uint8_t name[100] = "TestAshmem";
    int32_t bufferLen = BUFF_LENTH;
    auto ret = OH_DDK_CreateAshmem(name, bufferLen, &ashmem);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DdkBaseTest, OH_DDK_CreateAshmem_002, TestSize.Level1)
{
    DDK_Ashmem *ashmem = nullptr;
    const uint8_t name[100] = "TestAshmem";
    uint32_t bufferLen = PARAM_0;
    auto ret = OH_DDK_CreateAshmem(name, bufferLen, &ashmem);
    EXPECT_EQ(ret, DDK_INVALID_PARAMETER);
}

HWTEST_F(DdkBaseTest, OH_DDK_CreateAshmem_003, TestSize.Level1)
{
    DDK_Ashmem *ashmem = nullptr;
    const uint8_t *name = nullptr;
    uint32_t bufferLen = BUFF_LENTH;
    auto ret = OH_DDK_CreateAshmem(name, bufferLen, &ashmem);
    EXPECT_EQ(ret, DDK_INVALID_PARAMETER);
}

HWTEST_F(DdkBaseTest, OH_DDK_MapAshmem_001, TestSize.Level1)
{
    DDK_Ashmem *ashmem = nullptr;
    const uint8_t name[100] = "TestAshmem";
    uint32_t bufferLen = BUFF_LENTH;
    auto ret = OH_DDK_CreateAshmem(name, bufferLen, &ashmem);
    EXPECT_EQ(ret, DDK_SUCCESS);
    const uint8_t ashmemMapType = PORT_READ | PORT_WRITE;
    ret = OH_DDK_MapAshmem(ashmem, ashmemMapType);
    EXPECT_EQ(ret, DDK_SUCCESS);
}

HWTEST_F(DdkBaseTest, OH_DDK_MapAshmem_002, TestSize.Level1)
{
    const uint8_t ashmemMapType = PORT_READ | PORT_WRITE;
    auto ret = OH_DDK_MapAshmem(nullptr, ashmemMapType);
    EXPECT_EQ(ret, DDK_NULL_PTR);
}

HWTEST_F(DdkBaseTest, OH_DDK_MapAshmem_003, TestSize.Level1)
{
    DDK_Ashmem *ashmem = nullptr;
    const uint8_t name[100] = "TestAshmem";
    uint32_t bufferLen = BUFF_LENTH;
    auto ret = OH_DDK_CreateAshmem(name, bufferLen, &ashmem);
    EXPECT_EQ(ret, DDK_SUCCESS);
    const uint8_t ashmemMapType = PORT_READ | PORT_WRITE;
    ashmem->ashmemFd = 0;
    ret = OH_DDK_MapAshmem(ashmem, ashmemMapType);
    EXPECT_EQ(ret, DDK_FAILURE);
}

HWTEST_F(DdkBaseTest, OH_DDK_MapAshmem_004, TestSize.Level1)
{
    DDK_Ashmem *ashmem = nullptr;
    const uint8_t name[100] = "TestAshmem";
    uint32_t bufferLen = BUFF_LENTH;
    auto ret = OH_DDK_CreateAshmem(name, bufferLen, &ashmem);
    EXPECT_EQ(ret, DDK_SUCCESS);
    const uint8_t ashmemMapType = PORT_ILLEGAL;
    ret = OH_DDK_MapAshmem(ashmem, ashmemMapType);
    EXPECT_EQ(ret, DDK_INVALID_OPERATION);
}

HWTEST_F(DdkBaseTest, OH_DDK_UnMapAshmem_001, TestSize.Level1)
{
    DDK_Ashmem *ashmem = nullptr;
    const uint8_t name[100] = "TestAshmem";
    uint32_t bufferLen = BUFF_LENTH;
    auto ret = OH_DDK_CreateAshmem(name, bufferLen, &ashmem);
    EXPECT_EQ(ret, DDK_SUCCESS);
    const uint8_t ashmemMapType = PORT_READ | PORT_WRITE;
    ret = OH_DDK_MapAshmem(ashmem, ashmemMapType);
    EXPECT_EQ(ret, DDK_SUCCESS);
    ret = OH_DDK_UnmapAshmem(ashmem);
    EXPECT_EQ(ret, DDK_SUCCESS);
}

HWTEST_F(DdkBaseTest, OH_DDK_UnMapAshmem_002, TestSize.Level1)
{
    DDK_Ashmem *ashmem = nullptr;
    const uint8_t name[100] = "TestAshmem";
    uint32_t bufferLen = BUFF_LENTH;
    auto ret = OH_DDK_CreateAshmem(name, bufferLen, &ashmem);
    EXPECT_EQ(ret, DDK_SUCCESS);
    const uint8_t ashmemMapType = PORT_READ | PORT_WRITE;
    ret = OH_DDK_MapAshmem(ashmem, ashmemMapType);
    EXPECT_EQ(ret, DDK_SUCCESS);
    ret = OH_DDK_UnmapAshmem(nullptr);
    EXPECT_EQ(ret, DDK_NULL_PTR);
}


HWTEST_F(DdkBaseTest, OH_DDK_DestroyAshmem_001, TestSize.Level1)
{
    DDK_Ashmem *ashmem = nullptr;
    const uint8_t name[100] = "TestAshmem";
    uint32_t bufferLen = BUFF_LENTH;
    auto ret = OH_DDK_CreateAshmem(name, bufferLen, &ashmem);
    EXPECT_EQ(ret, DDK_SUCCESS);
    const uint8_t ashmemMapType = PORT_READ | PORT_WRITE;
    ret = OH_DDK_MapAshmem(ashmem, ashmemMapType);
    EXPECT_EQ(ret, DDK_SUCCESS);
    ret = OH_DDK_DestroyAshmem(ashmem);
    EXPECT_EQ(ret, DDK_SUCCESS);
}

HWTEST_F(DdkBaseTest, OH_DDK_DestroyAshmem_002, TestSize.Level1)
{
    DDK_Ashmem *ashmem = nullptr;
    const uint8_t name[100] = "TestAshmem";
    uint32_t bufferLen = BUFF_LENTH;
    auto ret = OH_DDK_CreateAshmem(name, bufferLen, &ashmem);
    EXPECT_EQ(ret, DDK_SUCCESS);
    const uint8_t ashmemMapType = PORT_READ | PORT_WRITE;
    ret = OH_DDK_MapAshmem(ashmem, ashmemMapType);
    EXPECT_EQ(ret, DDK_SUCCESS);
    ret = OH_DDK_DestroyAshmem(nullptr);
    EXPECT_EQ(ret, DDK_NULL_PTR);
}
} // ExternalDeviceManager
} // OHOS
