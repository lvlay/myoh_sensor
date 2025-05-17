/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hcs_fuzzer.h"

#include "hcs_tree_if.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "securec.h"

namespace OHOS {
constexpr size_t THRESHOLD = 10;
#define HDF_LOG_TAG hcs_fuzzer_fuzzer

enum FuncId {
    HCS_HCSGETBOOL,
    HCS_HCSGETUINT8,
    HCS_HCSGETUINT32,
    HCS_HCSGETUINT64,
    HCS_HCSGETUINT8ARRAYELEM,
    HCS_HCSGETUINT16ARRAYELEM,
    HCS_HCSGETUINT32ARRAYELEM,
    HCS_HCSGETUINT64ARRAYELEM,
    HCS_END
};

void FuncHcsGetBool(const uint8_t *data, size_t size)
{
    if (size < sizeof(struct DeviceResourceNode)) {
        return;
    }

    struct DeviceResourceNode node;
    size_t sz = sizeof(struct DeviceResourceNode);
    if (memcpy_s(&node, sz, data, sz) != EOK) {
        return;
    }
    node.name = "nothing";
    node.attrData = NULL;
    node.parent = NULL;
    node.child = NULL;
    node.sibling = NULL;
    const char *attrName = "nothing";
    HcsGetBool(&node, attrName);
    return;
}

void FuncHcsGetUint8(const uint8_t *data, size_t size)
{
    if (size < sizeof(struct DeviceResourceNode)) {
        return;
    }

    struct DeviceResourceNode node;
    size_t sz = sizeof(struct DeviceResourceNode);
    if (memcpy_s(&node, sz, data, sz) != EOK) {
        return;
    }
    node.name = "nothing";
    node.attrData = NULL;
    node.parent = NULL;
    node.child = NULL;
    node.sibling = NULL;
    const char *attrName = "nothing";
    uint8_t value = 0;
    uint8_t def = 0;

    HcsGetUint8(&node, attrName, &value, def);
    return;
}

void FuncHcsGetUint32(const uint8_t *data, size_t size)
{
    if (size < sizeof(struct DeviceResourceNode)) {
        return;
    }

    struct DeviceResourceNode node;
    size_t sz = sizeof(struct DeviceResourceNode);
    if (memcpy_s(&node, sz, data, sz) != EOK) {
        return;
    }
    node.name = "nothing";
    node.attrData = NULL;
    node.parent = NULL;
    node.child = NULL;
    node.sibling = NULL;
    const char *attrName = "nothing";
    uint32_t value = 0;
    uint32_t def = 0;

    HcsGetUint32(&node, attrName, &value, def);
    return;
}

void FuncHcsGetUint64(const uint8_t *data, size_t size)
{
    if (size < sizeof(struct DeviceResourceNode)) {
        return;
    }

    struct DeviceResourceNode node;
    size_t sz = sizeof(struct DeviceResourceNode);
    if (memcpy_s(&node, sz, data, sz) != EOK) {
        return;
    }
    node.name = "nothing";
    node.attrData = NULL;
    node.parent = NULL;
    node.child = NULL;
    node.sibling = NULL;
    const char *attrName = "nothing";
    uint64_t value = 0;
    uint64_t def = 0;

    HcsGetUint64(&node, attrName, &value, def);
    return;
}

void FuncHcsGetUint8ArrayElem(const uint8_t *data, size_t size)
{
    if (size < sizeof(struct DeviceResourceNode)) {
        return;
    }

    struct DeviceResourceNode node;
    size_t sz = sizeof(struct DeviceResourceNode);
    if (memcpy_s(&node, sz, data, sz) != EOK) {
        return;
    }
    node.name = "nothing";
    node.attrData = NULL;
    node.parent = NULL;
    node.child = NULL;
    node.sibling = NULL;
    const char *attrName = "nothing";
    uint32_t index = 0;
    uint8_t value = 0;
    uint8_t def = 0;

    HcsGetUint8ArrayElem(&node, attrName, index, &value, def);
    return;
}

void FuncHcsGetUint16ArrayElem(const uint8_t *data, size_t size)
{
    if (size < sizeof(struct DeviceResourceNode)) {
        return;
    }

    struct DeviceResourceNode node;
    size_t sz = sizeof(struct DeviceResourceNode);
    if (memcpy_s(&node, sz, data, sz) != EOK) {
        return;
    }
    node.name = "nothing";
    node.attrData = NULL;
    node.parent = NULL;
    node.child = NULL;
    node.sibling = NULL;
    const char *attrName = "nothing";
    uint32_t index = 0;
    uint16_t value = 0;
    uint16_t def = 0;

    HcsGetUint16ArrayElem(&node, attrName, index, &value, def);
    return;
}

void FuncHcsGetUint32ArrayElem(const uint8_t *data, size_t size)
{
    if (size < sizeof(struct DeviceResourceNode)) {
        return;
    }

    struct DeviceResourceNode node;
    size_t sz = sizeof(struct DeviceResourceNode);
    if (memcpy_s(&node, sz, data, sz) != EOK) {
        return;
    }
    node.name = "nothing";
    node.attrData = NULL;
    node.parent = NULL;
    node.child = NULL;
    node.sibling = NULL;
    const char *attrName = "nothing";
    uint32_t index = 0;
    uint32_t value = 0;
    uint32_t def = 0;

    HcsGetUint32ArrayElem(&node, attrName, index, &value, def);
    return;
}

void FuncHcsGetUint64ArrayElem(const uint8_t *data, size_t size)
{
    if (size < sizeof(struct DeviceResourceNode)) {
        return;
    }

    struct DeviceResourceNode node;
    size_t sz = sizeof(struct DeviceResourceNode);
    if (memcpy_s(&node, sz, data, sz) != EOK) {
        return;
    }
    node.name = "nothing";
    node.attrData = NULL;
    node.parent = NULL;
    node.child = NULL;
    node.sibling = NULL;
    const char *attrName = "nothing";
    uint32_t index = 0;
    uint64_t value = 0;
    uint64_t def = 0;

    HcsGetUint64ArrayElem(&node, attrName, index, &value, def);
    return;
}

void FuncSwitch(uint32_t cmd, const uint8_t *data, size_t size)
{
    switch (cmd) {
        case HCS_HCSGETBOOL: {
            FuncHcsGetBool(data, size);
            break;
        }
        case HCS_HCSGETUINT8: {
            FuncHcsGetUint8(data, size);
            break;
        }
        case HCS_HCSGETUINT32: {
            FuncHcsGetUint32(data, size);
            break;
        }
        case HCS_HCSGETUINT64: {
            FuncHcsGetUint64(data, size);
            break;
        }
        case HCS_HCSGETUINT8ARRAYELEM: {
            FuncHcsGetUint8ArrayElem(data, size);
            break;
        }
        case HCS_HCSGETUINT16ARRAYELEM: {
            FuncHcsGetUint16ArrayElem(data, size);
            break;
        }
        case HCS_HCSGETUINT32ARRAYELEM: {
            FuncHcsGetUint32ArrayElem(data, size);
            break;
        }
        case HCS_HCSGETUINT64ARRAYELEM: {
            FuncHcsGetUint64ArrayElem(data, size);
            break;
        }
        default:
            return;
    }
}

void TraverseAllFunc(const uint8_t *data, size_t size)
{
    for (uint32_t cmd = 0; cmd < HCS_END; cmd++) {
        FuncSwitch(cmd, data, size);
    }
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::THRESHOLD) {
        return HDF_SUCCESS;
    }

    OHOS::TraverseAllFunc(data, size);
    return HDF_SUCCESS;
}
