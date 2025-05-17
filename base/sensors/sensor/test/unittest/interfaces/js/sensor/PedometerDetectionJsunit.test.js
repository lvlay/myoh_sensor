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
import abilityAccessCtrl from '@ohos.abilityAccessCtrl'
import bundle from '@ohos.bundle'
import CommonConstants from './CommonConstants';
import osAccount from '@ohos.account.osAccount'
import sensor from '@ohos.sensor'

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe("PedometerDetectionJsTest", function () {
    function callback(data) {
        console.info('callback' + JSON.stringify(data));
        expect(typeof(data.scalar)).assertEqual('number');
    }

    function callback2(data) {
        console.info('callback2' + JSON.stringify(data));
        expect(typeof(data.scalar)).assertEqual('number');
    }
 
    beforeAll(async function() {
        /*
         * @tc.setup: setup invoked before all testcases
         */
         console.info('beforeAll called')
         try {
            let accountManager = osAccount.getAccountManager();
            let userId = await accountManager.getOsAccountLocalIdFromProcess();
            let appInfo = await bundle.getApplicationInfo('com.example.myapplication', 0, userId);
            CommonConstants.tokenID = appInfo.accessTokenId;
            console.log('AccessTokenId accessTokenId:' + appInfo.accessTokenId + ', name:' + appInfo.name +
                ' ,bundleName:' + appInfo.bundleName)
         } catch(err) {
            console.error('exception in, msg:' + JSON.stringify(err))
         }
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
        let atManager = abilityAccessCtrl.createAtManager();
        atManager.revokeUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .catch((err) => {
            console.info('err:' + JSON.stringify(err));
            expect(err.code).assertEqual(ERR_NOT_HAVE_PERMISSION);
        })
    })

    /*
     * @tc.number: PedometerDetectionJsTest_001
     * @tc.name: PedometerDetectionJsTest
     * @tc.desc: verify app info is not null
     * @tc.type: FUNC
     * @tc.require: AR000HPEMR
     * @tc.size: MediumTest
     * @tc.type: Function
     * @tc.level: Level 1
     */
    it("PedometerDetectionJsTest_001", 0, async function (done) {
        console.info('----------------------PedometerDetectionJsTest_001---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER_DETECTION, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.on(sensor.SensorId.PEDOMETER_DETECTION, callback);
                        setTimeout(() => {
                            sensor.off(sensor.SensorId.PEDOMETER_DETECTION);
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
        .catch ((err) => {
            console.error('err:' + JSON.stringify(err));
            done();
        })
    })

    /*
    * @tc.number: PedometerDetectionJsTest_002
    * @tc.name: PedometerDetectionJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMR
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerDetectionJsTest_002", 0, async function (done) {
        console.info('----------------------PedometerDetectionJsTest_002---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER_DETECTION, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.on(sensor.SensorId.PEDOMETER_DETECTION, 5);
                    } catch (err) {
                        console.error('On fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(err.code).assertEqual(CommonConstants.PARAMETER_ERROR_CODE);
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
        .catch ((err) => {
            console.error('err:' + JSON.stringify(err));
            done();
        })
    })

    /*
    * @tc.number: PedometerDetectionJsTest_003
    * @tc.name: PedometerDetectionJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMR
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerDetectionJsTest_003", 0, async function (done) {
        console.info('----------------------PedometerDetectionJsTest_003---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER_DETECTION, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.once(sensor.SensorId.PEDOMETER_DETECTION, callback);
                        setTimeout(() => {
                            expect(true).assertTrue();
                            done();
                        }, 500);
                    } catch (err) {
                        console.error('Once fail, errCode:' + err.code + ' ,msg:' + err.message);
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
        .catch ((err) => {
            console.error('err:' + JSON.stringify(err));
            done();
        })
    })

    /*
    * @tc.number: PedometerDetectionJsTest_004
    * @tc.name: PedometerDetectionJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMR
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerDetectionJsTest_004", 0, async function (done) {
        console.info('----------------------PedometerDetectionJsTest_004---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER_DETECTION, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.once(sensor.SensorId.PEDOMETER_DETECTION, callback, 5);
                        setTimeout(() => {
                            expect(true).assertTrue();
                            done();
                        }, 500);
                    } catch (err) {
                        console.error('Once fail, errCode:' + err.code + ' ,msg:' + err.message);
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
        .catch ((err) => {
            console.error('err:' + JSON.stringify(err));
            done();
        })
    })

    /*
    * @tc.number: PedometerDetectionJsTest_005
    * @tc.name: PedometerDetectionJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMR
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerDetectionJsTest_005", 0, async function (done) {
        console.info('----------------------PedometerDetectionJsTest_005---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER_DETECTION, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.once(sensor.SensorId.PEDOMETER_DETECTION, 5);
                    } catch (err) {
                        console.error('On fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(err.code).assertEqual(CommonConstants.PARAMETER_ERROR_CODE);
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
        .catch ((err) => {
            console.error('err:' + JSON.stringify(err));
            done();
        })
    })

    /*
    * @tc.number: PedometerDetectionJsTest_006
    * @tc.name: PedometerDetectionJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMR
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerDetectionJsTest_006", 0, async function (done) {
        console.info('----------------------PedometerDetectionJsTest_006---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER_DETECTION, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.off(-1, callback);
                    } catch (err) {
                        expect(err.code).assertEqual(CommonConstants.PARAMETER_ERROR_CODE);
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
        .catch ((err) => {
            console.error('err:' + JSON.stringify(err));
            done();
        })
    })

    /*
    * @tc.number: PedometerDetectionJsTest_007
    * @tc.name: PedometerDetectionJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMR
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerDetectionJsTest_007", 0, async function (done) {
        console.info('----------------------PedometerDetectionJsTest_007---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER_DETECTION, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.on(sensor.SensorId.PEDOMETER_DETECTION, callback);
                        sensor.on(sensor.SensorId.PEDOMETER_DETECTION, callback2);
                        setTimeout(() => {
                            sensor.off(sensor.SensorId.PEDOMETER_DETECTION);
                            done();
                        }, 1000);
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
        .catch ((err) => {
            console.error('err:' + JSON.stringify(err));
            done();
        })
    })

    /*
    * @tc.number: PedometerDetectionJsTest_008
    * @tc.name: PedometerDetectionJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMR
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerDetectionJsTest_008", 0, async function (done) {
        console.info('----------------------PedometerDetectionJsTest_008---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER_DETECTION, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.on(sensor.SensorId.PEDOMETER_DETECTION, callback);
                        sensor.on(sensor.SensorId.PEDOMETER_DETECTION, callback2);
                        setTimeout(() => {
                            sensor.off(sensor.SensorId.PEDOMETER_DETECTION, callback);
                        }, 500);
                        setTimeout(() => {
                            sensor.off(sensor.SensorId.PEDOMETER_DETECTION, callback2);
                            done();
                        }, 1000);
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
        .catch ((err) => {
            console.error('err:' + JSON.stringify(err));
            done();
        })
    })

    /*
    * @tc.number: PedometerDetectionJsTest_009
    * @tc.name: PedometerDetectionJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMR
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerDetectionJsTest_009", 0, async function (done) {
        console.info('----------------------PedometerDetectionJsTest_009---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER_DETECTION, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.on(sensor.SensorId.PEDOMETER_DETECTION, callback);
                        sensor.once(sensor.SensorId.PEDOMETER_DETECTION, callback2);
                        setTimeout(() => {
                            sensor.off(sensor.SensorId.PEDOMETER_DETECTION);
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
        .catch ((err) => {
            console.error('err:' + JSON.stringify(err));
            done();
        })
    })

    /*
    * @tc.number: PedometerDetectionJsTest_010
    * @tc.name: PedometerDetectionJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMR
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerDetectionJsTest_010", 0, async function (done) {
        console.info('----------------------PedometerDetectionJsTest_010---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.PEDOMETER_DETECTION, (err, data) => {
                if (err) {
                    console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.PEDOMETER_DETECTION, callback);
                } catch (err) {
                    console.error('On fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(err.code).assertEqual(CommonConstants.PERMISSION_DENIED_CODE);
                    expect(err.message).assertEqual(CommonConstants.PERMISSION_DENIED_MSG);
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