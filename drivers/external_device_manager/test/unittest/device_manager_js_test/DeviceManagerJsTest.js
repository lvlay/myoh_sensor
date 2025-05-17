/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

import deviceManager from '@ohos.driver.deviceManager'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe("DeviceManagerJsTest", function () {
    function callback(data) {
        console.info("callback" + JSON.stringify(data));
        expect(typeof(data.x)).assertEqual("number");
    }

    function callback2() {
        console.info("callback2" + JSON.stringify(data));
        expect(typeof(data.x)).assertEqual("number");
    }

    beforeAll(function() {
        console.info('beforeAll called')
    })

    afterAll(function() {
        console.info('afterAll called')
    })

    beforeEach(function() {
        console.info('beforeEach called')
    })

    afterEach(function() {
        console.info('afterEach called')
    })

    const PARAMETER_ERROR_CODE = 401
    const SERVICE_EXCEPTION_CODE = 22900001
    const SERVICE_EXCEPTION_CODE_NEW = 26300001

    /*
     * @tc.name:DeviceManager_queryDevices_001
     * @tc.desc:verify queryDevice result
     * @tc.type: FUNC
     */
    it("DeviceManager_queryDevices_001", 0, function () {
        console.info('----------------------DeviceManager_queryDevices_001---------------------------');
        try {
            var devices = deviceManager.queryDevices(deviceManager.BusType.USB);
            expect(devices != null).assertEqual(true);
            if (devices.length > 0) {
                expect(devices[0] != null).assertEqual(true);
                expect(devices[0].vendorId != null).assertEqual(true);
                expect(devices[0].productId != null).assertEqual(true);
            }
        } catch (err) {
            expect(err.code).assertEqual(SERVICE_EXCEPTION_CODE);
        }
    })

    /*
     * @tc.name:DeviceManager_queryDevices_002
     * @tc.desc:verify queryDevice no param result
     * @tc.type: FUNC
     */
    it("DeviceManager_queryDevices_002", 0, function () {
        console.info('----------------------DeviceManager_queryDevices_002---------------------------');
        try {
            var devices = deviceManager.queryDevices();
            expect(devices != null).assertEqual(true);
        } catch (err) {
            expect(err.code).assertEqual(SERVICE_EXCEPTION_CODE);
        }
    })

    /*
     * @tc.name:DeviceManager_bindDevices_003
     * @tc.desc:verify bindDevice invalid param
     * @tc.type: FUNC
     */
    it("DeviceManager_bindDevices_003", 0, async function (done) {
        console.info('----------------------DeviceManager_bindDevices_003---------------------------');
        try {
            deviceManager.bindDevice('fakeid', (error, data) => {
                expect(false).assertTrue();
                done();
            }, (error, data) => {
                expect(false).assertTrue();
                done();
            });
            expect(false).assertTrue();
            done();
        } catch (error) {
            expect(error.code).assertEqual(PARAMETER_ERROR_CODE);
            done();
        }
    })

    /*
     * @tc.name:DeviceManager_bindDevices_004
     * @tc.desc:verify bindDevice any device
     * @tc.type: FUNC
     */
    it("DeviceManager_bindDevices_004", 0, async function (done) {
        console.info('----------------------DeviceManager_bindDevices_004---------------------------');
        try {
            deviceManager.bindDevice(12345, (error, data) => {
                expect(false).assertTrue();
                done();
            }, (error, data) => {
                expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE);
                done();
            });
            expect(false).assertTrue();
            done();
        } catch (error) {
            expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE);
            done();
        }
    })

    /*
     * @tc.name:DeviceManager_bindDeviceDriver_005
     * @tc.desc:verify bindDeviceDriver any device
     * @tc.type: FUNC
     */
    it("DeviceManager_bindDeviceDriver_005", 0, async function (done) {
        console.info('----------------------DeviceManager_bindDeviceDriver_005---------------------------');
        try {
            deviceManager.bindDeviceDriver(12345, (error, data) => {
                expect(false).assertTrue();
                done();
            }, (error, data) => {
                expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE);
                done();
            });
            expect(false).assertTrue();
            done();
        } catch (error) {
            expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE);
            done();
        }
    })

    /*
     * @tc.name:DeviceManager_bindDevices_006
     * @tc.desc:verify bindDevice invalid param count
     * @tc.type: FUNC
     */
    it("DeviceManager_bindDevices_006", 0, async function (done) {
        console.info('----------------------DeviceManager_bindDevices_006---------------------------');
        try {
            deviceManager.bindDevice();
            expect(false).assertTrue();
            done();
        } catch (error) {
            expect(error.code).assertEqual(PARAMETER_ERROR_CODE);
            done();
        }
    })

    /*
     * @tc.name:DeviceManager_bindDevices_007
     * @tc.desc:verify bindDevice invalid param
     * @tc.type: FUNC
     */
    it("DeviceManager_bindDevices_007", 0, async function (done) {
        console.info('----------------------DeviceManager_bindDevices_007---------------------------');
        try {
            deviceManager.bindDevice(12345);
            expect(false).assertTrue();
            done();
        } catch (error) {
            expect(error.code).assertEqual(PARAMETER_ERROR_CODE);
            done();
        }
    })

    /*
     * @tc.name:DeviceManager_bindDevices_008
     * @tc.desc:verify bindDevice invalid param
     * @tc.type: FUNC
     */
    it("DeviceManager_bindDevices_008", 0, async function (done) {
        console.info('----------------------DeviceManager_bindDevices_008---------------------------');
        try {
            deviceManager.bindDevice(12345, 23456);
            expect(false).assertTrue();
            done();
        } catch (error) {
            expect(error.code).assertEqual(PARAMETER_ERROR_CODE);
            done();
        }
    })

    /*
     * @tc.name:DeviceManager_bindDevices_009
     * @tc.desc:verify bindDevice promise
     * @tc.type: FUNC
     */
    it("DeviceManager_bindDevices_009", 0, async function (done) {
        console.info('----------------------DeviceManager_bindDevices_009---------------------------');
        try {
            deviceManager.bindDevice('fakeid', (error, data) => {
                expect(false).assertTrue();
                done();
            }).then(data => {
                expect(false).assertTrue();
                done();
            }, error => {
                expect(false).assertTrue();
                done();
            });
            expect(false).assertTrue();
            done();
        } catch (error) {
            expect(error.code).assertEqual(PARAMETER_ERROR_CODE);
            done();
        }
    })

    /*
     * @tc.name:DeviceManager_bindDevices_010
     * @tc.desc:verify bindDevice promise
     * @tc.type: FUNC
     */
    it("DeviceManager_bindDevices_010", 0, async function (done) {
        console.info('----------------------DeviceManager_bindDevices_010---------------------------');
        try {
            deviceManager.bindDevice(12345, (error, data) => {
                expect(false).assertTrue();
                done();
            }).then(data => {
                expect(false).assertTrue();
                done();
            }, error => {
                expect(false).assertTrue();
                done();
            });
            expect(false).assertTrue();
            done();
        } catch (error) {
            expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE);
            done();
        }
    })

    /*
     * @tc.name:DeviceManager_bindDeviceDriver_011
     * @tc.desc:verify bindDeviceDriver promise
     * @tc.type: FUNC
     */
    it("DeviceManager_bindDeviceDriver_011", 0, async function (done) {
        console.info('----------------------DeviceManager_bindDeviceDriver_011---------------------------');
        try {
            deviceManager.bindDeviceDriver(12345, (error, data) => {
                expect(false).assertTrue();
                done();
            }).then(data => {
                expect(data != null).assertTrue();
                let remoteDeviceDriver = data;
                expect(remoteDeviceDriver.deviceId != null).assertTrue();
                expect(remoteDeviceDriver.remote != null).assertTrue();
                done();
            }, error => {
                expect(false).assertTrue();
                done();
            });
            expect(false).assertTrue();
            done();
        } catch (error) {
            expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE);
            done();
        }
    })

    /*
     * @tc.name:DeviceManager_unbindDevices_012
     * @tc.desc:verify unbindDevice any device
     * @tc.type: FUNC
     */
    it("DeviceManager_unbindDevices_012", 0, async function (done) {
        console.info('----------------------DeviceManager_unbindDevices_012---------------------------');
        try {
            deviceManager.unbindDevice('fakeid', (error, data) => {
                expect(false).assertTrue();
                done();
            });
            expect(false).assertTrue();
            done();
        } catch (error) {
            expect(error.code).assertEqual(PARAMETER_ERROR_CODE);
            done();
        }
    })

    /*
     * @tc.name:DeviceManager_unbindDevices_013
     * @tc.desc:verify unbindDevice any device
     * @tc.type: FUNC
     */
    it("DeviceManager_unbindDevices_013", 0, async function (done) {
        console.info('----------------------DeviceManager_unbindDevices_013---------------------------');
        try {
            deviceManager.unbindDevice(12345, (error, data) => {
                expect(false).assertTrue();
                done();
            });
            expect(false).assertTrue();
            done();
        } catch (error) {
            expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE);
            done();
        }
    })

    /*
     * @tc.name:DeviceManager_unbindDevices_014
     * @tc.desc:verify unbindDevice invalid param
     * @tc.type: FUNC
     */
    it("DeviceManager_unbindDevices_014", 0, async function (done) {
        console.info('----------------------DeviceManager_unbindDevices_014---------------------------');
        try {
            deviceManager.unbindDevice();
            expect(false).assertTrue();
            done();
        } catch (error) {
            expect(error.code).assertEqual(PARAMETER_ERROR_CODE);
            done();
        }
    })

    /*
     * @tc.name:DeviceManager_unbindDevices_015
     * @tc.desc:verify unbindDevice promise
     * @tc.type: FUNC
     */
    it("DeviceManager_unbindDevices_015", 0, async function (done) {
        console.info('----------------------DeviceManager_unbindDevices_015---------------------------');
        try {
            deviceManager.unbindDevice(12345).then(data => {
                expect(false).assertTrue();
                done();
            }, error => {
                expect(false).assertTrue();
                done();
            });
            expect(false).assertTrue();
            done();
        } catch (error) {
            expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE);
            done();
        }
    })

    /*
     * @tc.name:DeviceManager_queryDeviceInfo_001
     * @tc.desc:verify queryDeviceInfo invalid param
     * @tc.type: FUNC
     */
    it("DeviceManager_queryDeviceInfo_001", 0, async function () {
        console.info('----------------------DeviceManager_queryDeviceInfo_001---------------------------');
        try {
            deviceManager.queryDeviceInfo('invalidDeviceId');
            expect(false).assertTrue();
        } catch (error) {
            expect(error.code).assertEqual(PARAMETER_ERROR_CODE);
        }
    })

    function isUsbDevice(deviceId) {
        return (deviceId & 0x00000000FFFFFFFF) === deviceManager.BusType.USB;
    }

    function assertInterfaceDesc(interfaceDesc) {
        expect(Object.prototype.toString.call(interfaceDesc)).assertEqual('[object Object]');
        console.log('interfaceDesc.bInterfaceNumber:' + interfaceDesc.bInterfaceNumber);
        expect(typeof(interfaceDesc.bInterfaceNumber)).assertEqual('number');
        console.log('interfaceDesc.bClass:' + interfaceDesc.bClass);
        expect(typeof(interfaceDesc.bClass)).assertEqual('number');
        console.log('interfaceDesc.bSubClass:' + interfaceDesc.bSubClass);
        expect(typeof(interfaceDesc.bSubClass)).assertEqual('number');
        console.log('interfaceDesc.bProtocol:' + interfaceDesc.bProtocol);
        expect(typeof(interfaceDesc.bProtocol)).assertEqual('number');
    }

    function assertUsbDeviceInfoExt(usbDeviceInfo) {
        expect(Object.prototype.toString.call(usbDeviceInfo)).assertEqual('[object Object]');
        console.log('usbDeviceInfo.vendorId:' + usbDeviceInfo.vendorId);
        expect(typeof(usbDeviceInfo.vendorId)).assertEqual('number');
        console.log('usbDeviceInfo.productId:' + usbDeviceInfo.productId);
        expect(typeof(usbDeviceInfo.productId)).assertEqual('number');
        expect(Array.isArray(usbDeviceInfo.interfaceDescList)).assertTrue();
        for (const desc of usbDeviceInfo.interfaceDescList) {
            assertInterfaceDesc(desc);
        }
    }

    function assertDeviceInfo(deviceInfo) {
        expect(Object.prototype.toString.call(deviceInfo)).assertEqual('[object Object]');
        console.log('deviceInfo.deviceId:' + deviceInfo.deviceId);
        expect(typeof(deviceInfo.deviceId)).assertEqual('number');
        console.log('deviceInfo.isDriverMatched:' + deviceInfo.isDriverMatched);
        expect(typeof(deviceInfo.isDriverMatched)).assertEqual('boolean');
        if (deviceInfo.isDriverMatched) {
            console.log('deviceInfo.driverUid:' + deviceInfo.driverUid);
            expect(typeof(deviceInfo.driverUid)).assertEqual('string');
        }
        if (isUsbDevice(deviceInfo.deviceId)) {
            assertUsbDeviceInfoExt(deviceInfo)
        }
    }

    /*
     * @tc.name:DeviceManager_queryDeviceInfo_002
     * @tc.desc:verify queryDeviceInfo none deviceId
     * @tc.type: FUNC
     */
    it("DeviceManager_queryDeviceInfo_002", 0, async function () {
        console.info('----------------------DeviceManager_queryDeviceInfo_002---------------------------');
        try {
            const deviceInfos = deviceManager.queryDeviceInfo();
            expect(Array.isArray(deviceInfos)).assertTrue();
            for (const deviceInfo of deviceInfos) {
                assertDeviceInfo(deviceInfo);
            }
        } catch (error) {
            expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE_NEW);
        }
    })

    /*
     * @tc.name:DeviceManager_queryDeviceInfo_003
     * @tc.desc:verify queryDeviceInfo has deviceId
     * @tc.type: FUNC
     */
    it("DeviceManager_queryDeviceInfo_003", 0, async function () {
        console.info('----------------------DeviceManager_queryDeviceInfo_003---------------------------');
        try {
            const deviceInfos = deviceManager.queryDeviceInfo(12345);
            expect(Array.isArray(deviceInfos)).assertTrue();
            for (const deviceInfo of deviceInfos) {
                assertDeviceInfo(deviceInfo);
            }
        } catch (error) {
            expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE_NEW);
        }
    })

    /*
     * @tc.name:DeviceManager_queryDriverInfo_001
     * @tc.desc:verify queryDriverInfo invalid param
     * @tc.type: FUNC
     */
    it("DeviceManager_queryDriverInfo_001", 0, async function () {
        console.info('----------------------DeviceManager_queryDriverInfo_001---------------------------');
        try {
            deviceManager.queryDriverInfo(12345);
            expect(false).assertTrue();
        } catch (error) {
            expect(error.code).assertEqual(PARAMETER_ERROR_CODE);
        }
    })

    function assertDriverInfo(driverInfo) {
        expect(Object.prototype.toString.call(driverInfo)).assertEqual('[object Object]');
        console.log('driverInfo.busType:' + driverInfo.busType);
        expect(typeof(driverInfo.busType)).assertEqual('number');
        console.log('driverInfo.driverUid:' + driverInfo.driverUid);
        expect(typeof(driverInfo.driverUid)).assertEqual('string');
        console.log('driverInfo.driverName:' + driverInfo.driverName);
        expect(typeof(driverInfo.driverName)).assertEqual('string');
        console.log('driverInfo.driverVersion:' + driverInfo.driverVersion);
        expect(typeof(driverInfo.driverVersion)).assertEqual('string');
        console.log('driverInfo.driverSize:' + driverInfo.driverSize);
        expect(typeof(driverInfo.driverSize)).assertEqual('string');
        console.log('driverInfo.description:' + driverInfo.description);
        expect(typeof(driverInfo.description)).assertEqual('string');
        if (driverInfo.busType === deviceManager.BusType.USB) {
            console.log('driverInfo.productIdList:' + JSON.stringify(driverInfo.productIdList));
            expect(Array.isArray(driverInfo.productIdList)).assertTrue();
            console.log('driverInfo.vendorIdList:' + JSON.stringify(driverInfo.vendorIdList));
            expect(Array.isArray(driverInfo.vendorIdList)).assertTrue();
            for (const productId of driverInfo.productIdList) {
                expect(typeof(productId)).assertEqual('number');
            }
            for (const vendorId of driverInfo.vendorIdList) {
                expect(typeof(vendorId)).assertEqual('number');
            }
        }
    }

    /*
     * @tc.name:DeviceManager_queryDriverInfo_002
     * @tc.desc:verify queryDriverInfo none driverUid
     * @tc.type: FUNC
     */
    it("DeviceManager_queryDriverInfo_002", 0, async function () {
        console.info('----------------------DeviceManager_queryDriverInfo_002---------------------------');
        try {
            const driverInfos = deviceManager.queryDriverInfo();
            expect(Array.isArray(driverInfos)).assertTrue();
            for (const driverInfo of driverInfos) {
                assertDriverInfo(driverInfo);
            }
        } catch (error) {
            expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE_NEW);
        }
    })

    /*
     * @tc.name:DeviceManager_queryDriverInfo_003
     * @tc.desc:verify queryDriverInfo has driverUid
     * @tc.type: FUNC
     */
    it("DeviceManager_queryDriverInfo_003", 0, async function () {
        console.info('----------------------DeviceManager_queryDriverInfo_003---------------------------');
        try {
            const driverInfos = deviceManager.queryDriverInfo('driver-12345');
            expect(Array.isArray(driverInfos)).assertTrue();
            for (const driverInfo of driverInfos) {
                assertDriverInfo(driverInfo);
            }
        } catch (error) {
            expect(error.code).assertEqual(SERVICE_EXCEPTION_CODE_NEW);
        }
    })
})