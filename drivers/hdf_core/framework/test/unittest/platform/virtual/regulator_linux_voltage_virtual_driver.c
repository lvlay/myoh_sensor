/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regmap.h>
#include <linux/sysfs.h>
#include "hdf_log.h"
#include "hdf_base.h"

#define HDF_LOG_TAG regulator_virtual

#define MINIMUN_VOLTAGE            1000
#define MAXIMUN_VOLTAGE            50000

struct VirtualVoltageRegulatorDev {
    struct regmap *regmap;
    struct regulator_dev *dev;
};

// note:linux kernel constraints:len(devName) + len(supplyName) < REG_STR_SIZE(64)
static struct regulator_consumer_supply g_virtualVoltageRegulatorVoltSupplies[] = {
    REGULATOR_SUPPLY("vir-voltage-reg-hdf-adp", "regulator_adapter_consumer01"),
};

// virtual regulator init info
static struct regulator_init_data g_virtualVoltageRegulatorInitData = {
    .constraints = {
        .name = "virtual_regulator",
        .min_uV = MINIMUN_VOLTAGE,
        .max_uV = MAXIMUN_VOLTAGE,
        .always_on = 1,
        .valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
    },
    .num_consumer_supplies = ARRAY_SIZE(g_virtualVoltageRegulatorVoltSupplies),
    .consumer_supplies = g_virtualVoltageRegulatorVoltSupplies,
};

static void VirtualVoltageRegulatorDevRelease(struct device *dev)
{
    (void)dev;
}
static struct platform_device g_virtualVoltageRegulatorPlatformDevice = {
    .name = "virtual_regulator_dev",
    .id = -1,
    .dev = {
        .release = VirtualVoltageRegulatorDevRelease,
    }
};

enum RegulatorStatus {
    VIR_REGULATOR_STATUS_OFF,
    VIR_REGULATOR_STATUS_ON,
};

static int g_virStatus = VIR_REGULATOR_STATUS_OFF;
static int VirtualVoltageRegulatorEnable(struct regulator_dev *rdev)
{
    if (rdev == NULL) {
        HDF_LOGE("VirtualVoltageRegulatorEnable: rdev is null!");
        return HDF_FAILURE;
    }

    g_virStatus = VIR_REGULATOR_STATUS_ON;
    return HDF_SUCCESS;
}

static int VirtualVoltageRegulatorDisable(struct regulator_dev *rdev)
{
    if (rdev == NULL) {
        HDF_LOGE("VirtualVoltageRegulatorDisable: rdev is null!");
        return HDF_FAILURE;
    }

    g_virStatus = VIR_REGULATOR_STATUS_OFF;
    return HDF_SUCCESS;
}

static int VirtualVoltageRegulatorIsEnabled(struct regulator_dev *rdev)
{
    if (rdev == NULL) {
        HDF_LOGE("VirtualVoltageRegulatorIsEnabled: rdev is null!");
        return HDF_FAILURE;
    }

    return g_virStatus;
}

static int VirtualVoltageRegulatorSetVoltage(struct regulator_dev *rdev, int minUv,
    int maxUv, unsigned *selector)
{
    (void)selector;
    if ((rdev == NULL) || (rdev->constraints == NULL)) {
        HDF_LOGE("VirtualVoltageRegulatorSetVoltage: rdev or constraints is null!");
        return HDF_FAILURE;
    }

    struct regulation_constraints *reguConstraints = rdev->constraints;
    if (reguConstraints->min_uV == minUv &&
        reguConstraints->max_uV == maxUv) {
        return HDF_SUCCESS;
    }

    return HDF_SUCCESS;
}

#define VIRTUAL_VOLTAGE_VAL_500 500
static int VirtualVoltageRegulatorGetVoltage(struct regulator_dev *rdev)
{
    if (rdev == NULL) {
        HDF_LOGE("VirtualVoltageRegulatorGetVoltage: rdev is null!");
        return HDF_FAILURE;
    }

    return VIRTUAL_VOLTAGE_VAL_500;
}

static struct regulator_ops g_virtualVoltageRegulatorOps = {
    .enable = VirtualVoltageRegulatorEnable,
    .disable = VirtualVoltageRegulatorDisable,
    .is_enabled = VirtualVoltageRegulatorIsEnabled,
    .set_voltage = VirtualVoltageRegulatorSetVoltage,
    .get_voltage = VirtualVoltageRegulatorGetVoltage,
};

static struct regulator_desc g_virtualVoltageRegulatorDesc = {
    .name = "regulator_virtual",
    .type = REGULATOR_VOLTAGE,
    .ops = &g_virtualVoltageRegulatorOps,
    .min_uV = MINIMUN_VOLTAGE,
    .owner = THIS_MODULE,
};

static int VirtualVoltageRegulatorPlatformProbe(struct platform_device *platformDev)
{
    if (platformDev == NULL) {
        HDF_LOGE("VirtualVoltageRegulatorPlatformProbe: platformDev is null!");
        return HDF_FAILURE;
    }
    struct VirtualVoltageRegulatorDev *data;
    struct regulator_config config = {0};

    data = devm_kzalloc(&platformDev->dev, sizeof(*data), GFP_KERNEL);
    if (!data) {
        return -ENOMEM;
    }

    config.dev = &platformDev->dev;
    config.init_data = &g_virtualVoltageRegulatorInitData;
    config.driver_data = data;

    data->dev = regulator_register(&g_virtualVoltageRegulatorDesc, &config);
    if (IS_ERR(data->dev)) {
        HDF_LOGE("VirtualVoltageRegulatorPlatformProbe: fail to register regulator %s\n",
            g_virtualVoltageRegulatorDesc.name);
        return PTR_ERR(data->dev);
    }

    platform_set_drvdata(platformDev, data);
    HDF_LOGI("VirtualVoltageRegulatorPlatformProbe: success!");
    return 0;
}

static int VirtualVoltageRegulatorPlatformRemove(struct platform_device *platformDev)
{
    struct VirtualVoltageRegulatorDev *rdev = platform_get_drvdata(platformDev);

    regulator_unregister(rdev->dev);

    platform_set_drvdata(platformDev, NULL);
    HDF_LOGI("VirtualVoltageRegulatorPlatformRemove: success!");
    return 0;
}

static struct platform_driver g_virtualVoltageRegulatorPlatformDriver = {
    .driver = {
        .name = "virtual_regulator_dev",
        .owner = THIS_MODULE,
    },
    .probe = VirtualVoltageRegulatorPlatformProbe,
    .remove = VirtualVoltageRegulatorPlatformRemove,
};

int VirtualVoltageRegulatorAdapterInit(void)
{
    HDF_LOGI("VirtualVoltageRegulatorAdapterInit: enter!");
    int ret = platform_device_register(&g_virtualVoltageRegulatorPlatformDevice);
    if (ret == 0) {
        ret = platform_driver_register(&g_virtualVoltageRegulatorPlatformDriver);
    } else {
        HDF_LOGE("VirtualVoltageRegulatorAdapterInit:device register fail, ret: %d!", ret);
    }
    return ret;
}

static int __init VirtualVoltageRegulatorInit(void)
{
    HDF_LOGI("VirtualVoltageRegulatorInit: enter!");
    int ret = platform_device_register(&g_virtualVoltageRegulatorPlatformDevice);
    if (ret == 0) {
        ret = platform_driver_register(&g_virtualVoltageRegulatorPlatformDriver);
    }
    return ret;
}

static void __exit VirtualVoltageRegulatorExit(void)
{
    HDF_LOGI("VirtualVoltageRegulatorExit: enter!");
    platform_device_unregister(&g_virtualVoltageRegulatorPlatformDevice);
    platform_driver_unregister(&g_virtualVoltageRegulatorPlatformDriver);
}

MODULE_DESCRIPTION("Virtual voltage Regulator Controller Platform Device Drivers");
MODULE_LICENSE("GPL");
