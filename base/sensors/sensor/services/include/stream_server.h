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

#ifndef STREAM_SERVER_H
#define STREAM_SERVER_H

#include <map>
#include <mutex>

#include "stream_session.h"
#include "stream_socket.h"

namespace OHOS {
namespace Sensors {
class StreamServer : public StreamSocket {
public:
    StreamServer() = default;
    virtual ~StreamServer();
    int32_t GetClientFd(int32_t pid);
    int32_t GetClientPid(int32_t fd);
    SessionPtr GetSession(int32_t fd);
    SessionPtr GetSessionByPid(int32_t pid);
    int32_t AddSocketPairInfo(int32_t uid, int32_t pid, int32_t tokenType, int32_t &serverFd, int32_t &clientFd);

protected:
    bool AddSession(SessionPtr sess);
    void DelSession(int32_t pid);
    std::mutex sessionMutex_;
    std::map<int32_t, int32_t> idxPidMap_;
    std::map<int32_t, SessionPtr> sessionsMap_;
};
} // namespace Sensors
} // namespace OHOS
#endif // STREAM_SERVER_H
