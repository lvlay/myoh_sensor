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

use super::*;
use crate::error::SessionStatusCode;
use hilog_rust::{info, hilog, HiLogLabel, LogType};
const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002700,
    tag: "stream_session_ffi"
};
/// Create unique_ptr of StreamSession for C++ code
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// If uninitialized memory requires special handling, please refer to std::mem::MaybeUninit.
/// The pointer needs to be aligned for access. If the memory pointed to by the pointer is a compact
/// memory layout and requires special consideration. Please refer to (#[repr(packed)]).
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSessionCreate() -> *mut StreamSession {
    info!(LOG_LABEL, "enter StreamSessionCreate");
    let stream_session: Box::<StreamSession> = Box::default();
    Box::into_raw(stream_session)
}
/// Drop unique_ptr of StreamSession for C++ code
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// If uninitialized memory requires special handling, please refer to std::mem::MaybeUninit.
/// The pointer needs to be aligned for access. If the memory pointed to by the pointer is a compact
/// memory layout and requires special consideration. Please refer to (#[repr(packed)]).
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSessionDelete(raw: *mut StreamSession) {
    info!(LOG_LABEL, "enter StreamSessionDelete");
    if !raw.is_null() {
        drop(Box::from_raw(raw));
    }
}
/// Set StreamSession's uid
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSessionSetUid(object: *mut StreamSession, uid: i32) -> i32 {
    info!(LOG_LABEL, "enter StreamSessionSetUid");
    if let Some(obj) = StreamSession::as_mut(object) {
        obj.set_uid(uid);
        SessionStatusCode::Ok.into()
    } else {
        SessionStatusCode::SetUidFail.into()
    }
}
/// Set StreamSession's fd
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSessionSetFd(object: *mut StreamSession, fd: i32) -> i32 {
    info!(LOG_LABEL, "enter StreamSessionSetFd");
    if let Some(obj) = StreamSession::as_mut(object) {
        obj.set_fd(fd);
        SessionStatusCode::Ok.into()
    } else {
        SessionStatusCode::SetFdFail.into()
    }
}
/// Set StreamSession's pid
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSessionSetPid(object: *mut StreamSession, pid: i32) -> i32 {
    info!(LOG_LABEL, "enter StreamSessionSetPid");
    if let Some(obj) = StreamSession::as_mut(object) {
        obj.set_pid(pid);
        SessionStatusCode::Ok.into()
    } else {
        SessionStatusCode::SetPidFail.into()
    }
}
/// Obtain StreamSession's uid
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSessionGetUid(object: *const StreamSession) -> i32 {
    info!(LOG_LABEL, "enter StreamSessionGetUid");
    if let Some(obj) = StreamSession::as_ref(object) {
        obj.uid()
    } else {
        SessionStatusCode::UidFail.into()
    }
}
/// Obtain StreamSession's pid
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSessionGetPid(object: *const StreamSession) -> i32 {
    info!(LOG_LABEL, "enter StreamSessionGetPid");
    if let Some(obj) = StreamSession::as_ref(object) {
        obj.pid()
    } else {
        SessionStatusCode::PidFail.into()
    }
}

/// Obtain StreamSession's fd
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSessionGetFd(object: *const StreamSession) -> i32 {
    info!(LOG_LABEL, "enter StreamSessionGetFd");
    if let Some(obj) = StreamSession::as_ref(object) {
        obj.session_fd()
    } else {
        SessionStatusCode::FdFail.into()
    }
}
/// Set StreamSession's tokentype
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSessionSetTokenType(object: *mut StreamSession, style: i32) -> i32 {
    info!(LOG_LABEL, "enter StreamSessionSetTokenType");
    if let Some(obj) = StreamSession::as_mut(object) {
        obj.set_token_type(style);
        SessionStatusCode::Ok.into()
    } else {
        SessionStatusCode::SetTokenTypeFail.into()
    }
}
/// Obtain StreamSession's tokentype
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSessionGetTokenType(object: *const StreamSession) -> i32 {
    info!(LOG_LABEL, "enter StreamSessionGetTokenType");
    if let Some(obj) = StreamSession::as_ref(object) {
        obj.token_type()
    } else {
        SessionStatusCode::TokenTypeFail.into()
    }
}
/// Obtain StreamSession's moduletype
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSessionGetModuleType(object: *const StreamSession) -> i32 {
    info!(LOG_LABEL, "enter StreamSessionGetModuleType");
    if let Some(obj) = StreamSession::as_ref(object) {
        obj.module_type()
    } else {
        SessionStatusCode::ModuleTypeFail.into()
    }
}
/// Close StreamSession's fd
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSessionClose(object: *mut StreamSession) -> i32 {
    info!(LOG_LABEL, "enter StreamSessionClose");
    if let Some(obj) = StreamSession::as_mut(object) {
        obj.session_close();
        SessionStatusCode::Ok.into()
    } else {
        SessionStatusCode::CloseFail.into()
    }
}

/// Send message via StreamSessions
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSessionSendMsg(object: *const StreamSession, buf: *const c_char, size: usize) -> bool {
    info!(LOG_LABEL, "enter StreamSessionSendMsg");
    if let Some(obj) = StreamSession::as_ref(object) {
        obj.session_send_msg(buf, size)
    } else {
        false
    }
}