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
pub(super) mod net_packet;
mod binding;
use hilog_rust::{error, hilog, debug, HiLogLabel, LogType};
use std::ffi::{CString, c_char};
use std::mem::size_of;
use binding::CSensorServiceClient;
use net_packet::{NetPacket, CNetPacket, PackHead};
type ErrorStatus = crate::stream_buffer::ErrStatus;
/// function pointer alias
pub type ClientPacketCallBackFun = unsafe extern "C" fn (
    client: *const CSensorServiceClient,
    pkt: *const CNetPacket,
);

const ONCE_PROCESS_NETPACKET_LIMIT: i32 = 100;
const MAX_STREAM_BUF_SIZE: usize = 256;
/// max buffer size of packet
pub const MAX_PACKET_BUF_SIZE: usize = 256;
const PARAM_INPUT_INVALID: i32 = 5;
const MEM_OUT_OF_BOUNDS: i32 = 3;
const MEMCPY_SEC_FUN_FAIL: i32 = 4;
const STREAM_BUF_READ_FAIL: i32 = 1;
const MAX_VECTOR_SIZE: i32 = 10;
const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002700,
    tag: "StreamBuffer"
};

/// enum errstatus
#[derive(Copy, Clone, PartialEq)]
#[repr(C)]
pub enum ErrStatus {
    /// status ok
    Ok = 0,
    /// readerror
    Read = 1,
    /// writeerror
    Write = 2,
}

#[derive(Copy, Clone)]
#[repr(C)]
pub struct StreamBuffer {
    rw_error_status: ErrorStatus,
    r_count: usize,
    w_count: usize,
    r_pos: usize,
    w_pos: usize,
    sz_buff: [c_char; MAX_STREAM_BUF_SIZE + 1],
}

impl Default for StreamBuffer {
    fn default() -> Self {
        Self {
            rw_error_status: ErrorStatus::Ok,
            r_count: 0,
            w_count: 0,
            r_pos: 0,
            w_pos: 0,
            sz_buff: [0; MAX_STREAM_BUF_SIZE + 1],
        }
    }
}


impl StreamBuffer {
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
    fn write<T>(&mut self, data: T) {
        let data: *const c_char = &data as *const T as *const c_char;
        let size = size_of::<T>();
        self.write_char_usize(data, size);
    }
    fn reset(&mut self) {
        self.r_pos = 0;
        self.w_pos = 0;
        self.r_count = 0;
        self.w_count = 0;
        self.rw_error_status = ErrorStatus::Ok;
    }
    fn clean(&mut self) {
        self.reset();
        let size = MAX_STREAM_BUF_SIZE + 1;
        let reference = &(self.sz_buff);
        let pointer = reference as *const c_char;
        // SAFETY: memset_s is the security function of the C library
        let ret = unsafe {
            binding::memset_s(pointer as *mut libc::c_void, size, 0, size)
        };
        if ret != 0 {
            error!(LOG_LABEL, "Call memset_s fail");
        }
    }
    fn seek_read_pos(&mut self, n: usize) -> bool {
        let pos: usize = self.r_pos + n;
        if pos > self.w_pos {
            error!(LOG_LABEL, "The position in the calculation is not as expected. pos:{} [0, {}]",
                pos, self.w_pos);
            false
        } else {
            self.r_pos = pos;
            true
        }
    }
    fn unread_size(&self) -> usize {
        if self.w_pos <= self.r_pos {
            0
        } else {
            self.w_pos - self.r_pos
        }
    }
    fn is_empty(&self) -> bool {
        self.r_pos == self.w_pos
    }
    fn write_streambuffer(&mut self, buf: &Self) -> bool {
        self.write_char_usize(buf.data(), buf.size())
    }
    fn read_streambuffer(&self, buf: &mut Self) -> bool {
        buf.write_char_usize(self.data(), self.size())
    }
    pub(crate) fn data(&self) -> *const c_char {
        &(self.sz_buff[0]) as *const c_char
    }
    pub(crate) fn size(&self) -> usize {
        self.w_pos
    }
    pub(crate) fn chk_rwerror(&self) -> bool {
        self.rw_error_status != ErrorStatus::Ok
    }
    fn get_available_buf_size(&self) -> usize {
        if self.w_pos >= MAX_STREAM_BUF_SIZE {
            0
        } else {
            MAX_STREAM_BUF_SIZE - self.w_pos
        }
    }
    fn get_error_status_remark(&self) -> *const c_char {
        let s: &[c_char] = match self.rw_error_status {
            ErrorStatus::Ok => b"OK\0",
            ErrorStatus::Read => b"READ_ERROR\0",
            ErrorStatus::Write => b"WRITE_ERROR\0",
        };
        s.as_ptr()
    }
    fn read_buf(&self) -> *const c_char {
        &(self.sz_buff[self.r_pos]) as *const c_char
    }
    fn write_char_usize(&mut self, buf: *const c_char, size: usize) -> bool {
        if self.chk_rwerror() {
            return false;
        }
        if buf.is_null() {
            error!(LOG_LABEL, "Invalid input parameter buf=nullptr errCode:{}", PARAM_INPUT_INVALID);
            self.rw_error_status = ErrorStatus::Write;
            return false;
        }
        if size == 0 {
            error!(LOG_LABEL, "Invalid input parameter size={} errCode:{}", size, PARAM_INPUT_INVALID);
            self.rw_error_status = ErrorStatus::Write;
            return false;
        }
        if (self.w_pos + size) > MAX_STREAM_BUF_SIZE {
            error!(LOG_LABEL, "The write length exceeds buffer. wIdx:{} size:{} maxBufSize:{} errCode:{}",
                self.w_pos, size, MAX_STREAM_BUF_SIZE, MEM_OUT_OF_BOUNDS);
            self.rw_error_status = ErrorStatus::Write;
            return false;
        }
        let pointer = &(self.sz_buff[0]) as *const c_char;
        // SAFETY: memcpy_s is the security function of the C library
        let ret = unsafe {
            binding::memcpy_s(pointer.add(self.w_pos) as *mut libc::c_void, self.get_available_buf_size(),
            buf as *mut libc::c_void, size)
        };
        if ret != 0 {
            error!(LOG_LABEL, "Failed to call memcpy_s. ret:{}", ret);
            self.rw_error_status = ErrorStatus::Write;
            return false;
        }
        self.w_pos += size;
        self.w_count += 1;
        true
    }
    fn check_write(&mut self, size: usize) -> bool {
        let buffer_size = size;
        let mut avail_size = self.get_available_buf_size();
        if buffer_size > avail_size && self.r_pos > 0 {
            self.copy_data_to_begin();
            avail_size = self.get_available_buf_size();
        }
        avail_size >= buffer_size
    }
    fn copy_data_to_begin(&mut self) {
        let unread_size = self.unread_size();
        if unread_size > 0 && self.r_pos > 0 {
            for (index, value) in (self.r_pos..=self.w_pos).enumerate() {
                self.sz_buff[index] = self.sz_buff[value];
            }
        }
        debug!(LOG_LABEL, "unread_size:{} rPos:{} wPos:{}", unread_size, self.r_pos, self.w_pos);
        self.r_pos = 0;
        self.w_pos = unread_size;
    }
    fn read_char_usize(&mut self, buf: *const c_char, size: usize) -> bool {
        if self.chk_rwerror() {
            return false;
        }
        if buf.is_null() {
            error!(LOG_LABEL, "Invalid input parameter buf=nullptr errCode:{}", PARAM_INPUT_INVALID);
            self.rw_error_status = ErrorStatus::Read;
            return false;
        }
        if size == 0 {
            error!(LOG_LABEL, "Invalid input parameter size={} errCode:{}", size, PARAM_INPUT_INVALID);
            self.rw_error_status = ErrorStatus::Read;
            return false;
        }
        if (self.r_pos + size) > self.w_pos {
            error!(LOG_LABEL, "Memory out of bounds on read... errCode:{}", MEM_OUT_OF_BOUNDS);
            self.rw_error_status = ErrorStatus::Read;
            return false;
        }
        // SAFETY: memcpy_s is the security function of the C library
        let ret = unsafe {
            binding::memcpy_s(buf as *mut libc::c_void, size, self.read_buf() as *const libc::c_void, size)
        };
        if ret != 0 {
            error!(LOG_LABEL, "Failed to call memcpy_s. ret:{}", ret);
            self.rw_error_status = ErrorStatus::Read;
            return false;
        }
        self.r_pos += size;
        self.r_count += 1;
        true
    }
    fn circle_write(&mut self, buf: *const c_char, size: usize) -> bool {
        if !self.check_write(size) {
            error!(LOG_LABEL, "Out of buffer memory, availableSize:{}, size:{}, unreadSize:{}, rPos:{}, wPos:{}",
                self.get_available_buf_size(), size, self.unread_size(),
                self.r_pos, self.w_pos);
            return false;
        }
        self.write_char_usize(buf, size)
    }

    pub unsafe fn read_client_packets(&mut self, client: *const CSensorServiceClient, callback_fun: 
        ClientPacketCallBackFun) {
        const HEAD_SIZE: usize = size_of::<PackHead>();
        for _i in 0..ONCE_PROCESS_NETPACKET_LIMIT {
            let unread_size = self.unread_size();
            if unread_size < HEAD_SIZE {
                break;
            }
            let data_size = unread_size - HEAD_SIZE;
            let buf: *const c_char = self.read_buf();
            if buf.is_null() {
                error!(LOG_LABEL, "buf is null, skip then break");
                break;
            }
            let head: *const PackHead = buf as *const PackHead;
            if head.is_null() {
                error!(LOG_LABEL, "head is null, skip then break");
                break;
            }
            // SAFETY: head pointer should be not null certainly
            let size = unsafe {
                (*head).size
            };
            // SAFETY: head pointer should be not null certainly
            let id_msg = unsafe {
                (*head).id_msg
            };
            if !(0..=MAX_PACKET_BUF_SIZE).contains(&size) {
                error!(LOG_LABEL, "Packet header parsing error, and this error cannot be recovered. \
                    The buffer will be reset. size:{}, unreadSize:{}", size, unread_size);
                self.reset();
                break;
            }
            if size > data_size {
                break;
            }
            let mut pkt: NetPacket = NetPacket {
                msg_id: id_msg,
                ..Default::default()
            };
            unsafe {
                if size > 0 &&
                    !pkt.stream_buffer.write_char_usize(buf.add(HEAD_SIZE) as *const c_char, size) {
                    error!(LOG_LABEL, "Error writing data in the NetPacket. It will be retried next time. \
                        messageid:{}, size:{}", id_msg as i32, size);
                    break;
                }
            }
            if !self.seek_read_pos(pkt.get_packet_length()) {
                error!(LOG_LABEL, "Set read position error, and this error cannot be recovered, and the buffer \
                    will be reset. packetSize:{} unreadSize:{}", pkt.get_packet_length(), unread_size);
                self.reset();
                break;
            }
            let c_net_packet: CNetPacket = CNetPacket {
                msg_id: pkt.msg_id,
                stream_buffer_ptr: Box::into_raw(Box::new(pkt.stream_buffer))
            };
            unsafe {
                callback_fun(client, &c_net_packet as *const CNetPacket);
            }
            if self.is_empty() {
                self.reset();
                break;
            }
        }
    }
    fn r_count(&self) -> usize {
        self.r_count
    }
    fn w_count(&self) -> usize {
        self.w_count
    }
    fn w_pos(&self) -> usize {
        self.w_pos
    }
    fn r_pos(&self) -> usize {
        self.r_pos
    }
    fn sz_buff(&self) -> *const c_char {
        &self.sz_buff[0] as *const c_char
    }
    fn set_rw_error_status(&mut self, rw_error_status: ErrorStatus) {
        self.rw_error_status = rw_error_status
    }
    fn set_r_pos(&mut self, r_pos: usize) {
        self.r_pos = r_pos
    }
}

