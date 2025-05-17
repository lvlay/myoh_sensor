/*
 * Copyright (c) 2022 Chipsea Technologies (Shenzhen) Corp., Ltd.
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
import { Callback } from './basic';

/**
 * This module provides the capability to subscribe to medical data.
 *
 * @since 8
 * @sysCap SystemCapability.Sensors.Medical_sensor
 * @import import medical from '@ohos.medical'
 * @permission ohos.permission.HEALTH_DATA
 */
declare namespace medical {
    /**
     * Subscribe to medical sensor data, If the API is called multiple times, the last call takes effect.
     * @param type Indicate the medical type to listen for, {@code MedicalSensorType.TYPE_ID_PHOTOPLETHYSMOGRAPH}.
     * @param options Optional parameters specifying the interval at which medical sensor data is reported, {@code Options}.
     * @permission ohos.permission.PHOTOPLETHYSMOGRAPH
     * @since 8
     */
    function on(type: MedicalSensorType.TYPE_ID_PHOTOPLETHYSMOGRAPH, callback: Callback<PpgResponse>, options?: Options): void;

    /**
     * Unsubscribe to medical sensor data once.
     * @param type Indicate the medical sensor type to listen for, {@code MedicalSensorType}.
     * @permission N/A
     * @since 8
     */
    function off(type: MedicalSensorType.TYPE_ID_PHOTOPLETHYSMOGRAPH, callback?: Callback<PpgResponse>): void;

    /**
     * Subscribe to the medical sensor's optional parameters.
     * @sysCap SystemCapability.Sensors.Medical_sensor
     */
    interface Options {
        interval?: number; /**< Sensor event reporting event interval */
    }

    /**
     * The type of medical sensor.
     * @sysCap SystemCapability.Sensors.Medical_sensor
     */
    enum MedicalSensorType {
        TYPE_ID_PHOTOPLETHYSMOGRAPH = 129,    /**< Photoplethysmography */
    }

    interface PpgResponse {
        dataArray: Array<number>; /**< Acceleration x-axis component */
    }
 }

 export default medical;