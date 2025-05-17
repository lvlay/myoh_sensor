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

use std::ffi::{CString, c_char};
use hilog_rust::{hilog, error, HiLogLabel, LogType};
use std::mem::size_of;
use crate::stream_buffer::StreamBuffer;
const STREAM_BUF_WRITE_FAIL: i32 = 2;
const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002700,
    tag: "NetPacket"
};

#[repr(packed(1))]
#[repr(C)]
pub(crate) struct PackHead {
    pub(crate) id_msg: MessageId,
    pub(crate) size: usize,
}

#[derive(Copy, Clone)]
#[repr(C)]
pub(crate) enum MessageId {
    Invalid = 0,
    Device,
    DeviceIds,
    DeviceSupportKeys,
    AddDeviceListener,
    DeviceKeyboardType,
    DisplayInfo,
    NoticeAnr,
    MarkProcess,
    OnSubscribeKey,
    OnKeyEvent,
    OnPointerEvent,
    ReportKeyEvent,
    ReportPointerEvent,
    OnDeviceAdded,
    OnDeviceRemoved,
    CoordinationAddListener,
    CoordinationMessage,
    CoordinationGetState,
    DragNotifyResult,
    DragStateListener,
}

#[derive(Copy, Clone)]
#[repr(C)]
pub(crate) struct NetPacket {
    pub(crate) msg_id: MessageId,
    pub(crate) stream_buffer: StreamBuffer,
}

#[derive(Copy, Clone)]
#[repr(C)]
pub struct CNetPacket {
    pub(super) msg_id: MessageId,
    pub(super) stream_buffer_ptr: *const StreamBuffer,
}

impl Default for NetPacket {
    fn default() -> Self {
        Self {
            msg_id: MessageId::Invalid,
            stream_buffer: Default::default(),
        }
    }
}

impl NetPacket {
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
    fn size(&self) -> usize {
        self.stream_buffer.size()
    }
    pub(super) fn get_packet_length(&self) -> usize {
        size_of::<PackHead>() + self.stream_buffer.w_pos
    }
    pub(crate) fn make_data(&self, buf: &mut StreamBuffer) {
        let head = PackHead {
            id_msg: self.msg_id,
            size: self.stream_buffer.w_pos,
        };
        buf.write(head);
        if self.stream_buffer.w_pos > 0 && !buf.write_char_usize(&self.stream_buffer.sz_buff[0] as *const c_char,
            self.stream_buffer.w_pos) {
            error!(LOG_LABEL, "Write data to stream failed, errCode:{}", STREAM_BUF_WRITE_FAIL);
        }
    }
}

