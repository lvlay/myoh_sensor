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

describe("SensorFrequencyJsTest", function () {
    function callback(data) {
        console.info('callback' + JSON.stringify(data));
        expect(typeof(data.x)).assertEqual('number');
        expect(typeof(data.y)).assertEqual('number');
        expect(typeof(data.z)).assertEqual('number');
    }

    function callback2(data) {
        console.info('callback2' + JSON.stringify(data));
        expect(typeof(data.alpha)).assertEqual('number');
        expect(typeof(data.beta)).assertEqual('number');
        expect(typeof(data.gamma)).assertEqual('number');
    }

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
     * @tc.name: SensorFrequencyJsTest_001
     * @tc.desc: verify app info is not null
     * @tc.size: MediumTest
     * @tc.type: Function
     * @tc.level: Level 1
     * @tc.require: Issue Number
     * @tc.number: SensorFrequencyJsTest_001
     */
    it("SensorFrequencyJsTest_001", 0, async function (done) {
        console.info('----------------------SensorFrequencyJsTest_001---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.ACCELEROMETER, (err, data) => {
                if (err) {
                    console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.ACCELEROMETER, callback, {'interval': 'game'});
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.ACCELEROMETER);
                        done();
                    }, 500);
                } catch (err) {
                    console.error('On fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
            });
        } catch (err) {
            console.error('Sensor is not support');
            expect(err.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(err.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })

    /*
    * @tc.name: SensorFrequencyJsTest_002
    * @tc.desc: verify app info is not null
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    * @tc.require: Issue Number
    * @tc.number: SensorFrequencyJsTest_002
    */
    it("SensorFrequencyJsTest_002", 0, async function (done) {
        console.info('----------------------SensorFrequencyJsTest_002---------------------------');
            try {
                sensor.getSingleSensor(sensor.SensorId.ACCELEROMETER, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.on(sensor.SensorId.ACCELEROMETER, callback, {'interval': 'ui'});
                        setTimeout(() => {
                            sensor.off(sensor.SensorId.ACCELEROMETER);
                            done();
                        }, 500);
                    } catch (err) {
                        console.error('On fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                });
            } catch (err) {
                console.error('Sensor is not support');
                expect(err.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
                expect(err.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
                done();
            }
        })

    /*
    * @tc.name: SensorFrequencyJsTest_003
    * @tc.desc: verify app info is not null
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    * @tc.require: Issue Number
    * @tc.number: SensorFrequencyJsTest_003
    */
    it("SensorFrequencyJsTest_003", 0, async function (done) {
        console.info('----------------------SensorFrequencyJsTest_003---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.ACCELEROMETER, (err, data) => {
                if (err) {
                    console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.ACCELEROMETER, callback, {'interval': 'normal'});
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.ACCELEROMETER);
                        done();
                    }, 500);
                } catch (err) {
                    console.error('On fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
            });
        } catch (err) {
            console.error('Sensor is not support');
            expect(err.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(err.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })

    /*
    * @tc.name: SensorFrequencyJsTest_004
    * @tc.desc: verify app info is not null
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    * @tc.require: Issue Number
    * @tc.number: SensorFrequencyJsTest_004
    */
    it("SensorFrequencyJsTest_004", 0, async function (done) {
        console.info('----------------------SensorFrequencyJsTest_004---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.GYROSCOPE, (err, data) => {
                if (err) {
                    console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.GYROSCOPE, callback, {'interval': 'game'});
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.GYROSCOPE);
                        done();
                    }, 500);
                } catch (err) {
                    console.error('On fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
            });
        } catch (err) {
            console.error('Sensor is not support');
            expect(err.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(err.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })

    /*
    * @tc.name: SensorFrequencyJsTest_005
    * @tc.desc: verify app info is not null
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    * @tc.require: Issue Number
    * @tc.number: SensorFrequencyJsTest_005
    */
    it("SensorFrequencyJsTest_005", 0, async function (done) {
        console.info('----------------------SensorFrequencyJsTest_005---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.GYROSCOPE, (err, data) => {
                if (err) {
                    console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.GYROSCOPE, callback, {'interval': 'ui'});
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.GYROSCOPE);
                        done();
                    }, 500);
                } catch (err) {
                    console.error('On fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
            });
        } catch (err) {
            console.error('Sensor is not support');
            expect(err.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(err.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })

    /*
    * @tc.name: SensorFrequencyJsTest_006
    * @tc.desc: verify app info is not null
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    * @tc.require: Issue Number
    * @tc.number: SensorFrequencyJsTest_006
    */
    it("SensorFrequencyJsTest_006", 0, async function (done) {
        console.info('----------------------SensorFrequencyJsTest_006---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.GYROSCOPE, (err, data) => {
                if (err) {
                    console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.GYROSCOPE, callback, {'interval': 'normal'});
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.GYROSCOPE);
                        done();
                    }, 500);
                } catch (err) {
                    console.error('On fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
            });
        } catch (err) {
            console.error('Sensor is not support');
            expect(err.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(err.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })

    /*
    * @tc.name: SensorFrequencyJsTest_007
    * @tc.desc: verify app info is not null
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    * @tc.require: Issue Number
    * @tc.number: SensorFrequencyJsTest_007
    */
    it("SensorFrequencyJsTest_007", 0, async function (done) {
        console.info('----------------------SensorFrequencyJsTest_007---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.ORIENTATION, (err, data) => {
                if (err) {
                    console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.ORIENTATION, callback2, {'interval': 'game'});
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.ORIENTATION);
                        done();
                    }, 500);
                } catch (err) {
                    console.error('On fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
            });
        } catch (err) {
            console.error('Sensor is not support');
            expect(err.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(err.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })

    /*
    * @tc.name: SensorFrequencyJsTest_008
    * @tc.desc: verify app info is not null
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    * @tc.require: Issue Number
    * @tc.number: SensorFrequencyJsTest_008
    */
    it("SensorFrequencyJsTest_008", 0, async function (done) {
        console.info('----------------------SensorFrequencyJsTest_008---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.ORIENTATION, (err, data) => {
                if (err) {
                    console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.ORIENTATION, callback2, {'interval': 'ui'});
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.ORIENTATION);
                        done();
                    }, 500);
                } catch (err) {
                    console.error('On fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
            });
        } catch (err) {
            console.error('Sensor is not support');
            expect(err.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(err.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })

    /*
    * @tc.name: SensorFrequencyJsTest_009
    * @tc.desc: verify app info is not null
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    * @tc.require: Issue Number
    * @tc.number: SensorFrequencyJsTest_009
    */
    it("SensorFrequencyJsTest_009", 0, async function (done) {
        console.info('----------------------SensorFrequencyJsTest_009---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.ORIENTATION, (err, data) => {
                if (err) {
                    console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.ORIENTATION, callback2, {'interval': 'normal'});
                    setTimeout(() => {
                        sensor.off(sensor.SensorId.ORIENTATION);
                        done();
                    }, 500);
                } catch (err) {
                    console.error('On fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
            });
        } catch (err) {
            console.error('Sensor is not support');
            expect(err.code).assertEqual(CommonConstants.SENSOR_NO_SUPPORT_CODE);
            expect(err.message).assertEqual(CommonConstants.SENSOR_NO_SUPPOR_MSG);
            done();
        }
    })
})