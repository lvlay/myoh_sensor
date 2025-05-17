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
#include <codecvt>
#include <string>
#include "hdi_support.h"
#include "object_collector.h"
#include "hdf_base.h"

using namespace testing::ext;

namespace OHOS {
    struct TestService {
    };

    const std::u16string  INTERFACE_NAME = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,
        char16_t> {}.from_bytes("sample_driver");


class ObjectCollectorTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(ObjectCollectorTest, ObjectCollectorTest001, TestSize.Level1)
{
    sptr<IRemoteObject> object = HDI::ObjectCollector::GetInstance().NewObject(nullptr, INTERFACE_NAME);
    ASSERT_EQ(object, nullptr);
}
} // namespace OHOS