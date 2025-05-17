/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import { beforeAll, describe, it, expect } from "@ohos/hypium"
import ddkPermissionTest from 'libddk_permission_js_test.so'
import deviceManager from "@ohos.driver.deviceManager";


export default function DdkPermissionJsTest() {
  let deviceId: number = -1;

  describe('DdkPermissionJsTest', () => {

    const TAG: string = "[DdkPermissionJsTest]";
    const DDK_ERR_NOPERM: number = -19;

    beforeAll(async () => {
      try {
        let devices: Array<deviceManager.Device> = deviceManager.queryDevices(deviceManager.BusType.USB);
        for (let item of devices) {
          let device: deviceManager.USBDevice = item as deviceManager.USBDevice;
          deviceId = device.deviceId;
          console.info(`Device id is ${device.deviceId}`)
        }
      } catch (error) {
        console.error(`Failed to query device. Code is ${error.code}, message is ${error.message}`);
      }
    })

    /**
     * @tc.name       : testUsbInit001
     * @tc.desc       : OH_Usb_Init Interface testing
     * @tc.type       : Function
     */
    it('testUsbInit001', 0, async (done: Function) => {
      console.info('----------------------testUsbInit001---------------------------');
      try {
        const ret = ddkPermissionTest.usbInit();
        console.info("Test Result testUsbInit001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbInit001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbGetDeviceDescriptor001
     * @tc.desc       : OH_Usb_GetDeviceDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testUsbGetDeviceDescriptor001', 0, async (done: Function) => {
      console.info('----------------------testUsbGetDeviceDescriptor001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbGetDeviceDescriptor(deviceId);
        console.info("Test Result testUsbGetDeviceDescriptor001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbGetDeviceDescriptor001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbGetConfigDescriptor001
     * @tc.desc       : OH_Usb_GetConfigDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testUsbGetConfigDescriptor001', 0, async (done: Function) => {
      console.info('----------------------testUsbGetConfigDescriptor001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbGetConfigDescriptor(deviceId);
        console.info("Test Result testUsbGetConfigDescriptor001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbGetConfigDescriptor001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbFreeConfigDescriptor001
     * @tc.desc       : OH_Usb_FreeConfigDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testUsbFreeConfigDescriptor001', 0, async (done: Function) => {
      console.info('----------------------testUsbFreeConfigDescriptor001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbFreeConfigDescriptor(deviceId);
        console.info("Test Result testUsbFreeConfigDescriptor001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbFreeConfigDescriptor001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbClaimInterface001
     * @tc.desc       : OH_Usb_ClaimInterface Interface testing
     * @tc.type       : Function
     */
    it('testUsbClaimInterface001', 0, async (done: Function) => {
      console.info('----------------------testUsbClaimInterface001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbClaimInterface(deviceId);
        console.info("Test Result testUsbClaimInterface001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbClaimInterface001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbReleaseInterface001
     * @tc.desc       : OH_Usb_ReleaseInterface Interface testing
     * @tc.type       : Function
     */
    it('testUsbReleaseInterface001', 0, async (done: Function) => {
      console.info('----------------------testUsbReleaseInterface001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbReleaseInterface(deviceId);
        console.info("Test Result testUsbReleaseInterface001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbReleaseInterface001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSelectInterfaceSetting001
     * @tc.desc       : OH_Usb_SelectInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testUsbSelectInterfaceSetting001', 0, async (done: Function) => {
      console.info('----------------------testUsbSelectInterfaceSetting001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbSelectInterfaceSetting(deviceId);
        console.info("Test Result testUsbSelectInterfaceSetting001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSelectInterfaceSetting001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbGetCurrentInterfaceSetting001
     * @tc.desc       : OH_Usb_GetCurrentInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testUsbGetCurrentInterfaceSetting001', 0, async (done: Function) => {
      console.info('----------------------testUsbGetCurrentInterfaceSetting001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbGetCurrentInterfaceSetting(deviceId);
        console.info("Test Result testUsbGetCurrentInterfaceSetting001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbGetCurrentInterfaceSetting001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendControlReadRequest001
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendControlReadRequest001', 0, async (done: Function) => {
      console.info('----------------------testUsbSendControlReadRequest001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbSendControlReadRequest(deviceId);
        console.info("Test Result testUsbSendControlReadRequest001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendControlReadRequest001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendControlWriteRequest001
     * @tc.desc       : OH_Usb_SendControlWriteRequest Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendControlWriteRequest001', 0, async (done: Function) => {
      console.info('----------------------testUsbSendControlWriteRequest001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbSendControlWriteRequest(deviceId);
        console.info("Test Result testUsbSendControlWriteRequest001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendControlWriteRequest001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });


    /**
     * @tc.name       : testUsbSendPipeRequest001
     * @tc.desc       : OH_Usb_SendPipeRequest Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendPipeRequest001', 0, async (done: Function) => {
      console.info('----------------------testUsbSendPipeRequest001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbSendPipeRequest(deviceId);
        console.info("Test Result testUsbSendPipeRequest001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendPipeRequest001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbCreateDeviceMemMap001
     * @tc.desc       : OH_Usb_CreateDeviceMemMap Interface testing
     * @tc.type       : Function
     */
    it('testUsbCreateDeviceMemMap001', 0, async (done: Function) => {
      console.info('----------------------testUsbCreateDeviceMemMap001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbCreateDeviceMemMape(deviceId);
        console.info("Test Result testUsbCreateDeviceMemMap001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbCreateDeviceMemMap001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendPipeRequestWithAshmem001
     * @tc.desc       : OH_Usb_SendPipeRequestWithAshmem Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendPipeRequestWithAshmem001', 0, async (done: Function) => {
      console.info('----------------------testUsbSendPipeRequestWithAshmem001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbSendPipeRequestWithAshmem(deviceId);
        console.info("Test Result testUsbSendPipeRequestWithAshmem001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendPipeRequestWithAshmem001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testhidCreateDevice001
     * @tc.desc       : OH_Hid_CreateDevice Interface testing
     * @tc.type       : Function
     */
    it('testhidCreateDevice001', 0, async (done: Function) => {
      console.info('----------------------testhidCreateDevice001---------------------------');
      try {
        const ret = ddkPermissionTest.hidCreateDevice();
        console.info("Test Result testhidCreateDevice001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testhidCreateDevice001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidEmitEvent001
     * @tc.desc       : OH_Hid_EmitEvent Interface testing
     * @tc.type       : Function
     */
    it('testHidEmitEvent001', 0, async (done: Function) => {
      console.info('----------------------testHidEmitEvent001---------------------------');
      try {
        const ret = ddkPermissionTest.hidEmitEvent();
        console.info("Test Result testHidEmitEvent001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidEmitEvent001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

     /**
     * @tc.name       : testHidDestroyDevice001
     * @tc.desc       : OH_Hid_DestroyDevice Interface testing
     * @tc.type       : Function
     */
    it('testHidDestroyDevice001', 0, async (done: Function) => {
      console.info('----------------------testHidDestroyDevice001---------------------------');
      try {
        const ret = ddkPermissionTest.hidDestroyDevice();
        console.info("Test Result testHidDestroyDevice001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidDestroyDevice001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });
  })
}
