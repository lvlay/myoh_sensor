/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

import usb from '@ohos.usbManager'
import deviceManager from '@ohos.driver.deviceManager'
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index'

describe("SystemApiJsTest", function () {
    const TAG = "[SystemApiJsTest]";
    const SYSTEMAPI_DENIED_CODE = 202;
    const TEST_DEVICE_ID = 0;
    const TEST_DRIVER_UID = 'testDriverUid'
    const TEST_FUNCTION = () => {
        console.info("Test function is called");
    };

    let deviceNum = 0;
    const isDeviceConnected = done => {
        if (deviceNum > 0) {
            console.info("Test USB device is connected");
            return true;
        }
        console.info("Test USB device is not connected");
        expect(true).assertTrue();
        if (typeof(done) === 'function') {
            done();
        }
        return false;
    }

    beforeAll(function () {
        console.info('beforeAll called');
        try {
            const devicesList = usb.getDevices();
            if (Array.isArray(devicesList)) {
                deviceNum = devicesList.length;
            }
        } catch (err) {
            console.error(TAG, `getDevices failed, message is ${err.message}`);
        }
    })

    afterAll(function () {
        console.info('AfterAll called');
    })

    /*
     * @tc.name:SystemApi_queryDeviceInfo_001
     * @tc.desc:verify SystemApi of queryDeviceInfo
     * @tc.type: FUNC
     */
    it("SystemApi_queryDeviceInfo_001", 0, done => {
        console.info('----------------------SystemApi_queryDeviceInfo_001---------------------------');
        if (!isDeviceConnected(done)) {
            return;
        }
        try {
            deviceManager.queryDeviceInfo(TEST_DEVICE_ID);
            expect(false).assertTrue();
            done();
        } catch (err) {
            expect(err.code).assertEqual(SYSTEMAPI_DENIED_CODE);
            done();
        }
    });

    /*
     * @tc.name:SystemApi_queryDriverInfo_001
     * @tc.desc:verify SystemApi of queryDriverInfo
     * @tc.type: FUNC
     */
    it("SystemApi_queryDriverInfo_001", 0, done => {
        console.info('----------------------SystemApi_queryDriverInfo_001---------------------------');
        if (!isDeviceConnected(done)) {
            return;
        }
        try {
            deviceManager.queryDriverInfo(TEST_DRIVER_UID);
            expect(false).assertTrue();
            done();
        } catch (err) {
            expect(err.code).assertEqual(SYSTEMAPI_DENIED_CODE);
            done();
        }
    });
});