/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "stream_server.h"

#include <cinttypes>

#include <sys/socket.h>

#include "sensor_errors.h"

#undef LOG_TAG
#define LOG_TAG "StreamServer"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t INVALID_PID = -1;
constexpr int32_t INVALID_FD = -1;
} // namespace

StreamServer::~StreamServer()
{
    CALL_LOG_ENTER;
    std::lock_guard<std::mutex> sessionLock(sessionMutex_);
    idxPidMap_.clear();
    for (const auto &item : sessionsMap_) {
        item.second->Close();
    }
    sessionsMap_.clear();
}

int32_t StreamServer::GetClientFd(int32_t pid)
{
    std::lock_guard<std::mutex> sessionLock(sessionMutex_);
    auto it = idxPidMap_.find(pid);
    return it == idxPidMap_.end() ? INVALID_FD : it->second;
}

int32_t StreamServer::GetClientPid(int32_t fd)
{
    std::lock_guard<std::mutex> sessionLock(sessionMutex_);
    auto it = sessionsMap_.find(fd);
    return it == sessionsMap_.end() ? INVALID_PID : it->second->GetPid();
}

SessionPtr StreamServer::GetSession(int32_t fd)
{
    std::lock_guard<std::mutex> sessionLock(sessionMutex_);
    auto it = sessionsMap_.find(fd);
    if (it == sessionsMap_.end()) {
        SEN_HILOGE("Session not found");
        return nullptr;
    }
    CHKPP(it->second);
    return it->second->GetSharedPtr();
}

SessionPtr StreamServer::GetSessionByPid(int32_t pid)
{
    int32_t fd = GetClientFd(pid);
    if (fd <= 0) {
        SEN_HILOGE("Session not found");
        return nullptr;
    }
    return GetSession(fd);
}

int32_t StreamServer::AddSocketPairInfo(int32_t uid, int32_t pid, int32_t tokenType,
    int32_t &serverFd, int32_t &clientFd)
{
    CALL_LOG_ENTER;
    int32_t sockFds[2] = { -1 };
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockFds) != 0) {
        SEN_HILOGE("Socketpair failed, errno:%{public}d", errno);
        return ERROR;
    }
    serverFd = sockFds[0];
    clientFd = sockFds[1];
    if (serverFd < 0 || clientFd < 0) {
        SEN_HILOGE("ServerFd or clientFd is invalid");
        return ERROR;
    }
    static constexpr size_t bufferSize = 32 * 1024;
    SessionPtr sess = nullptr;
    if (setsockopt(serverFd, SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(bufferSize)) != 0) {
        SEN_HILOGE("Setsockopt serverFd send buffer size failed, errno:%{public}d", errno);
        goto CLOSE_SOCK;
    }
    if (setsockopt(serverFd, SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize)) != 0) {
        SEN_HILOGE("Setsockopt serverFd recv buffer size failed, errno:%{public}d", errno);
        goto CLOSE_SOCK;
    }
    if (setsockopt(clientFd, SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(bufferSize)) != 0) {
        SEN_HILOGE("Setsockopt clientFd send buffer size failed, errno:%{public}d", errno);
        goto CLOSE_SOCK;
    }
    if (setsockopt(clientFd, SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize)) != 0) {
        SEN_HILOGE("Setsockopt clientFd recv buffer size failed, errno:%{public}d", errno);
        goto CLOSE_SOCK;
    }
    sess = std::make_shared<StreamSession>("", serverFd, uid, pid);
    sess->SetTokenType(tokenType);
    if (!AddSession(sess)) {
        SEN_HILOGE("AddSession fail");
        goto CLOSE_SOCK;
    }
    return ERR_OK;

CLOSE_SOCK:
    close(serverFd);
    close(clientFd);
    return ERROR;
}

bool StreamServer::AddSession(SessionPtr sess)
{
    CALL_LOG_ENTER;
    CHKPF(sess);
    auto fd = sess->GetFd();
    if (fd < 0) {
        SEN_HILOGE("Fd is Invalid");
        return false;
    }
    auto pid = sess->GetPid();
    if (pid <= 0) {
        SEN_HILOGE("Pid is invalid");
        return false;
    }
    std::lock_guard<std::mutex> sessionLock(sessionMutex_);
    if (sessionsMap_.size() > MAX_SESSON_ALARM) {
        SEN_HILOGE("Too many clients, size:%{public}zu", sessionsMap_.size());
        return false;
    }
    idxPidMap_[pid] = fd;
    sessionsMap_[fd] = sess;
    return true;
}

void StreamServer::DelSession(int32_t pid)
{
    CALL_LOG_ENTER;
    std::lock_guard<std::mutex> sessionLock(sessionMutex_);
    auto pidIt = idxPidMap_.find(pid);
    if (pidIt == idxPidMap_.end()) {
        SEN_HILOGW("Pid session not exist");
        return;
    }
    int32_t fd = pidIt->second;
    idxPidMap_.erase(pidIt);
    auto fdIt = sessionsMap_.find(fd);
    if (fdIt != sessionsMap_.end()) {
        sessionsMap_.erase(fdIt);
    }
    if (fd >= 0) {
        int32_t ret = close(fd);
        if (ret != 0) {
            SEN_HILOGE("Socket fd close failed, ret:%{public}d, errno:%{public}d", ret, errno);
        }
    }
}
} // namespace Sensors
} // namespace OHOS