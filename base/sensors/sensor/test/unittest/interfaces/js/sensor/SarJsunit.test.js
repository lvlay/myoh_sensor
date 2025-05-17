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
import CommonConstants from './CommonConstants';
import sensor from '@ohos.sensor'

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe("SarJsTest", function () {
    function callback(data) {
        console.info("callback" + JSON.stringify(data));
        expect(typeof(data.absorptionRatio)).assertEqual("number");
    }

    function callback2(data) {
        console.info("callback2" + JSON.stringify(data));
        expect(typeof(data.absorptionRatio)).assertEqual("number");
    }

    beforeAll(function() {
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
     * @tc.number: SarJsTest_001
     * @tc.name: SarJsTest
     * @tc.desc: verify app info is not null
     * @tc.type: FUNC
     * @tc.require: Issue Number
     * @tc.size: MediumTest
     * @tc.type: Function
     * @tc.level: Level 1
     */
    it("SarJsTest_001", 0, async function (done) {
        console.info('----------------------SarJsTest_001---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.SAR, (error, data) => {
                if (error) {
                    console.error('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.SAR, callback);
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.SAR);
                        done();
                    }, 500);
                } catch (error) {
                    console.error('On fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(false).assertTrue();
                    done();
                }
            });
        } catch (error) {
            console.info('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
            expect(error.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(error.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })

    /*
    * @tc.number: SarJsTest_002
    * @tc.name: SarJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: Issue Number
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("SarJsTest_002", 0, async function (done) {
        console.info('----------------------SarJsTest_002---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.SAR, (error, data) => {
                if (error) {
                    console.error('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.SAR, callback, {'interval': 100000000});
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.SAR);
                        done();
                    }, 500);
                } catch (error) {
                    console.error('On fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(false).assertTrue();
                    done();
                }
            });
        } catch (error) {
            console.info('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
            expect(error.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(error.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })

    /*
    * @tc.number: SarJsTest_003
    * @tc.name: SarJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: Issue Number
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("SarJsTest_003", 0, async function (done) {
        console.info('----------------------SarJsTest_003---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.SAR, (error, data) => {
                if (error) {
                    console.error('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(false).assertTrue();
                    done();
                }
                function onSensorCallback(data) {
                    console.error('SarJsTest_003  callback in');
                    expect(true).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.SAR, onSensorCallback, {'interval': 100000000}, 5);
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.SAR);
                        done();
                    }, 500);
                } catch (error) {
                    console.info('On fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(false).assertTrue();
                    done();
                }
            });
        } catch (error) {
            console.info('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
            expect(error.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(error.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })

    /*
    * @tc.number: SarJsTest_004
    * @tc.name: SarJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: Issue Number
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("SarJsTest_004", 0, async function (done) {
        console.info('----------------------SarJsTest_004---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.SAR, (error, data) => {
                if (error) {
                    console.error('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.SAR, callback, {'interval': -100000000});
                    sensor.off(sensor.SensorId.SAR);
                    done();
                } catch (error) {
                    console.error('On fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(error.code).assertEqual(CommonConstants.SERVICE_EXCEPTION_CODE);
                    expect(error.message).assertEqual(CommonConstants.SERVICE_EXCEPTION_MSG);
                    done();
                }
            });
        } catch (error) {
            console.info('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
            expect(error.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(error.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })

    /*
    * @tc.number: SarJsTest_005
    * @tc.name: SarJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: Issue Number
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("SarJsTest_005", 0, async function (done) {
        console.info('----------------------SarJsTest_005---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.SAR, (error, data) => {
                if (error) {
                    console.error('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.off(-1, callback);
                } catch (error) {
                    expect(error.code).assertEqual(CommonConstants.PARAMETER_ERROR_CODE);
                    done();
                }
            });
        } catch (error) {
            console.error('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
            expect(error.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(error.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })

    /*
    * @tc.number: SarJsTest_006
    * @tc.name: SarJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: Issue Number
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("SarJsTest_006", 0, async function (done) {
        console.info('----------------------SarJsTest_006---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.SAR, (error, data) => {
                if (error) {
                    console.error('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.SAR, callback);
                    sensor.on(sensor.SensorId.SAR, callback2);
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.SAR);
                        done();
                    }, 1000);
                } catch (error) {
                    console.error('On fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(false).assertTrue();
                    done();
                }
            });
        } catch (error) {
            console.error('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
            expect(error.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(error.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })

    /*
    * @tc.number: SarJsTest_007
    * @tc.name: SarJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: Issue Number
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("SarJsTest_007", 0, async function (done) {
        console.info('----------------------SarJsTest_007---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.SAR, (error, data) => {
                if (error) {
                    console.error('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.SAR, callback);
                    sensor.on(sensor.SensorId.SAR, callback2);
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.SAR, callback);
                    }, 500);
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.SAR, callback2);
                        done();
                    }, 1000);
                } catch (error) {
                    console.error('On fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(false).assertTrue();
                    done();
                }
            });
        } catch (error) {
            console.error('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
            expect(error.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(error.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })

    /*
    * @tc.number: SarJsTest_008
    * @tc.name: SarJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: Issue Number
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("SarJsTest_008", 0, async function (done) {
        console.info('----------------------SarJsTest_008---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.SAR, (error, data) => {
                if (error) {
                    console.error('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.SAR, callback, {'interval': 100000000});
                    sensor.on(sensor.SensorId.SAR, callback2, {'interval': 100000000});
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.SAR, callback);
                    }, 500);
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.SAR, callback2);
                        done();
                    }, 1000);
                } catch (error) {
                    console.error('On fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(false).assertTrue();
                    done();
                }
            });
        } catch (error) {
            console.error('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
            expect(error.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(error.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })

    /*
    * @tc.number: SarJsTest_009
    * @tc.name: SarJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: Issue Number
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("SarJsTest_009", 0, async function (done) {
        console.info('----------------------SarJsTest_009---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.SAR, (error, data) => {
                if (error) {
                    console.error('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.SAR, callback, {'interval': 100000000});
                    sensor.on(sensor.SensorId.SAR, callback2, {'interval': 100000000});
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.SAR);
                        done();
                    }, 1000);
                } catch (error) {
                    console.error('On fail, errCode:' + error.code + ' ,msg:' + error.message);
                    expect(false).assertTrue();
                    done();
                }
            });
        } catch (error) {
            console.error('getSingleSensor fail, errCode:' + error.code + ' ,msg:' + error.message);
            expect(error.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(error.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })
})
