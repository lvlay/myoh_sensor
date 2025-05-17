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

import { describe, expect, it } from '@ohos/hypium'
import hidDdkNdkTest from 'libhid_ddk_ndk_test.so'

export default function hidDdkNdkJsTest() {
  describe('hidDdkNdkJsTest', () => {
    const TAG: string = "[hidDdkNdkJsTest]";
    const HID_DDK_SUCCESS: number = 0;

    /**
     * @tc.name       : testHidCreateDevice001
     * @tc.desc       : OH_Hid_CreateDevice Interface testing
     * @tc.type       : Function
     */
    it('testHidCreateDevice001', 0, async (done: Function) => {
      console.info('----------------------testHidCreateDevice001---------------------------');
      try {
        const ret = hidDdkNdkTest.hidCreateDevice();
        console.info("Test result testHidCreateDevice001 : " + ret);
        expect(ret >= HID_DDK_SUCCESS).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testHidCreateDevice001 failed, message is ${err.message}`);
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
        const ret = hidDdkNdkTest.hidEmitEventOne();
        console.info("Test result testHidEmitEvent001 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidEmitEvent001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidEmitEvent002
     * @tc.desc       : OH_Hid_EmitEvent Interface testing
     * @tc.type       : Function
     */
    it('testHidEmitEvent002', 0, async (done: Function) => {
      console.info('----------------------testHidEmitEvent002---------------------------');
      try {
        const ret = hidDdkNdkTest.hidEmitEventTwo();
        console.info("Test result testHidEmitEvent002 : " + ret);
        expect(ret !== HID_DDK_SUCCESS).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testHidEmitEvent002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidEmitEvent003
     * @tc.desc       : OH_Hid_EmitEvent Interface testing
     * @tc.type       : Function
     */
    it('testHidEmitEvent003', 0, async (done: Function) => {
      console.info('----------------------testHidEmitEvent003---------------------------');
      try {
        const ret = hidDdkNdkTest.hidEmitEventThree();
        console.info("Test result testHidEmitEvent003 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidEmitEvent003 failed, message is ${err.message}`);
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
        const ret = hidDdkNdkTest.hidDestroyDeviceOne();
        console.info("Test result testHidDestroyDevice001 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidDestroyDevice001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidDestroyDevice002
     * @tc.desc       : OH_Hid_DestroyDevice Interface testing
     * @tc.type       : Function
     */
    it('testHidDestroyDevice002', 0, async (done: Function) => {
      console.info('----------------------testHidDestroyDevice002---------------------------');
      try {
        const ret = hidDdkNdkTest.hidDestroyDeviceTwo();
        console.info("Test result testHidDestroyDevice002 : " + ret);
        expect(ret != HID_DDK_SUCCESS).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testHidDestroyDevice002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidDestroyDevice003
     * @tc.desc       : OH_Hid_DestroyDevice Interface testing
     * @tc.type       : Function
     */
    it('testHidDestroyDevice003', 0, async (done: Function) => {
      console.info('----------------------testHidDestroyDevice003---------------------------');
      try {
        const ret = hidDdkNdkTest.hidDestroyDeviceThree();
        console.info("Test result testHidDestroyDevice003 : " + ret);
        expect(ret !== HID_DDK_SUCCESS).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testHidDestroyDevice003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

  })
}
