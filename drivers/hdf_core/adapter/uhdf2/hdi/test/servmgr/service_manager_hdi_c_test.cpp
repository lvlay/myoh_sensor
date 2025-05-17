/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <devmgr_hdi.h>
#include <gtest/gtest.h>
#include <hdf_io_service_if.h>
#include <hdf_log.h>
#include <hdf_remote_service.h>
#include <hdf_sbuf.h>
#include <hdf_service_status.h>
#include <osal_time.h>
#include <servmgr_hdi.h>
#include <shared_mem.h>
#include <string>
#include <sys/mman.h>

#include "sample_hdi.h"
#include "hdi_support.h"

#define HDF_LOG_TAG service_manager_test

namespace OHOS {
using namespace testing::ext;

#ifdef SAMPLE_DRIVER
static constexpr const char *TEST_SERVICE_NAME = "sample_driver_service";
static constexpr const char *TEST1_SERVICE_NAME = "sample1_driver_service";
static constexpr const char *TEST_SERVICE_INTERFACE_DESC = "hdf.test.sampele_service";
static constexpr int PAYLOAD_NUM = 1234;
static constexpr int WAIT_LOAD_UNLOAD_TIME = 300;
static constexpr int INVALID_DISPATCH_CODE = -1;
#endif

static constexpr const char *TEST_SERVICE_INTERFACE_DESC_INVALID = "____";
static constexpr const char *TEST_SERVICE_INTERFACE_DESC_VOID = "";
static constexpr const char *TEST_SERVICE_INTERFACE_DESC_NULL = nullptr;

class HdfServiceMangerHdiCTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
#ifdef SAMPLE_DRIVER
        struct HDIDeviceManager *devmgr = HDIDeviceManagerGet();
        if (devmgr != nullptr) {
            devmgr->LoadDevice(devmgr, TEST_SERVICE_NAME);
        }
#endif
    }
    static void TearDownTestCase()
    {
#ifdef SAMPLE_DRIVER
        struct HDIDeviceManager *devmgr = HDIDeviceManagerGet();
        if (devmgr != nullptr) {
            devmgr->UnloadDevice(devmgr, TEST_SERVICE_NAME);
        }
#endif
    }
    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest001, TestSize.Level1)
{
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);
    HDIServiceManagerRelease(servmgr);
}

#ifdef SAMPLE_DRIVER
HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest002, TestSize.Level1)
{
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    HDIServiceManagerRelease(servmgr);
    ASSERT_TRUE(sampleService != nullptr);

    bool ret = HdfRemoteServiceSetInterfaceDesc(sampleService, TEST_SERVICE_INTERFACE_DESC);
    ASSERT_EQ(ret, true);

    struct HdfSBuf *data = HdfSbufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSbufTypedObtain(SBUF_IPC);
    ASSERT_TRUE(data != nullptr);
    ASSERT_TRUE(reply != nullptr);
    ret = HdfRemoteServiceWriteInterfaceToken(sampleService, data);
    ASSERT_EQ(ret, true);

    ret = HdfSbufWriteString(data, "sample_service test call");
    ASSERT_EQ(ret, true);

    int status = sampleService->dispatcher->Dispatch(sampleService, SAMPLE_SERVICE_PING, data, reply);
    ASSERT_EQ(status, 0);

    HdfSbufRecycle(data);
    HdfSbufRecycle(reply);
}

static int32_t g_callbackPayload = 0;

static int ServiceManagerTestCallbackDispatch(
    struct HdfRemoteService *service, int code, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    HDF_LOGI("ServiceManagerTestCallbackDispatch called, code = %{public}d", code);
    HdfSbufReadInt32(data, &g_callbackPayload);
    return HDF_SUCCESS;
}

static struct HdfRemoteDispatcher g_callbackDispatcher {
    .Dispatch = ServiceManagerTestCallbackDispatch,
};

HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest003, TestSize.Level1)
{
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    HDIServiceManagerRelease(servmgr);
    ASSERT_TRUE(sampleService != nullptr);
    bool ret = HdfRemoteServiceSetInterfaceDesc(sampleService, TEST_SERVICE_INTERFACE_DESC);
    ASSERT_EQ(ret, true);

    struct HdfRemoteService *callback = HdfRemoteServiceObtain(nullptr, &g_callbackDispatcher);
    ASSERT_NE(callback, nullptr);
    struct HdfSBuf *data = HdfSbufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSbufTypedObtain(SBUF_IPC);
    ASSERT_TRUE(data != nullptr);
    ASSERT_TRUE(reply != nullptr);
    ret = HdfRemoteServiceWriteInterfaceToken(sampleService, data);
    ASSERT_EQ(ret, true);
    int32_t payload = PAYLOAD_NUM;
    HdfSbufWriteInt32(data, payload);
    HdfSbufWriteRemoteService(data, callback);

    int status = sampleService->dispatcher->Dispatch(sampleService, SAMPLE_SERVICE_CALLBACK, data, reply);
    ASSERT_EQ(status, 0);
    ASSERT_EQ(g_callbackPayload, payload);

    HdfSbufRecycle(data);
    HdfSbufRecycle(reply);
}

HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest004, TestSize.Level1)
{
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    HDIServiceManagerRelease(servmgr);
    ASSERT_TRUE(sampleService != nullptr);
    bool ret = HdfRemoteServiceSetInterfaceDesc(sampleService, TEST_SERVICE_INTERFACE_DESC);
    ASSERT_EQ(ret, true);

    struct HdfSBuf *data = HdfSbufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSbufTypedObtain(SBUF_IPC);
    ASSERT_TRUE(data != nullptr);
    ASSERT_TRUE(reply != nullptr);
    ret = HdfRemoteServiceWriteInterfaceToken(sampleService, data);
    ASSERT_EQ(ret, true);
    HdfSbufWriteInt32(data, PAYLOAD_NUM);
    HdfSbufWriteInt32(data, PAYLOAD_NUM);

    int status = sampleService->dispatcher->Dispatch(sampleService, SAMPLE_SERVICE_SUM, data, reply);
    ASSERT_EQ(status, 0);
    int32_t result;
    ret = HdfSbufReadInt32(reply, &result);
    ASSERT_TRUE(ret);

    int32_t expRes = PAYLOAD_NUM + PAYLOAD_NUM;
    ASSERT_EQ(result, expRes);

    HdfSbufRecycle(data);
    HdfSbufRecycle(reply);
}

HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest005, TestSize.Level1)
{
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    HDIServiceManagerRelease(servmgr);
    ASSERT_TRUE(sampleService != nullptr);
    bool ret = HdfRemoteServiceSetInterfaceDesc(sampleService, TEST_SERVICE_INTERFACE_DESC);
    ASSERT_EQ(ret, true);

    struct HdfSBuf *data = HdfSbufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSbufTypedObtain(SBUF_IPC);
    ASSERT_TRUE(data != nullptr);
    ASSERT_TRUE(reply != nullptr);
    ret = HdfRemoteServiceWriteInterfaceToken(sampleService, data);
    ASSERT_EQ(ret, true);
    struct DataBlock dataBlock = {1, 2, "dataBolck", 3};
    ret = DataBlockBlockMarshalling(&dataBlock, data);
    ASSERT_TRUE(ret);

    int status = sampleService->dispatcher->Dispatch(sampleService, SAMPLE_STRUCT_TRANS, data, reply);
    ASSERT_EQ(status, 0);

    struct DataBlock *dataBlock_ = DataBlockBlockUnmarshalling(reply);
    ASSERT_TRUE(dataBlock_ != nullptr);

    ASSERT_EQ(dataBlock_->a, dataBlock.a);
    ASSERT_EQ(dataBlock_->b, dataBlock.b);
    ASSERT_EQ(dataBlock_->c, dataBlock.c);
    ASSERT_TRUE(!strcmp(dataBlock_->str, dataBlock.str));
    DataBlockFree(dataBlock_);

    HdfSbufRecycle(data);
    HdfSbufRecycle(reply);
}

HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest006, TestSize.Level1)
{
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    HDIServiceManagerRelease(servmgr);
    ASSERT_TRUE(sampleService != nullptr);
    bool ret = HdfRemoteServiceSetInterfaceDesc(sampleService, TEST_SERVICE_INTERFACE_DESC);
    ASSERT_EQ(ret, true);

    struct HdfSBuf *data = HdfSbufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSbufTypedObtain(SBUF_IPC);
    ASSERT_TRUE(data != nullptr);
    ASSERT_TRUE(reply != nullptr);

    constexpr int buffersize = 10;
    uint8_t dataBuffer[buffersize];
    for (int i = 0; i < buffersize; i++) {
        dataBuffer[i] = i;
    }
    ret = HdfRemoteServiceWriteInterfaceToken(sampleService, data);
    ASSERT_EQ(ret, true);
    ret = HdfSbufWriteUnpadBuffer(data, dataBuffer, sizeof(dataBuffer));
    ASSERT_TRUE(ret);

    int status = sampleService->dispatcher->Dispatch(sampleService, SAMPLE_BUFFER_TRANS, data, reply);
    ASSERT_EQ(status, 0);

    const uint8_t *retBuffer = HdfSbufReadUnpadBuffer(reply, buffersize);
    ASSERT_TRUE(retBuffer != nullptr);

    for (int i = 0; i < buffersize; i++) {
        ASSERT_EQ(retBuffer[i], i);
    }

    HdfSbufRecycle(data);
    HdfSbufRecycle(reply);
}

HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest007, TestSize.Level1)
{
    struct HDIDeviceManager *devmgr = HDIDeviceManagerGet();
    ASSERT_TRUE(devmgr != nullptr);
    devmgr->UnloadDevice(devmgr, TEST_SERVICE_NAME);

    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);
    OsalMSleep(WAIT_LOAD_UNLOAD_TIME);
    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService == nullptr);

    int ret = devmgr->LoadDevice(devmgr, TEST_SERVICE_NAME);
    ASSERT_EQ(ret, HDF_SUCCESS);
    OsalMSleep(WAIT_LOAD_UNLOAD_TIME);
    sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService != nullptr);
    ret = HdfRemoteServiceSetInterfaceDesc(sampleService, TEST_SERVICE_INTERFACE_DESC);
    ASSERT_EQ(ret, true);

    struct HdfSBuf *data = HdfSbufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSbufTypedObtain(SBUF_IPC);
    ASSERT_TRUE(data != nullptr);
    ASSERT_TRUE(reply != nullptr);

    ret = HdfRemoteServiceWriteInterfaceToken(sampleService, data);
    ASSERT_EQ(ret, true);
    const char *newServName = "sample_driver_service2";
    ret = HdfSbufWriteString(data, newServName);
    ASSERT_TRUE(ret);

    int status = sampleService->dispatcher->Dispatch(sampleService, SAMPLE_REGISTER_DEVICE, data, reply);
    ASSERT_EQ(status, HDF_SUCCESS);

    struct HdfRemoteService *sampleService2 = servmgr->GetService(servmgr, newServName);
    ASSERT_TRUE(sampleService != nullptr);
    ret = HdfRemoteServiceSetInterfaceDesc(sampleService, TEST_SERVICE_INTERFACE_DESC);
    ASSERT_EQ(ret, true);

    HdfSbufFlush(data);
    HdfSbufFlush(reply);
    ret = HdfRemoteServiceWriteInterfaceToken(sampleService, data);
    ASSERT_EQ(ret, true);
    HdfSbufWriteInt32(data, PAYLOAD_NUM);
    HdfSbufWriteInt32(data, PAYLOAD_NUM);

    status = sampleService2->dispatcher->Dispatch(sampleService2, SAMPLE_SERVICE_SUM, data, reply);
    ASSERT_EQ(status, 0);
    int32_t result;
    ret = HdfSbufReadInt32(reply, &result);
    ASSERT_TRUE(ret);

    int32_t expRes = PAYLOAD_NUM + PAYLOAD_NUM;
    ASSERT_EQ(result, expRes);
    HdfRemoteServiceRecycle(sampleService2);

    HdfSbufFlush(data);
    ret = HdfRemoteServiceWriteInterfaceToken(sampleService, data);
    ASSERT_EQ(ret, true);
    ret = HdfSbufWriteString(data, newServName);
    ASSERT_TRUE(ret);

    status = sampleService->dispatcher->Dispatch(sampleService, SAMPLE_UNREGISTER_DEVICE, data, reply);
    ASSERT_EQ(status, HDF_SUCCESS);

    sampleService2 = servmgr->GetService(servmgr, newServName);
    ASSERT_TRUE(sampleService2 == nullptr);

    ret = devmgr->UnloadDevice(devmgr, TEST_SERVICE_NAME);
    ASSERT_EQ(ret, HDF_SUCCESS);

    HdfRemoteServiceRecycle(sampleService);
    OsalMSleep(WAIT_LOAD_UNLOAD_TIME);
    sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService == nullptr);

    HdfSbufRecycle(data);
    HdfSbufRecycle(reply);
}

struct ServiceStatusData {
    ServiceStatusData() : devClass(0), servStatus(0), callbacked(false), waitStatus(SERVIE_STATUS_START) {}
    ~ServiceStatusData() = default;
    std::string servInfo;
    uint16_t devClass;
    uint16_t servStatus;
    bool callbacked;
    uint16_t waitStatus;
};

static void TestOnServiceStatusReceived(struct ServiceStatusListener *listener, struct ServiceStatus *servstat)
{
    struct ServiceStatusData *ssd = static_cast<struct ServiceStatusData *>(listener->priv);
    if (ssd == nullptr) {
        return;
    }
    if (strcmp(servstat->serviceName, TEST_SERVICE_NAME) == 0 && (servstat->status == ssd->waitStatus)) {
        ssd->servInfo = ((servstat->info != nullptr) ? (servstat->info) : (""));
        ssd->devClass = servstat->deviceClass;
        ssd->servStatus = servstat->status;
        ssd->callbacked = true;
    }

    HDF_LOGI("service status listener callback: %{public}s, %{public}d", servstat->serviceName, servstat->status);
}

/*
 * Test service start status listener
 */
HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest008, TestSize.Level1)
{
    struct HDIDeviceManager *devmgr = HDIDeviceManagerGet();
    ASSERT_TRUE(devmgr != nullptr);
    devmgr->UnloadDevice(devmgr, TEST_SERVICE_NAME);

    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);
    OsalMSleep(WAIT_LOAD_UNLOAD_TIME);
    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService == nullptr);

    struct ServiceStatusData ssd;
    ssd.waitStatus = SERVIE_STATUS_START;
    struct ServiceStatusListener *listener = HdiServiceStatusListenerNewInstance();
    listener->callback = TestOnServiceStatusReceived;
    listener->priv = static_cast<void *>(&ssd);

    int status = servmgr->RegisterServiceStatusListener(servmgr, listener, DEVICE_CLASS_DEFAULT);
    ASSERT_EQ(status, HDF_SUCCESS);

    int ret = devmgr->LoadDevice(devmgr, TEST_SERVICE_NAME);
    ASSERT_EQ(ret, HDF_SUCCESS);
    OsalMSleep(WAIT_LOAD_UNLOAD_TIME);
    sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService != nullptr);

    constexpr int WAIT_COUNT = 300;
    int count = WAIT_COUNT;
    while (!ssd.callbacked && count > 0) {
        OsalMSleep(1);
        count--;
    }

    ASSERT_TRUE(ssd.callbacked);
    ASSERT_EQ(ssd.devClass, DEVICE_CLASS_DEFAULT);
    ASSERT_EQ(ssd.servInfo, std::string(TEST_SERVICE_NAME));
    ASSERT_EQ(ssd.servStatus, SERVIE_STATUS_START);

    ssd.callbacked = false;
    ssd.waitStatus = SERVIE_STATUS_STOP;
    ret = devmgr->UnloadDevice(devmgr, TEST_SERVICE_NAME);
    ASSERT_EQ(ret, HDF_SUCCESS);

    count = WAIT_COUNT;
    while (!ssd.callbacked && count > 0) {
        OsalMSleep(1);
        count--;
    }
    ASSERT_TRUE(ssd.callbacked);
    ASSERT_EQ(ssd.devClass, DEVICE_CLASS_DEFAULT);
    ASSERT_EQ(ssd.servInfo, std::string(TEST_SERVICE_NAME));
    ASSERT_EQ(ssd.servStatus, SERVIE_STATUS_STOP);

    status = servmgr->UnregisterServiceStatusListener(servmgr, listener);
    ASSERT_EQ(status, HDF_SUCCESS);

    HdiServiceStatusListenerFree(listener);
}

/*
 * Test service status listener update service info
 */
HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest009, TestSize.Level1)
{
    struct HDIDeviceManager *devmgr = HDIDeviceManagerGet();
    ASSERT_TRUE(devmgr != nullptr);
    devmgr->UnloadDevice(devmgr, TEST_SERVICE_NAME);
    OsalMSleep(WAIT_LOAD_UNLOAD_TIME);
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService == nullptr);

    int ret = devmgr->LoadDevice(devmgr, TEST_SERVICE_NAME);
    ASSERT_EQ(ret, HDF_SUCCESS);
    OsalMSleep(WAIT_LOAD_UNLOAD_TIME);
    sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService != nullptr);
    bool res = HdfRemoteServiceSetInterfaceDesc(sampleService, TEST_SERVICE_INTERFACE_DESC);
    ASSERT_EQ(res, true);

    struct ServiceStatusData ssd;
    struct ServiceStatusListener *listener = HdiServiceStatusListenerNewInstance();
    listener->callback = TestOnServiceStatusReceived;
    listener->priv = static_cast<void *>(&ssd);

    int status = servmgr->RegisterServiceStatusListener(servmgr, listener, DEVICE_CLASS_DEFAULT);
    ASSERT_EQ(status, HDF_SUCCESS);
    constexpr int FIRST_WAIT = 20;
    OsalMSleep(FIRST_WAIT); // skip callback on register

    std::string info = "foo";
    struct HdfSBuf *data = HdfSbufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSbufTypedObtain(SBUF_IPC);
    ASSERT_TRUE(data != nullptr);
    ASSERT_TRUE(reply != nullptr);
    res = HdfRemoteServiceWriteInterfaceToken(sampleService, data);
    ASSERT_EQ(res, true);
    res = HdfSbufWriteString(data, info.data());
    ASSERT_TRUE(res);

    ssd.callbacked = false;
    ssd.waitStatus = SERVIE_STATUS_CHANGE;
    status = sampleService->dispatcher->Dispatch(sampleService, SAMPLE_UPDATE_SERVIE, data, reply);
    ASSERT_EQ(status, HDF_SUCCESS);

    constexpr int WAIT_COUNT = 100;
    int count = WAIT_COUNT;
    while (!ssd.callbacked && count > 0) {
        OsalMSleep(1);
        count--;
    }
    ASSERT_TRUE(ssd.callbacked);
    ASSERT_EQ(ssd.devClass, DEVICE_CLASS_DEFAULT);
    ASSERT_EQ(ssd.servInfo, info);
    ASSERT_EQ(ssd.servStatus, SERVIE_STATUS_CHANGE);

    ret = devmgr->UnloadDevice(devmgr, TEST_SERVICE_NAME);
    ASSERT_EQ(ret, HDF_SUCCESS);

    status = servmgr->UnregisterServiceStatusListener(servmgr, listener);
    ASSERT_EQ(status, HDF_SUCCESS);

    HdiServiceStatusListenerFree(listener);
}

/*
 * Test service status listener unregister
 */
HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest010, TestSize.Level1)
{
    struct HDIDeviceManager *devmgr = HDIDeviceManagerGet();
    ASSERT_TRUE(devmgr != nullptr);
    devmgr->UnloadDevice(devmgr, TEST_SERVICE_NAME);

    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);
    OsalMSleep(WAIT_LOAD_UNLOAD_TIME);
    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService == nullptr);

    struct ServiceStatusData ssd;
    ssd.waitStatus = SERVIE_STATUS_START;
    struct ServiceStatusListener *listener = HdiServiceStatusListenerNewInstance();
    listener->callback = TestOnServiceStatusReceived;
    listener->priv = static_cast<void *>(&ssd);

    int status = servmgr->RegisterServiceStatusListener(servmgr, listener, DEVICE_CLASS_DEFAULT);
    ASSERT_EQ(status, HDF_SUCCESS);

    int ret = devmgr->LoadDevice(devmgr, TEST_SERVICE_NAME);
    ASSERT_EQ(ret, HDF_SUCCESS);

    constexpr int WAIT_COUNT = 300;
    int count = WAIT_COUNT;
    while (!ssd.callbacked && count > 0) {
        OsalMSleep(1);
        count--;
    }
    ASSERT_TRUE(ssd.callbacked);
    ASSERT_EQ(ssd.devClass, DEVICE_CLASS_DEFAULT);
    ASSERT_EQ(ssd.servInfo, std::string(TEST_SERVICE_NAME));
    ASSERT_EQ(ssd.servStatus, SERVIE_STATUS_START);

    sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService != nullptr);

    status = servmgr->UnregisterServiceStatusListener(servmgr, listener);
    ASSERT_EQ(status, HDF_SUCCESS);
    HdiServiceStatusListenerFree(nullptr);
    HdiServiceStatusListenerFree(listener);

    ret = devmgr->UnloadDevice(devmgr, TEST_SERVICE_NAME);
    ASSERT_EQ(ret, HDF_SUCCESS);

    ssd.callbacked = false;
    OsalMSleep(WAIT_COUNT);
    ASSERT_FALSE(ssd.callbacked);
}
#endif

/*
 * Test shared mem interface
 */
HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest011, TestSize.Level1)
{
    constexpr int mapSize = 128;
    int memFd = SharedMemCreate("shared_mem_test", mapSize);
    ASSERT_TRUE(memFd >= 0);

    void *ptr = mmap(nullptr, mapSize, PROT_READ | PROT_WRITE, MAP_SHARED, memFd, 0);
    ASSERT_NE(ptr, MAP_FAILED);

    uint8_t *data = static_cast<uint8_t *>(ptr);
    for (int i = 0; i < mapSize; i++) {
        data[i] = i;
    }

    for (int i = 0; i < mapSize; i++) {
        ASSERT_EQ(data[i], i);
    }

    int memSize = SharedMemGetSize(memFd);
    ASSERT_EQ(memSize, mapSize);

    auto ret = munmap(ptr, mapSize);
    ASSERT_EQ(ret, 0);

    close(memFd);
}

#ifdef SAMPLE_DRIVER
/*
 * Test get service set by interfacedesc
 */
HWTEST_F(HdfServiceMangerHdiCTest, ListServiceByInterfaceDescTest001, TestSize.Level1)
{
    struct HDIDeviceManager *devmgr = HDIDeviceManagerGet();
    ASSERT_TRUE(devmgr != nullptr);
    devmgr->UnloadDevice(devmgr, TEST_SERVICE_NAME);
    OsalMSleep(WAIT_LOAD_UNLOAD_TIME);
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService == nullptr);

    int ret = devmgr->LoadDevice(devmgr, TEST_SERVICE_NAME);
    ASSERT_EQ(ret, HDF_SUCCESS);
    OsalMSleep(WAIT_LOAD_UNLOAD_TIME);
    sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService != nullptr);

    struct HdiServiceSet *serviceSet = servmgr->ListServiceByInterfaceDesc(servmgr, TEST_SERVICE_INTERFACE_DESC);
    HDIServiceManagerRelease(servmgr);
    ASSERT_TRUE(serviceSet != nullptr);
    constexpr int  sampleServiceCount = 1;
    ASSERT_TRUE(serviceSet->count == sampleServiceCount);
    ASSERT_TRUE(strcmp(serviceSet->serviceNames[0], TEST_SERVICE_NAME) == 0);
    ret = HdiServiceSetRelease(serviceSet);
    ASSERT_TRUE(ret == HDF_SUCCESS);
}
#endif

HWTEST_F(HdfServiceMangerHdiCTest, ListServiceByInterfaceDescTest002, TestSize.Level1)
{
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdiServiceSet *serviceSet =
        servmgr->ListServiceByInterfaceDesc(servmgr, TEST_SERVICE_INTERFACE_DESC_INVALID);
    HDIServiceManagerRelease(servmgr);
    ASSERT_TRUE(serviceSet == nullptr);
}

HWTEST_F(HdfServiceMangerHdiCTest, ListServiceByInterfaceDescTest003, TestSize.Level1)
{
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdiServiceSet *serviceSet = servmgr->ListServiceByInterfaceDesc(servmgr, TEST_SERVICE_INTERFACE_DESC_VOID);
    HDIServiceManagerRelease(servmgr);
    ASSERT_TRUE(serviceSet == nullptr);
}

HWTEST_F(HdfServiceMangerHdiCTest, ListServiceByInterfaceDescTest004, TestSize.Level1)
{
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdiServiceSet *serviceSet = servmgr->ListServiceByInterfaceDesc(servmgr, TEST_SERVICE_INTERFACE_DESC_NULL);
    HDIServiceManagerRelease(servmgr);
    ASSERT_TRUE(serviceSet == nullptr);
}

#ifdef SAMPLE_DRIVER
HWTEST_F(HdfServiceMangerHdiCTest, ListServiceByInterfaceDescTest005, TestSize.Level1)
{
    struct HDIDeviceManager *devmgr = HDIDeviceManagerGet();
    ASSERT_TRUE(devmgr != nullptr);
    devmgr->UnloadDevice(devmgr, TEST_SERVICE_NAME);
    OsalMSleep(WAIT_LOAD_UNLOAD_TIME);
    devmgr->UnloadDevice(devmgr, TEST1_SERVICE_NAME);
    OsalMSleep(WAIT_LOAD_UNLOAD_TIME);
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService == nullptr);
    struct HdfRemoteService *sample1Service = servmgr->GetService(servmgr, TEST1_SERVICE_NAME);
    ASSERT_TRUE(sample1Service == nullptr);

    int ret = devmgr->LoadDevice(devmgr, TEST_SERVICE_NAME);
    ASSERT_EQ(ret, HDF_SUCCESS);
    OsalMSleep(WAIT_LOAD_UNLOAD_TIME);
    sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService != nullptr);

    ret = devmgr->LoadDevice(devmgr, TEST1_SERVICE_NAME);
    ASSERT_EQ(ret, HDF_SUCCESS);
    OsalMSleep(WAIT_LOAD_UNLOAD_TIME);
    sample1Service = servmgr->GetService(servmgr, TEST1_SERVICE_NAME);
    ASSERT_TRUE(sample1Service != nullptr);

    struct HdiServiceSet *serviceSet = servmgr->ListServiceByInterfaceDesc(servmgr, TEST_SERVICE_INTERFACE_DESC);
    HDIServiceManagerRelease(servmgr);
    ASSERT_TRUE(serviceSet != nullptr);
    constexpr int sampleServiceCount = 2;
    ASSERT_TRUE(serviceSet->count == sampleServiceCount);
    ASSERT_TRUE(strcmp(serviceSet->serviceNames[0], TEST_SERVICE_NAME) == 0);
    ASSERT_TRUE(strcmp(serviceSet->serviceNames[1], TEST1_SERVICE_NAME) == 0);
    ret = HdiServiceSetRelease(serviceSet);
    ASSERT_TRUE(ret == HDF_SUCCESS);
    ret = devmgr->UnloadDevice(devmgr, TEST1_SERVICE_NAME);
    ASSERT_TRUE(ret == HDF_SUCCESS);
    HDIDeviceManagerRelease(devmgr);
}
#endif

HWTEST_F(HdfServiceMangerHdiCTest, DevMgrQueryUsableDeviceTest, TestSize.Level1)
{
    struct HDIDeviceManager *devmgr = HDIDeviceManagerGet();
    ASSERT_TRUE(devmgr != nullptr);

    struct DeviceInfoList list;
    int32_t ret = devmgr->QueryUsableDeviceInfo(devmgr, &list);
    ASSERT_TRUE(ret == HDF_SUCCESS);

    devmgr->FreeQueryDeviceList(devmgr, &list);
    HDIDeviceManagerRelease(devmgr);
}

HWTEST_F(HdfServiceMangerHdiCTest, DevMgrQueryUnusableDeviceTest, TestSize.Level1)
{
    struct HDIDeviceManager *devmgr = HDIDeviceManagerGet();
    ASSERT_TRUE(devmgr != nullptr);

    struct DeviceInfoList list;
    int32_t ret = devmgr->QueryUnusableDeviceInfo(devmgr, &list);
    ASSERT_TRUE(ret == HDF_SUCCESS);

    devmgr->FreeQueryDeviceList(devmgr, &list);
    HDIDeviceManagerRelease(devmgr);
}

HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest018, TestSize.Level1)
{
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, NULL);
    ASSERT_TRUE(sampleService == nullptr);

    int status = servmgr->RegisterServiceStatusListener(servmgr, NULL, DEVICE_CLASS_DEFAULT);
    ASSERT_EQ(status, HDF_ERR_INVALID_PARAM);

    status = servmgr->UnregisterServiceStatusListener(servmgr, NULL);
    ASSERT_EQ(status, HDF_ERR_INVALID_PARAM);

    status = HdiServiceSetRelease(NULL);
    ASSERT_TRUE(status == HDF_SUCCESS);

    HDIServiceManagerRelease(servmgr);
    HDIServiceManagerRelease(NULL);
}

#ifdef SAMPLE_DRIVER
HWTEST_F(HdfServiceMangerHdiCTest, RemoteServiceTest001, TestSize.Level1) {
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService != nullptr);
    struct HdfDeathRecipient* recipient = nullptr;
    HdfRemoteServiceAddDeathRecipient(sampleService, recipient);
    HdfRemoteServiceRemoveDeathRecipient(sampleService, recipient);
}

HWTEST_F(HdfServiceMangerHdiCTest, RemoteServiceTest002, TestSize.Level1) {
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService != nullptr);
    int deviceServiceManagerSAID = 5100;
    int status = HdfRemoteServiceRegister(deviceServiceManagerSAID, sampleService);
    ASSERT_TRUE(status == HDF_SUCCESS);
}

HWTEST_F(HdfServiceMangerHdiCTest, RemoteServiceTest003, TestSize.Level1) {
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService != nullptr);

    struct HdfSBuf *dataSbuf = HdfSbufTypedObtain(SBUF_IPC);
    struct HdfSBuf *replySbuf = HdfSbufTypedObtain(SBUF_IPC);

    int status = HdfRemoteServiceDefaultDispatch(sampleService, INVALID_DISPATCH_CODE, dataSbuf, replySbuf);
    ASSERT_TRUE(status != HDF_SUCCESS);
}
#endif

HWTEST_F(HdfServiceMangerHdiCTest, DevMgrTest, TestSize.Level1)
{
    struct HDIDeviceManager *devmgr = HDIDeviceManagerGet();
    ASSERT_TRUE(devmgr != nullptr);

    int ret = devmgr->QueryUnusableDeviceInfo(devmgr, NULL);
    ASSERT_TRUE(ret == HDF_ERR_INVALID_PARAM);

    ret = devmgr->QueryUsableDeviceInfo(devmgr, NULL);
    ASSERT_TRUE(ret == HDF_ERR_INVALID_PARAM);

    ret = devmgr->LoadDevice(devmgr, NULL);
    ASSERT_EQ(ret, HDF_ERR_INVALID_PARAM);

    ret = devmgr->UnloadDevice(devmgr, NULL);
    ASSERT_EQ(ret, HDF_ERR_INVALID_PARAM);

    devmgr->FreeQueryDeviceList(devmgr, NULL);
    HDIDeviceManagerRelease(devmgr);
    HDIDeviceManagerRelease(NULL);
}

HWTEST_F(HdfServiceMangerHdiCTest, HdiSupportTest, TestSize.Level1)
{
    ASSERT_EQ(LoadHdiImpl(NULL, NULL), nullptr);
    UnloadHdiImpl(NULL, NULL, NULL);
}
} // namespace OHOS