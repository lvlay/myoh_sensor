/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "bluetooth_address.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <hdf_log.h>
#include <unistd.h>
#include <dlfcn.h>
#include "securec.h"

#ifdef LOG_DOMAIN
#undef LOG_DOMAIN
#endif
#define LOG_DOMAIN 0xD000105

namespace OHOS {
namespace HDI {
namespace Bluetooth {
namespace Hci {
namespace {
constexpr int ADDRESS_STR_LEN = 17;
constexpr int ADDRESS_SIZE = 6;
}  // namespace

BluetoothAddress::BluetoothAddress()
{
    address_.resize(ADDRESS_SIZE);
}

constexpr int START_POS = 6;
constexpr int END_POS = 13;
constexpr int ADDR_BYTE = 18;
std::string GetEncryptAddr(std::string addr)
{
    if (addr.empty() || addr.length() != ADDRESS_STR_LEN) {
        HDF_LOGE("addr is invalid.");
        return std::string("");
    }
    std::string tmp = "**:**:**:**:**:**";
    std::string out = addr;
    for (int i = START_POS; i <= END_POS; i++) {
        out[i] = tmp[i];
    }
    return out;
}

void BluetoothAddress::ParseAddressToString(std::vector<uint8_t> &address, std::string &outString)
{
    char temp[ADDR_BYTE] = {0};
    int ret = sprintf_s(temp, sizeof(temp), "%02X:%02X:%02X:%02X:%02X:%02X",
        address[0], address[1], address[2], address[3], address[4], address[5]);
    if (ret == -1) {
        HDF_LOGE("ConvertAddr sprintf_s return error, ret -1");
    }
    outString = temp;
}

int BluetoothAddress::ParseAddressFromString(const std::string &string) const
{
    size_t offset = 0;
    int bytesIndex = 0;
    int readCount = 0;
    for (bytesIndex = 0; bytesIndex < ADDRESS_SIZE && offset < string.size(); bytesIndex++) {
        readCount = 0;
        if (sscanf_s(&string[offset], "%02hhx:%n", &address_[bytesIndex], &readCount) > 0) {
            if (readCount == 0 && bytesIndex != ADDRESS_SIZE - 1) {
                return bytesIndex;
            }
            offset += readCount;
        } else {
            break;
        }
    }

    return bytesIndex;
}

bool BluetoothAddress::GetConstantAddress(char *address, int len)
{
    if (address == nullptr || len < ADDRESS_STR_LEN + 1) {
        HDF_LOGE("GetConstantAddress buf error");
        return false;
    }

    void *libMac = dlopen(BT_MAC_LIB, RTLD_LAZY);
    if (libMac == nullptr) {
        HDF_LOGI("GetConstantAddress no mac lib ready for dlopen");
        return false;
    }

    using GetMacFun = int (*)(unsigned int, char*, int);
    GetMacFun getMac = reinterpret_cast<GetMacFun>(dlsym(libMac, GET_BT_MAC_SYMBOL_NAME));
    if (getMac == nullptr) {
        HDF_LOGE("GetConstantAddress dlsym error");
        dlclose(libMac);
        return false;
    }

    int ret = getMac(MAC_TYPE_BLUETOOTH, address, len);
    HDF_LOGI("GetConstantAddress ret: %{public}d", ret);
    dlclose(libMac);
    return (ret == 0);
}

std::shared_ptr<BluetoothAddress> BluetoothAddress::GetDeviceAddress(const std::string &path)
{
    const int bufsize = 256;
    char buf[bufsize] = {0};
    int addrFd = open(path.c_str(), O_RDONLY);
    if (addrFd < 0) {
        HDF_LOGI("GetDeviceAddress open %{public}s.", path.c_str());
        int newFd = open(path.c_str(), O_RDWR | O_CREAT, 00644);
        HDF_LOGI("GetDeviceAddress open newFd %{public}d.", newFd);
        char addressStr[ADDRESS_STR_LEN + 1] = {"00:11:22:33:44:55"};
        if (!GetConstantAddress(addressStr, ADDRESS_STR_LEN + 1)) {
            auto tmpPtr = GenerateDeviceAddress();
            std::string strAddress;
            ParseAddressToString(tmpPtr->address_, strAddress);
            HDF_LOGI("device mac addr: %{public}s", GetEncryptAddr(strAddress).c_str());
            int ret = strcpy_s(addressStr, ADDRESS_STR_LEN + 1, strAddress.c_str());
            if (ret != 0) {
                HDF_LOGI("ParseAddressToString strcpy_s err!");
            }
        }

        if (newFd >= 0) {
            int fdRet = write(newFd, addressStr, ADDRESS_STR_LEN);
            if (fdRet < 0) {
                strerror_r(errno, buf, sizeof(buf));
                HDF_LOGI("GetDeviceAddress addr write failed, err:%{public}s.", buf);
            }
            close(newFd);
        }
        auto ptr = std::make_shared<BluetoothAddress>();
        if (ptr->ParseAddressFromString(addressStr) != ADDRESS_SIZE) {
            return nullptr;
        }
        return ptr;
    }

    char addressStr[ADDRESS_STR_LEN + 1] = {0};
    if (read(addrFd, addressStr, ADDRESS_STR_LEN) != ADDRESS_STR_LEN) {
        HDF_LOGE("read %{public}s failed.", path.c_str());
        close(addrFd);
        return nullptr;
    }
    close(addrFd);

    auto ptr = std::make_shared<BluetoothAddress>();
    if (ptr->ParseAddressFromString(addressStr) != ADDRESS_SIZE) {
        return nullptr;
    }

    return ptr;
}

std::shared_ptr<BluetoothAddress> BluetoothAddress::GenerateDeviceAddress(const std::string &prefix)
{
    const int bufsize = 256;
    char buf[bufsize] = {0};
    auto ptr = std::make_shared<BluetoothAddress>();
    char addressStr[ADDRESS_STR_LEN + 1] = {"00:11:22:33:44:55"};
    ptr->ParseAddressFromString(addressStr);
    int prefixCount = ptr->ParseAddressFromString(prefix);
    if (prefixCount < ADDRESS_SIZE) {
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0) {
            strerror_r(errno, buf, sizeof(buf));
            HDF_LOGE("open /dev/urandom failed err:%{public}s.", buf);
            return ptr;
        }
        if (read(fd, &ptr->address_[prefixCount], ADDRESS_SIZE - prefixCount) != ADDRESS_SIZE - prefixCount) {
            strerror_r(errno, buf, sizeof(buf));
            HDF_LOGE("read /dev/urandom failed err:%{public}s.", buf);
        }
        close(fd);
    }
    return ptr;
}

void BluetoothAddress::ReadAddress(std::vector<uint8_t> &address) const
{
    address = address_;
}

void BluetoothAddress::ReadAddress(std::string &address) const
{
    address.resize(ADDRESS_STR_LEN + 1);

    int offset = 0;
    for (int ii = 0; ii < ADDRESS_SIZE; ii++) {
        int ret = snprintf_s(
            &address[offset], (ADDRESS_STR_LEN + 1) - offset, ADDRESS_STR_LEN - offset, "%02x:", address_[ii]);
        if (ret < 0) {
            break;
        }
        offset += ret;
    }
}
}  // namespace Hci
}  // namespace Bluetooth
}  // namespace HDI
}  // namespace OHOS
