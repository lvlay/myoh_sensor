/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "hdi_support.h"
#include <dlfcn.h>
#include <map>
#include <mutex>
#include <regex>
#include <sstream>
#include <securec.h>
#include <string>
#include <unistd.h>

#include "hdf_base.h"
#include "hdf_log.h"

#define HDF_LOG_TAG load_hdi

namespace {
#ifdef __LITEOS__
static constexpr const char *HDI_SO_EXTENSION = ".so";
#else
static constexpr const char *HDI_SO_EXTENSION = ".z.so";
#endif

constexpr size_t INTERFACE_MATCH_RESIZE = 4;
constexpr size_t INTERFACE_VERSION_MAJOR_INDEX = 1;
constexpr size_t INTERFACE_VERSION_MINOR_INDEX = 2;
constexpr size_t INTERFACE_NAME_INDEX = 3;
static const std::regex reInfDesc("[a-zA-Z_][a-zA-Z0-9_]*(?:\\.[a-zA-Z_][a-zA-Z0-9_]*)*\\."
                                  "[V|v]([0-9]+)_([0-9]+)\\."
                                  "([a-zA-Z_][a-zA-Z0-9_]*)");
using HdiImplInstanceFunc = void *(*)(void);
using HdiImplReleaseFunc = void (*)(void *);
} // namespace

static std::string TransFileName(const std::string &interfaceName)
{
    if (interfaceName.empty()) {
        return interfaceName;
    }

    std::string result;
    for (size_t i = 0; i < interfaceName.size(); i++) {
        char c = interfaceName[i];
        if (std::isupper(c) != 0) {
            if (i > 1) {
                result += '_';
            }
            result += std::tolower(c);
        } else {
            result += c;
        }
    }
    return result;
}

static std::string ParseLibName(const std::string &interfaceName, const std::string &serviceName,
    uint32_t versionMajor, uint32_t versionMinor)
{
    std::ostringstream libName;
    libName << "lib" << interfaceName << "_" << serviceName << "_" << versionMajor << "." << versionMinor;
    libName << HDI_SO_EXTENSION;
    return libName.str();
}

struct HdiImpl {
    HdiImpl() : handler(nullptr), constructor(nullptr), destructor(nullptr), useCount(0) {}
    ~HdiImpl() = default;
    void Unload()
    {
        if (handler != nullptr) {
            dlclose(handler);
        }
    }
    void *handler;
    void *(*constructor)(void);
    void (*destructor)(void *);
    uint32_t useCount;
};

static std::map<std::string, HdiImpl> g_hdiConstructorMap;
static std::mutex g_loaderMutex;

static int32_t ParseInterface(
    const std::string &desc, std::string &interface, std::string &libName, const char *serviceName)
{
    std::smatch result;
    if (!std::regex_match(desc, result, reInfDesc)) {
        return HDF_FAILURE;
    }

    if (result.size() < INTERFACE_MATCH_RESIZE) {
        return HDF_FAILURE;
    }

    uint32_t versionMajor = std::stoul(result[INTERFACE_VERSION_MAJOR_INDEX]);
    uint32_t versionMinor = std::stoul(result[INTERFACE_VERSION_MINOR_INDEX]);
    std::string interfaceName = result[INTERFACE_NAME_INDEX];

    interface = interfaceName[0] == 'I' ? interfaceName.substr(1) : interfaceName;
    if (interface.empty()) {
        return HDF_FAILURE;
    }

    libName = ParseLibName(TransFileName(interface), serviceName, versionMajor, versionMinor);
    return HDF_SUCCESS;
}

/*
 * service name: xxx_service
 * interface descriptor name: ohos.hdi.sample.v1_0.IFoo, the last two are version and interface base name
 * interface: Foo
 * versionMajor: 1
 * versionMinor: 0
 * library name: libfoo_xxx_service_1.0.z.so
 * method name: FooImplGetInstance
 */
void *LoadHdiImpl(const char *desc, const char *serviceName)
{
    if (desc == nullptr || serviceName == nullptr || strlen(desc) == 0 || strlen(serviceName) == 0) {
        HDF_LOGE("%{public}s invalid interface descriptor or service name", __func__);
        return nullptr;
    }

    std::string interfaceName;
    std::string libName;
    if (ParseInterface(desc, interfaceName, libName, serviceName) != HDF_SUCCESS) {
        HDF_LOGE("failed to parse hdi interface info from '%{public}s'", desc);
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(g_loaderMutex);
    auto constructor = g_hdiConstructorMap.find(libName);
    if (constructor != g_hdiConstructorMap.end()) {
        return constructor->second.constructor();
    }

    HdiImpl hdiImpl;
    hdiImpl.handler = dlopen(libName.c_str(), RTLD_LAZY);
    if (hdiImpl.handler == nullptr) {
        HDF_LOGE("%{public}s failed to dlopen, %{public}s", __func__, dlerror());
        return nullptr;
    }
    std::string symName = interfaceName + "ImplGetInstance";
    hdiImpl.constructor = reinterpret_cast<HdiImplInstanceFunc>(dlsym(hdiImpl.handler, symName.data()));
    if (hdiImpl.constructor == nullptr) {
        HDF_LOGE("%{public}s failed to get symbol of '%{public}s', %{public}s", __func__, symName.c_str(), dlerror());
        hdiImpl.Unload();
        return nullptr;
    }
    std::string desSymName = interfaceName + "ImplRelease";
    hdiImpl.destructor = reinterpret_cast<HdiImplReleaseFunc>(dlsym(hdiImpl.handler, desSymName.data()));
    if (hdiImpl.destructor == nullptr) {
        HDF_LOGW("%{public}s failed to get symbol of '%{public}s', %{public}s",
            __func__, desSymName.c_str(), dlerror());
    }

    void *implInstance = hdiImpl.constructor();
    if (implInstance == nullptr) {
        HDF_LOGE("%{public}s no full hdi implementation in %{public}s", __func__, libName.c_str());
        hdiImpl.Unload();
    } else {
        g_hdiConstructorMap.emplace(std::make_pair(libName, std::move(hdiImpl)));
    }
    return implInstance;
}

void UnloadHdiImpl(const char *desc, const char *serviceName, void *impl)
{
    if (desc == nullptr || impl == nullptr) {
        return;
    }

    std::string interfaceName;
    std::string libName;
    if (ParseInterface(desc, interfaceName, libName, serviceName) != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: failed to parse hdi interface info from '%{public}s'", __func__, desc);
        return;
    }
    std::lock_guard<std::mutex> lock(g_loaderMutex);
    auto constructor = g_hdiConstructorMap.find(libName);
    if (constructor != g_hdiConstructorMap.end() && constructor->second.destructor != nullptr) {
        constructor->second.destructor(impl);
    }
}
