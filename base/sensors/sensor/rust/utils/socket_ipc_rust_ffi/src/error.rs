/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

pub enum SocketStatusCode {
    Ok = 0,
    Fail = -1,
    FdFail = -2,
    EpollFdFail = -3,
    EpollCreateFail = -4,
    EpollCtlFail = -5,
    EpollWaitFail = -6,
    EpollCloseFail = -7,
    SocketCloseFail = -8,
    SocketSetFdFail = -9,
}

pub enum BufferStatusCode {
    Ok = 0,
    Fail = -1,
    ResetFail = -2,
    CleanFail = -3,
    UnreadSizeFail = -4,
    IsEmptyFail = -5,
    WriteStreamBufferFail = -6,
    ReadStreamBufferFail = -7,
    CheckRWErrorFail = -8,
    CopyDataToBeginFail = -9,
    ReadCharUsizeFail = -10,
    ReadServerPacketsFail = -11,
    ReadClientPacketsFail = -12,
    SizeFail = -13,
    RcountFail = -14,
    WcountFail = -15,
    WposFail = -16,
    RposFail = -17,
    SetRwErrStatusFail = -18,
    SetRposFail = -19,
}

pub enum SessionStatusCode {
    Ok = 0,
    Fail = -1,
    UidFail = -2,
    PidFail = -3,
    ModuleTypeFail = -4,
    FdFail = -5,
    SetTokenTypeFail = -6,
    TokenTypeFail = -7,
    CloseFail = -8,
    SetUidFail = -9,
    SetFdFail = -10,
    SetPidFail = -11,
}

pub enum NetPacketStatusCode {
    Ok = 0,
    Fail = -1,
    PacketLengthFail = -2,
}

impl From<SocketStatusCode> for i32 {
    fn from(code: SocketStatusCode) -> i32 {
        match code {
            SocketStatusCode::Ok => 0,
            SocketStatusCode::FdFail => -2,
            SocketStatusCode::EpollFdFail => -3,
            SocketStatusCode::EpollCreateFail => -4,
            SocketStatusCode::EpollCtlFail => -5,
            SocketStatusCode::EpollWaitFail => -6,
            SocketStatusCode::EpollCloseFail => -7,
            SocketStatusCode::SocketCloseFail => -8,
            SocketStatusCode::SocketSetFdFail => -9,
            _ => -1,
        }
    }
}

impl From<BufferStatusCode> for i32 {
    fn from(code: BufferStatusCode) -> i32 {
        match code {
            BufferStatusCode::Ok => 0,
            BufferStatusCode::ResetFail => -2,
            BufferStatusCode::CleanFail => -3,
            BufferStatusCode::UnreadSizeFail => -4,
            BufferStatusCode::IsEmptyFail => -5,
            BufferStatusCode::WriteStreamBufferFail => -6,
            BufferStatusCode::ReadStreamBufferFail => -7,
            BufferStatusCode::CheckRWErrorFail => -8,
            BufferStatusCode::CopyDataToBeginFail => -9,
            BufferStatusCode::ReadCharUsizeFail => -10,
            BufferStatusCode::ReadServerPacketsFail => -11,
            BufferStatusCode::ReadClientPacketsFail => -12,
            BufferStatusCode::SizeFail => -13,
            BufferStatusCode::RcountFail => -14,
            BufferStatusCode::WcountFail => -15,
            BufferStatusCode::WposFail => -16,
            BufferStatusCode::RposFail => -17,
            BufferStatusCode::SetRwErrStatusFail => -18,
            BufferStatusCode::SetRposFail => -19,
            _ => -1,
        }
    }
}

impl From<SessionStatusCode> for i32 {
    fn from(code: SessionStatusCode) -> i32 {
        match code {
            SessionStatusCode::Ok => 0,
            SessionStatusCode::UidFail => -2,
            SessionStatusCode::PidFail => -3,
            SessionStatusCode::ModuleTypeFail => -4,
            SessionStatusCode::FdFail => -5,
            SessionStatusCode::SetTokenTypeFail => -6,
            SessionStatusCode::TokenTypeFail => -7,
            SessionStatusCode::CloseFail => -8,
            SessionStatusCode::SetUidFail => -9,
            SessionStatusCode::SetFdFail => -10,
            SessionStatusCode::SetPidFail => -11,
            _ => -1,
        }
    }
}

impl From<NetPacketStatusCode> for i32 {
    fn from(code: NetPacketStatusCode) -> i32 {
        match code {
            NetPacketStatusCode::Ok => 0,
            NetPacketStatusCode::PacketLengthFail => -2,
            _ => -1,
        }
    }
}