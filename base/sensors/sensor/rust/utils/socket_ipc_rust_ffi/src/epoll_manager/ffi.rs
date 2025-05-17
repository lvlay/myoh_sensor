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
use hilog_rust::{hilog, HiLogLabel, LogType};
use crate::{error::SocketStatusCode, epoll_manager::EpollManager};
use std::mem::drop;
use std::ffi::c_char;
const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002700,
    tag: "stream_socket_ffi"
};

/// Create unique_ptr of StreamSocket for C++ code
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// If uninitialized memory requires special handling, please refer to std::mem::MaybeUninit.
/// The pointer needs to be aligned for access. If the memory pointed to by the pointer is a compact
/// memory layout and requires special consideration. Please refer to (#[repr(packed)]).
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSocketCreate() -> *mut EpollManager {
    info!(LOG_LABEL, "enter StreamSocketCreate");
    let epoll_manager: Box::<EpollManager> = Box::default();
    Box::into_raw(epoll_manager)
}
/// Drop unique_ptr of StreamSocket for C++ code
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// If uninitialized memory requires special handling, please refer to std::mem::MaybeUninit.
/// The pointer needs to be aligned for access. If the memory pointed to by the pointer is a compact
/// memory layout and requires special consideration. Please refer to (#[repr(packed)]).
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSocketDelete(raw: *mut EpollManager) {
    info!(LOG_LABEL, "enter StreamSocketDelete");
    if !raw.is_null() {
        drop(Box::from_raw(raw));
    }
}
/// Obtain StreamSocket's fd
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSocketGetFd(object: *const EpollManager) -> i32 {
    info!(LOG_LABEL, "enter StreamSocketGetFd");
    if let Some(obj) = EpollManager::as_ref(object) {
        obj.socket_fd()
    } else {
        SocketStatusCode::FdFail.into()
    }
}
/// Close socket fd after Sending data.
///
/// # Safety
/// 
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSocketClose(object: *mut EpollManager) -> i32 {
    info!(LOG_LABEL, "enter StreamSocketClose");
    if let Some(obj) = EpollManager::as_mut(object) {
        obj.socket_close();
        SocketStatusCode::Ok.into()
    } else {
        SocketStatusCode::SocketCloseFail.into()
    }
}
/// Set socket fd
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamSocketSetFd(object: *mut EpollManager, fd: i32) -> i32 {
    info!(LOG_LABEL, "enter StreamSocketSetFd");
    if let Some(obj) = EpollManager::as_mut(object) {
        obj.socket_set_fd(fd);
        SocketStatusCode::Ok.into()
    } else {
        SocketStatusCode::SocketSetFdFail.into()
    }
}
