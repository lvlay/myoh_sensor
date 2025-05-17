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

describe("PedometerJsTest", function () {
    function callback(data) {
        console.info('callback' + JSON.stringify(data));
        expect(typeof(data.steps)).assertEqual('number');
    }

    function callback2(data) {
        console.info('callback2' + JSON.stringify(data));
        expect(typeof(data.steps)).assertEqual('number');
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
     * @tc.number: PedometerJsTest_001
     * @tc.name: PedometerJsTest
     * @tc.desc: verify app info is not null
     * @tc.type: FUNC
     * @tc.require: AR000HPEMQ
     * @tc.size: MediumTest
     * @tc.type: Function
     * @tc.level: Level 1
     */
    it("PedometerJsTest_001", 0, async function (done) {
        console.info('----------------------PedometerJsTest_001---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.on(sensor.SensorId.PEDOMETER, callback);
                        setTimeout(() => {
                            sensor.off(sensor.SensorId.PEDOMETER);
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
    * @tc.number: PedometerJsTest_002
    * @tc.name: PedometerJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMQ
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerJsTest_002", 0, async function (done) {
        console.info('----------------------PedometerJsTest_002---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.on(sensor.SensorId.PEDOMETER, 5);
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
    * @tc.number: PedometerJsTest_003
    * @tc.name: PedometerJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMQ
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerJsTest_003", 0, async function (done) {
        console.info('----------------------PedometerJsTest_003---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.once(sensor.SensorId.PEDOMETER, callback);
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
    * @tc.number: PedometerJsTest_004
    * @tc.name: PedometerJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMQ
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerJsTest_004", 0, async function (done) {
        console.info('----------------------PedometerJsTest_004---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.once(sensor.SensorId.PEDOMETER, callback, 5);
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
    * @tc.number: PedometerJsTest_005
    * @tc.name: PedometerJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMQ
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerJsTest_005", 0, async function (done) {
        console.info('----------------------PedometerJsTest_005---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.once(sensor.SensorId.PEDOMETER, 5);
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
    * @tc.number: PedometerJsTest_006
    * @tc.name: PedometerJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMQ
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerJsTest_006", 0, async function (done) {
        console.info('----------------------PedometerJsTest_006---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER, (err, data) => {
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
    * @tc.number: PedometerJsTest_007
    * @tc.name: PedometerJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMQ
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerJsTest_007", 0, async function (done) {
        console.info('----------------------PedometerJsTest_007---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.on(sensor.SensorId.PEDOMETER, callback);
                        sensor.on(sensor.SensorId.PEDOMETER, callback2);
                        setTimeout(() => {
                            sensor.off(sensor.SensorId.PEDOMETER);
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
    * @tc.number: PedometerJsTest_008
    * @tc.name: PedometerJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMQ
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerJsTest_008", 0, async function (done) {
        console.info('----------------------PedometerJsTest_008---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.on(sensor.SensorId.PEDOMETER, callback);
                        sensor.on(sensor.SensorId.PEDOMETER, callback2);
                        setTimeout(() => {
                            sensor.off(sensor.SensorId.PEDOMETER, callback);
                        }, 500);
                        setTimeout(() => {
                            sensor.off(sensor.SensorId.PEDOMETER, callback2);
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
    * @tc.number: PedometerJsTest_009
    * @tc.name: PedometerJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMQ
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerJsTest_009", 0, async function (done) {
        console.info('----------------------PedometerJsTest_009---------------------------');
        let atManager = abilityAccessCtrl.createAtManager();
        await atManager.grantUserGrantedPermission(CommonConstants.tokenID, CommonConstants.permissionNameUser,
            CommonConstants.PermissionFlag.PERMISSION_USER_SET)
        .then(() => {
            try {
                sensor.getSingleSensor(sensor.SensorId.PEDOMETER, (err, data) => {
                    if (err) {
                        console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                        expect(false).assertTrue();
                        done();
                    }
                    try {
                        sensor.on(sensor.SensorId.PEDOMETER, callback);
                        sensor.once(sensor.SensorId.PEDOMETER, callback2);
                        setTimeout(() => {
                            sensor.off(sensor.SensorId.PEDOMETER);
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
    * @tc.number: PedometerJsTest_010
    * @tc.name: PedometerJsTest
    * @tc.desc: verify app info is not null
    * @tc.type: FUNC
    * @tc.require: AR000HPEMQ
    * @tc.size: MediumTest
    * @tc.type: Function
    * @tc.level: Level 1
    */
    it("PedometerJsTest_010", 0, async function (done) {
        console.info('----------------------PedometerJsTest_010---------------------------');
        try {
            sensor.getSingleSensor(sensor.SensorId.PEDOMETER, (err, data) => {
                if (err) {
                    console.error('getSingleSensor fail, errCode:' + err.code + ' ,msg:' + err.message);
                    expect(false).assertTrue();
                    done();
                }
                try {
                    sensor.on(sensor.SensorId.PEDOMETER, callback);
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