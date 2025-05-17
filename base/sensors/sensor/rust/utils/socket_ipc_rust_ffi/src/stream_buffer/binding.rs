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

 // C interface for socket core object
use libc::c_void;
/// C StreamServer struct pointer
#[repr(C)]
pub struct CStreamServer {
    _private: [u8; 0],
}
/// C Client struct pointer
#[repr(C)]
pub struct CSensorServiceClient {
    _private: [u8; 0],
}

extern "C" {
    /// extern safe C function
    pub fn memcpy_s(dest: *mut c_void, dest_size: libc::size_t, src: *const c_void, count: libc::size_t) -> i32;
    /// extern safe C function
    pub fn memset_s(dest: *mut c_void, dest_size: libc::size_t, ch: libc::c_int, count: libc::size_t) -> i32;
}

