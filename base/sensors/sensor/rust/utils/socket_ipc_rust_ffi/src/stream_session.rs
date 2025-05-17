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

/// provide C interface to C++ for calling
pub mod ffi;
use hilog_rust::{debug, error, hilog, HiLogLabel, LogType};
use libc::c_int;
use std::{ffi::{CString, c_char}, thread::sleep, time::Duration};
const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002700,
    tag: "StreamSession"
};
const MAX_PACKET_BUF_SIZE: usize = 256;
const SEND_RETRY_LIMIT: i32 = 32;
const SEND_RETRY_SLEEP_TIME: u64 = 10000;
const RET_ERR: i32 = -1;

#[repr(C)]
pub struct StreamSession {
    module_type: i32,
    fd: i32,
    uid : i32,
    pid: i32,
    token_type: i32,
}

impl Default for StreamSession {
    fn default() -> Self {
        Self {
            module_type: RET_ERR,
            fd: RET_ERR,
            uid: RET_ERR,
            pid: RET_ERR,
            token_type: RET_ERR,
        }
    }
}

impl StreamSession {
    fn as_ref<'a>(object: *const Self) -> Option<&'a Self> {
        // SAFETY: as_ref has already done no-null verification inside
        unsafe {
            object.as_ref()
        }
    }
    fn as_mut<'a>(object: *mut Self) -> Option<&'a mut Self> {
        // SAFETY: as_mut has already done no-null verification inside
        unsafe {
            object.as_mut()
        }
    }
    fn uid(&self) -> i32 {
        self.uid
    }

    fn pid(&self) -> i32 {
        self.pid
    }

    fn module_type(&self) -> i32 {
        self.module_type
    }

    fn session_fd(&self) -> i32 {
        self.fd
    }

    fn set_token_type(&mut self, style: i32) {
        self.token_type = style
    }

    fn set_uid(&mut self, uid: i32) {
        self.uid = uid
    }

    fn set_pid(&mut self, pid: i32) {
        self.pid = pid
    }

    fn set_fd(&mut self, fd: i32) {
        self.fd = fd
    }

    fn token_type(&self) -> i32 {
        self.token_type
    }

    fn session_close(&mut self) {
        debug!(LOG_LABEL, "Enter fd_:{}.", self.fd);
        if self.fd >= 0 {
            // SAFETY: call unsafe function
            unsafe {
                libc::close(self.fd as c_int);
            }
            self.fd = RET_ERR;
        }
    }

    fn session_send_msg(&self, buf: *const c_char, size: usize) -> bool {
        if buf.is_null() {
            error!(LOG_LABEL, "buf is null");
            return false;
        }
        if size == 0 || size > MAX_PACKET_BUF_SIZE {
            error!(LOG_LABEL, "size is either equal to 0 or greater than MAX_PACKET_BUF_SIZE, size: {}", size);
            return false;
        }
        if self.fd < 0 {
            error!(LOG_LABEL, "The fd is less than 0, fd: {}", self.fd);
            return false;
        }
        let mut idx: usize = 0;
        let mut retry_count: i32 = 0;
        let buf_size = size;
        let mut rem_size = buf_size;
        while rem_size > 0 && retry_count < SEND_RETRY_LIMIT {
            retry_count += 1;
            // SAFETY: call extern libc library function
            let count = unsafe {
                libc::send(self.fd as c_int, buf.add(idx) as *const libc::c_void, rem_size,
                libc::MSG_DONTWAIT | libc::MSG_NOSIGNAL)
            };
            // SAFETY: call extern libc library function
            let errno = unsafe {
                *libc::__errno_location()
            };
            if count < 0 {
                if errno == libc::EAGAIN || errno == libc::EINTR || errno == libc::EWOULDBLOCK {
                    sleep(Duration::from_micros(SEND_RETRY_SLEEP_TIME));
                    error!(LOG_LABEL, "Continue for errno EAGAIN|EINTR|EWOULDBLOCK, errno:{}", errno);
                    continue;
                }
                error!(LOG_LABEL, "Send return failed,error:{} fd:{}", errno, self.fd);
                return false;
            }
            idx += count as usize;
            rem_size -= count as usize;
            if rem_size > 0 {
                sleep(Duration::from_micros(SEND_RETRY_SLEEP_TIME));
            }
        }
        if retry_count >= SEND_RETRY_LIMIT || rem_size != 0 {
            error!(LOG_LABEL, "Send too many times:{}/{},size:{}/{} fd:{}",
                retry_count, SEND_RETRY_LIMIT, idx, buf_size, self.fd);
            return false;
        }
        true
    }
}
