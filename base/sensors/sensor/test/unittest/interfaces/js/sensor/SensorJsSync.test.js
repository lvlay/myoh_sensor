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
import CommonConstants from './CommonConstants';
import sensor from '@ohos.sensor'

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe("SensorSyncTest", function () {
    beforeAll(async function() {
        /*
         * @tc.setup: setup invoked before all testcases
         */
         console.info('beforeAll called')
    })

    afterAll(function() {
        /*
         * @tc.teardown: teardown invoked after all testcases
         */
         console.info('afterAll called')
    })

    beforeEach(function() {
        /*
         * @tc.setup: setup invoked before each testcases
         */
         console.info('beforeEach called')
    })

    afterEach(function() {
        /*
         * @tc.teardown: teardown invoked after each testcases
         */
        console.info('afterEach called')
    })

    /*
     * @tc.name: SensorSyncTest_001
     * @tc.desc: verify sensor sync interface
     * @tc.size: MediumTest
     * @tc.type: Function
     * @tc.level: Level 1
     * @tc.require: Issue Number
     * @tc.number: SensorSyncTest_001
     */
    it("SensorSyncTest_001", 0, async function (done) {
        console.info('----------------------SensorSyncTest_001---------------------------');
        try {
            let ret = sensor.getSingleSensorSync(sensor.SensorId.ACCELEROMETER);
            console.info('getSingleSensorSync: ' + JSON.stringify(ret));
            done();
        } catch (err) {
            console.error('getSingleSensorSync err: ' + JSON.stringify(err));
            done();
        }
    })

    /*
     * @tc.name: SensorSyncTest_002
     * @tc.desc: verify sensor sync interface
     * @tc.size: MediumTest
     * @tc.type: Function
     * @tc.level: Level 1
     * @tc.require: Issue Number
     * @tc.number: SensorSyncTest_002
     */
    it("SensorSyncTest_002", 0, async function (done) {
        console.info('----------------------SensorSyncTest_002---------------------------');
        try {
            let ret = sensor.getSingleSensorSync(-1);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error('getSingleSensorSync err: ' + JSON.stringify(err));
            expect(true).assertTrue();
            done();
        }
    })

    /*
     * @tc.name: SensorSyncTest_003
     * @tc.desc: verify sensor sync interface
     * @tc.size: MediumTest
     * @tc.type: Function
     * @tc.level: Level 1
     * @tc.require: Issue Number
     * @tc.number: SensorSyncTest_003
     */
    it("SensorSyncTest_003", 0, async function (done) {
        console.info('----------------------SensorSyncTest_003---------------------------');
        try {
            let ret = sensor.getSensorListSync();
            console.info('getSensorListSync: ' + JSON.stringify(ret));
            done();
        } catch (err) {
            console.error('getSensorListSync err: ' + JSON.stringify(err));
            expect(err.code).assertEqual(CommonConstants.SERVICE_EXCEPTION_CODE);
            done();
        }
    })
})