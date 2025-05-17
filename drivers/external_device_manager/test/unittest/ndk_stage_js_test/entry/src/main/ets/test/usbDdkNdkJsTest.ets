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
import usbDdkNdkTest from 'libusb_ddk_ndk_test.so'
import deviceManager from "@ohos.driver.deviceManager";


export default function usbDdkNdkJsTest() {
  let deviceId: number = -1;

  describe('usbDdkNdkJsTest', () => {

    const TAG: string = "[usbDdkNdkJsTest]";
    const USB_DDK_SUCCESS: number = 0;
    const USB_DDK_FAILED: number = -1;
    const USB_DDK_INVALID_PARAMETER: number = -2;
    const USB_DDK_INVALID_OPERATION: number = -4;

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
        const ret = usbDdkNdkTest.usbInit();
        console.info("Test Result testUsbInit001 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbInit001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbRelease001
     * @tc.desc       : OH_Usb_Release Interface testing
     * @tc.type       : Function
     */
    it('testUsbRelease001', 0, async (done: Function) => {
      console.info('----------------------testUsbRelease001---------------------------');
      try {
        const ret = usbDdkNdkTest.usbRelease();
        console.info("Test Result testUsbRelease001 : " + ret);
        expect(ret).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testUsbRelease001 failed, message is ${err.message}`);
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
        const ret = usbDdkNdkTest.usbGetDeviceDescriptorOne(deviceId);
        console.info("Test Result testUsbGetDeviceDescriptor001 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbGetDeviceDescriptor001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbGetDeviceDescriptor002
     * @tc.desc       : OH_Usb_GetDeviceDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testUsbGetDeviceDescriptor002', 0, async (done: Function) => {
      console.info('----------------------testUsbGetDeviceDescriptor002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbGetDeviceDescriptorTwo(deviceId);
        console.info("Test Result testUsbGetDeviceDescriptor002 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbGetDeviceDescriptor002 failed, message is ${err.message}`);
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
        const ret = usbDdkNdkTest.usbGetConfigDescriptorOne(deviceId);
        console.info("Test Result testUsbGetConfigDescriptor001 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbGetConfigDescriptor001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbGetConfigDescriptor002
     * @tc.desc       : OH_Usb_GetConfigDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testUsbGetConfigDescriptor002', 0, async (done: Function) => {
      console.info('----------------------testUsbGetConfigDescriptor002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbGetConfigDescriptorTwo();
        console.info("Test Result testUsbGetConfigDescriptor002 : " + ret);
        expect(ret).assertEqual(USB_DDK_FAILED);
        done();
      } catch (err) {
        console.error(TAG, `testUsbGetConfigDescriptor002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbGetConfigDescriptor003
     * @tc.desc       : OH_Usb_GetConfigDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testUsbGetConfigDescriptor003', 0, async (done: Function) => {
      console.info('----------------------testUsbGetConfigDescriptor003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbGetConfigDescriptorThree(deviceId);
        console.info("Test Result testUsbGetConfigDescriptor003 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbGetConfigDescriptor003 failed, message is ${err.message}`);
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
        const ret = usbDdkNdkTest.usbFreeConfigDescriptor(deviceId);
        console.info("Test Result testUsbFreeConfigDescriptor001 : " + ret);
        expect(ret).assertTrue();
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
        const ret = usbDdkNdkTest.usbClaimInterfaceOne(deviceId);
        console.info("Test Result testUsbClaimInterface001 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbClaimInterface001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbClaimInterface002
     * @tc.desc       : OH_Usb_ClaimInterface Interface testing
     * @tc.type       : Function
     */
    it('testUsbClaimInterface002', 0, async (done: Function) => {
      console.info('----------------------testUsbClaimInterface002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbClaimInterfaceTwo();
        console.info("Test Result testUsbClaimInterface002 : " + ret);
        expect(ret).assertEqual(USB_DDK_FAILED);
        done();
      } catch (err) {
        console.error(TAG, `testUsbClaimInterface002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbClaimInterface003
     * @tc.desc       : OH_Usb_ClaimInterface Interface testing
     * @tc.type       : Function
     */
    it('testUsbClaimInterface003', 0, async (done: Function) => {
      console.info('----------------------testUsbClaimInterface003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbClaimInterfaceThree(deviceId);
        console.info("Test Result testUsbClaimInterface003 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbClaimInterface003 failed, message is ${err.message}`);
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
        const ret = usbDdkNdkTest.usbReleaseInterface(deviceId);
        console.info("Test Result testUsbReleaseInterface001 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
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
        const ret = usbDdkNdkTest.usbSelectInterfaceSettingOne(deviceId);
        console.info("Test Result testUsbSelectInterfaceSetting001 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSelectInterfaceSetting001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSelectInterfaceSetting002
     * @tc.desc       : OH_Usb_SelectInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testUsbSelectInterfaceSetting002', 0, async (done: Function) => {
      console.info('----------------------testUsbSelectInterfaceSetting002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbSelectInterfaceSettingTwo(deviceId);
        console.info("Test Result testUsbSelectInterfaceSetting002 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSelectInterfaceSetting002 failed, message is ${err.message}`);
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
        const ret = usbDdkNdkTest.usbGetCurrentInterfaceSettingOne(deviceId);
        console.info("Test Result testUsbGetCurrentInterfaceSetting001 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbGetCurrentInterfaceSetting001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbGetCurrentInterfaceSetting002
     * @tc.desc       : OH_Usb_GetCurrentInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testUsbGetCurrentInterfaceSetting002', 0, async (done: Function) => {
      console.info('----------------------testUsbGetCurrentInterfaceSetting002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbGetCurrentInterfaceSettingTwo(deviceId);
        console.info("Test Result testUsbGetCurrentInterfaceSetting002 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testUsbGetCurrentInterfaceSetting002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbGetCurrentInterfaceSetting003
     * @tc.desc       : OH_Usb_GetCurrentInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testUsbGetCurrentInterfaceSetting003', 0, async (done: Function) => {
      console.info('----------------------testUsbGetCurrentInterfaceSetting003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbGetCurrentInterfaceSettingThree(deviceId);
        console.info("Test Result testUsbGetCurrentInterfaceSetting003 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbGetCurrentInterfaceSetting003 failed, message is ${err.message}`);
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
        const ret = usbDdkNdkTest.usbSendControlReadRequestOne(deviceId);
        console.info("Test Result testUsbSendControlReadRequest001 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendControlReadRequest001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendControlReadRequest002
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendControlReadRequest002', 0, async (done: Function) => {
      console.info('----------------------testUsbSendControlReadRequest002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbSendControlReadRequestTwo(deviceId);
        console.info("Test Result testUsbSendControlReadRequest002 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendControlReadRequest002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendControlReadRequest003
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendControlReadRequest003', 0, async (done: Function) => {
      console.info('----------------------testUsbSendControlReadRequest003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbSendControlReadRequestThree(deviceId);
        console.info("Test Result testUsbSendControlReadRequest003 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendControlReadRequest003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendControlReadRequest004
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendControlReadRequest004', 0, async (done: Function) => {
      console.info('----------------------testUsbSendControlReadRequest004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbSendControlReadRequestFour(deviceId);
        console.info("Test Result testUsbSendControlReadRequest004 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendControlReadRequest004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendControlReadRequest005
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendControlReadRequest005', 0, async (done: Function) => {
      console.info('----------------------testUsbSendControlReadRequest005---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbSendControlReadRequestFive(deviceId);
        console.info("Test Result testUsbSendControlReadRequest005 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendControlReadRequest005 failed, message is ${err.message}`);
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
        const ret = usbDdkNdkTest.usbSendControlWriteRequestOne(deviceId);
        console.info("Test Result testUsbSendControlWriteRequest001 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendControlWriteRequest001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendControlWriteRequest002
     * @tc.desc       : OH_Usb_SendControlWriteRequest Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendControlWriteRequest002', 0, async (done: Function) => {
      console.info('----------------------testUsbSendControlWriteRequest002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbSendControlWriteRequestTwo(deviceId);
        console.info("Test Result testUsbSendControlWriteRequest002 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendControlWriteRequest002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendControlWriteRequest003
     * @tc.desc       : OH_Usb_SendControlWriteRequest Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendControlWriteRequest003', 0, async (done: Function) => {
      console.info('----------------------testUsbSendControlWriteRequest003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbSendControlWriteRequestThree(deviceId);
        console.info("Test Result testUsbSendControlWriteRequest003 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendControlWriteRequest003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendControlWriteRequest004
     * @tc.desc       : OH_Usb_SendControlWriteRequest Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendControlWriteRequest004', 0, async (done: Function) => {
      console.info('----------------------testUsbSendControlWriteRequest004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbSendControlWriteRequestFour(deviceId);
        console.info("Test Result testUsbSendControlWriteRequest004 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendControlWriteRequest004 failed, message is ${err.message}`);
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
        const ret = usbDdkNdkTest.usbSendPipeRequestOne(deviceId);
        console.info("Test Result testUsbSendPipeRequest001 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendPipeRequest001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendPipeRequest002
     * @tc.desc       : OH_Usb_SendPipeRequest Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendPipeRequest002', 0, async (done: Function) => {
      console.info('----------------------testUsbSendPipeRequest002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbSendPipeRequestTwo(deviceId);
        console.info("Test Result testUsbSendPipeRequest002 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendPipeRequest002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendPipeRequest003
     * @tc.desc       : OH_Usb_SendPipeRequest Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendPipeRequest003', 0, async (done: Function) => {
      console.info('----------------------testUsbSendPipeRequest003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbSendPipeRequestThree(deviceId);
        console.info("Test Result testUsbSendPipeRequest003 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendPipeRequest003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendPipeRequest004
     * @tc.desc       : OH_Usb_SendPipeRequest Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendPipeRequest004', 0, async (done: Function) => {
      console.info('----------------------testUsbSendPipeRequest004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbSendPipeRequestFour(deviceId);
        console.info("Test Result testUsbSendPipeRequest004 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendPipeRequest004 failed, message is ${err.message}`);
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
        const ret = usbDdkNdkTest.usbCreateDeviceMemMapOne(deviceId);
        console.info("Test Result testUsbCreateDeviceMemMap001 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbCreateDeviceMemMap001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbCreateDeviceMemMap002
     * @tc.desc       : OH_Usb_CreateDeviceMemMap Interface testing
     * @tc.type       : Function
     */
    it('testUsbCreateDeviceMemMap002', 0, async (done: Function) => {
      console.info('----------------------testUsbCreateDeviceMemMap002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbCreateDeviceMemMapTwo(deviceId);
        console.info("Test Result testUsbCreateDeviceMemMap002 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbCreateDeviceMemMap002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbDestroyDeviceMemMap001
     * @tc.desc       : OH_Usb_DestroyDeviceMemMap Interface testing
     * @tc.type       : Function
     */
    it('testUsbDestroyDeviceMemMap001', 0, async (done: Function) => {
      console.info('----------------------testUsbDestroyDeviceMemMap001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbDestroyDeviceMemMap(deviceId);
        console.info("Test Result testUsbDestroyDeviceMemMap001 : " + ret);
        expect(ret).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testUsbDestroyDeviceMemMap001 failed, message is ${err.message}`);
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
        const ret = usbDdkNdkTest.usbSendPipeRequestWithAshmemOne(deviceId);
        console.info("Test Result testUsbSendPipeRequestWithAshmem001 : " + ret);
        expect(ret).assertEqual(USB_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendPipeRequestWithAshmem001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendPipeRequestWithAshmem002
     * @tc.desc       : OH_Usb_SendPipeRequestWithAshmem Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendPipeRequestWithAshmem002', 0, async (done: Function) => {
      console.info('----------------------testUsbSendPipeRequestWithAshmem002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbSendPipeRequestWithAshmemTwo(deviceId);
        console.info("Test Result testUsbSendPipeRequestWithAshmem002 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendPipeRequestWithAshmem002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendPipeRequestWithAshmem003
     * @tc.desc       : OH_Usb_SendPipeRequestWithAshmem Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendPipeRequestWithAshmem003', 0, async (done: Function) => {
      console.info('----------------------testUsbSendPipeRequestWithAshmem003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbSendPipeRequestWithAshmemThree(deviceId);
        console.info("Test Result testUsbSendPipeRequestWithAshmem003 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendPipeRequestWithAshmem003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendPipeRequestWithAshmem004
     * @tc.desc       : OH_Usb_SendPipeRequestWithAshmem Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendPipeRequestWithAshmem004', 0, async (done: Function) => {
      console.info('----------------------testUsbSendPipeRequestWithAshmem004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = usbDdkNdkTest.usbSendPipeRequestWithAshmemFour(deviceId);
        console.info("Test Result testUsbSendPipeRequestWithAshmem004 : " + ret);
        expect(ret).assertEqual(USB_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendPipeRequestWithAshmem004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

  })
}
