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
use hilog_rust::{info, hilog, HiLogLabel, LogType};
use crate::error::BufferStatusCode;
const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002700,
    tag: "stream_buffer_ffi"
};
/// Create unique_ptr of stream_buffer for C++ code
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// If uninitialized memory requires special handling, please refer to std::mem::MaybeUninit.
/// The pointer needs to be aligned for access. If the memory pointed to by the pointer is a compact
/// memory layout and requires special consideration. Please refer to (#[repr(packed)]).
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferCreate() -> *mut StreamBuffer {
    info!(LOG_LABEL, "enter StreamBufferCreate");
    let stream_buffer: Box::<StreamBuffer> = Box::default();
    Box::into_raw(stream_buffer)
}
/// Drop unique_ptr of stream_buffer for C++ code
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// If uninitialized memory requires special handling, please refer to std::mem::MaybeUninit.
/// The pointer needs to be aligned for access. If the memory pointed to by the pointer is a compact
/// memory layout and requires special consideration. Please refer to (#[repr(packed)]).
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferDelete(raw: *mut StreamBuffer) {
    info!(LOG_LABEL, "enter StreamBufferDelete");
    if !raw.is_null() {
        drop(Box::from_raw(raw));
    }
}
/// Obtain the first address of sz_buff
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferData(object: *const StreamBuffer) -> *const c_char {
    info!(LOG_LABEL, "enter data");
    if let Some(obj) = StreamBuffer::as_ref(object) {
        obj.data()
    } else {
        std::ptr::null()
    }
}
/// Obtain position writen
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferSize(object: *const StreamBuffer) -> usize {
    info!(LOG_LABEL, "enter size");
    if let Some(obj) = StreamBuffer::as_ref(object) {
        obj.size()
    } else {
        0
    }
}
/// Reset StreamBuffer value
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferReset(object: *mut StreamBuffer) -> i32 {
    info!(LOG_LABEL, "enter StreamBufferReset");
    if let Some(obj) = StreamBuffer::as_mut(object) {
        obj.reset();
        BufferStatusCode::Ok.into()
    } else {
        BufferStatusCode::ResetFail.into()
    }
}
/// Clean StreamBuffer value
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferClean(object: *mut StreamBuffer) -> i32 {
    info!(LOG_LABEL, "enter clean");
    if let Some(obj) = StreamBuffer::as_mut(object) {
        obj.clean();
        BufferStatusCode::Ok.into()
    } else {
        BufferStatusCode::CleanFail.into()
    }
}
/// Write object data into buf
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferWrite(object: *mut StreamBuffer, buf: *const StreamBuffer) -> bool {
    info!(LOG_LABEL, "enter StreamBufferWrite");
    if let Some(obj) = StreamBuffer::as_mut(object) {
        if let Some(buffer) = StreamBuffer::as_ref(buf) {
            obj.write_streambuffer(buffer)
        } else {
            false
        }
    } else {
        false
    }
}
/// Read object data into buf
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferRead(object: *const StreamBuffer, buf: *mut StreamBuffer) -> bool {
    info!(LOG_LABEL, "enter StreamBufferRead");
    if let Some(obj) = StreamBuffer::as_ref(object) {
        if let Some(buffer) = StreamBuffer::as_mut(buf) {
            obj.read_streambuffer(buffer)
        } else {
            false
        }
    } else {
        false
    }
}
/// Obtain status of reading or writing
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferChkRWError(object: *const StreamBuffer) -> bool {
    if let Some(obj) = StreamBuffer::as_ref(object) {
        obj.chk_rwerror()
    } else {
        false
    }
}
/// Obtain remarked string of status
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferGetErrorStatusRemark(object: *const StreamBuffer) -> *const c_char {
    info!(LOG_LABEL, "enter StreamBufferGetErrorStatusRemark");
    if let Some(obj) = StreamBuffer::as_ref(object) {
        obj.get_error_status_remark()
    } else {
        std::ptr::null()
    }
}
/// Buf Bytes will be writen into streambuffer's sz_buff.
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferWriteChar(object: *mut StreamBuffer, buf: *const c_char, size: usize) -> bool {
    info!(LOG_LABEL, "enter StreamBufferWriteChar");
    if let Some(obj) = StreamBuffer::as_mut(object) {
        obj.write_char_usize(buf, size)
    } else {
        false
    }
}
/// Check whether the condition of writing could be satisfied or not.
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferCheckWrite(object: *mut StreamBuffer, size: usize) -> bool {
    info!(LOG_LABEL, "enter StreamBufferCheckWrite");
    if let Some(obj) = StreamBuffer::as_mut(object) {
        obj.check_write(size)
    } else {
        false
    }
}
/// CircleStreamBuffer will copy data to beginning.
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn CircleStreamBufferCopyDataToBegin(object: *mut StreamBuffer) -> i32 {
    info!(LOG_LABEL, "enter CircleStreamBufferCopyDataToBegin");
    if let Some(obj) = StreamBuffer::as_mut(object) {
        obj.copy_data_to_begin();
        BufferStatusCode::Ok.into()
    } else {
        BufferStatusCode::CopyDataToBeginFail.into()
    }
}
/// Read sz_buf to buf.
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferReadChar(object: *mut StreamBuffer, buf: *const c_char, size: usize) -> bool {
    info!(LOG_LABEL, "enter StreamBufferReadChar");
    if let Some(obj) = StreamBuffer::as_mut(object) {
        obj.read_char_usize(buf, size)
    } else {
        false
    }
}
/// Write sz_buf to buf.
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn CircleStreamBufferWrite(object: *mut StreamBuffer, buf: *const c_char, size: usize) -> bool {
    info!(LOG_LABEL, "enter CircleStreamBufferWrite");
    if let Some(obj) = StreamBuffer::as_mut(object) {
        obj.circle_write(buf, size)
    } else {
        false
    }
}
/// read packets on client.
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn ReadClientPackets(object: *mut StreamBuffer, stream_client: *const CSensorServiceClient,
    callback_fun: ClientPacketCallBackFun) -> i32 {
    info!(LOG_LABEL,"enter ReadClientPackets");
    if let Some(obj) = StreamBuffer::as_mut(object) {
        obj.read_client_packets(stream_client, callback_fun);
        BufferStatusCode::Ok.into()
    } else {
        BufferStatusCode::ReadClientPacketsFail.into()
    }
}
/// StreamBufferReadBuf.
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferReadBuf(object: *const StreamBuffer) -> *const c_char {
    if let Some(obj) = StreamBuffer::as_ref(object) {
        obj.read_buf()
    } else {
        std::ptr::null()
    }
}
/// obtain streambuffer's r_count field.
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferGetRcount(object: *const StreamBuffer) -> i32 {
    if let Some(obj) = StreamBuffer::as_ref(object) {
        obj.r_count() as i32
    } else {
        BufferStatusCode::RcountFail.into()
    }
}
/// obtain streambuffer's w_count field.
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferGetWcount(object: *const StreamBuffer) -> i32 {
    if let Some(obj) = StreamBuffer::as_ref(object) {
        obj.w_count() as i32
    } else {
        BufferStatusCode::WcountFail.into()
    }
}
/// obtain streambuffer's w_pos field.
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferGetWpos(object: *const StreamBuffer) -> i32 {
    if let Some(obj) = StreamBuffer::as_ref(object) {
        obj.w_pos() as i32
    } else {
        BufferStatusCode::WposFail.into()
    }
}
/// obtain streambuffer's r_pos field.
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferGetRpos(object: *const StreamBuffer) -> i32 {
    if let Some(obj) = StreamBuffer::as_ref(object) {
        obj.r_pos() as i32
    } else {
        BufferStatusCode::RposFail.into()
    }
}
/// obtain streambuffer's sz_buff field.
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferGetSzBuff(object: *const StreamBuffer) -> *const c_char {
    if let Some(obj) = StreamBuffer::as_ref(object) {
        obj.sz_buff()
    } else {
        std::ptr::null()
    }
}
/// obtain streambuffer's rw_err_status field.
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferSetRwErrStatus(object: *mut StreamBuffer, rw_error_status: ErrorStatus) -> i32 {
    if let Some(obj) = StreamBuffer::as_mut(object) {
        obj.set_rw_error_status(rw_error_status);
        BufferStatusCode::Ok.into()
    } else {
        BufferStatusCode::SetRwErrStatusFail.into()
    }
}
/// set streambuffer's r_pos field.
///
/// # Safety
///
/// The pointer which pointed the memory already initialized must be valid.
/// Makesure the memory shouldn't be dropped while whose pointer is being used.
#[no_mangle]
pub unsafe extern "C" fn StreamBufferSetRpos(object: *mut StreamBuffer, r_pos: i32) -> i32 {
    if let Some(obj) = StreamBuffer::as_mut(object) {
        obj.set_r_pos(r_pos as usize);
        BufferStatusCode::Ok.into()
    } else {
        BufferStatusCode::SetRposFail.into()
    }
}

