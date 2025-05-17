/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

/**
 * Common constants for test
 */
export default class CommonConstants {
  /**
   * Error message of parameter
   */
  static PARAMETER_ERROR_MSG = 'The parameter invalid.';

  /**
   * Excption message of service
   */
  static SERVICE_EXCEPTION_MSG = 'Service exception.';

  /**
   * Excption message of sensorId no support
   */
    static SENSOR_NO_SUPPOR_MSG = 'The sensor is not supported by the device.';

  /**
   * Error code of parameter
   */
  static PARAMETER_ERROR_CODE = 401;

  /**
   * Exception code of service
   */
  static SERVICE_EXCEPTION_CODE = 14500101;

  /**
   * Exception code of sensor not supported
   */
  static SENSOR_NO_SUPPORT_CODE = 14500102;

  /**
   * Eps
   */
  static EPS = 0.01;

  /**
   * Delay time
   */
  static DELAY_TIME = 1000;

  /**
   * Sensor interval
   */
  static SENSOR_INTERVAL = 100000000;

  /**
   * Invalid sensor interval
   */
  static INVALID_SENSOR_INTERVAL = -1;

  /**
   * Invalid sensor id
   */
  static INVALID_SENSOR_ID = -1;

  /**
   * token id
   */
  static tokenID = undefined

  /**
   * user permission
   */
  static permissionNameUser = 'ohos.permission.ACTIVITY_MOTION'

  /**
   * Error code of permission denied
   */
  static PERMISSION_DENIED_CODE = 201

  /**
   * Error message of permission denied
   */
  static PERMISSION_DENIED_MSG = 'Permission denied.'
  /**
   * Flag of permission
   */
  static PermissionFlag = {
    PERMISSION_USER_SET:1,
    PERMISSION_USER_FIXED:2,
    PERMISSION_SYSTEM_FIXED:4
  }
}