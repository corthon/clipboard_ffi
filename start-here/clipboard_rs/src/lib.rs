mod bindings;

// use std::ffi::OsString;
// use std::fs::File;
// use std::io::prelude::*;
// use std::os::windows::prelude::*;

// use bindings::*;

struct ClipboardGuard;

impl ClipboardGuard {
    pub fn new(handle: Option<*const core::ffi::c_void>) -> Result<Self, u32> {
        let call_status = match handle {
            None => unsafe { bindings::OpenClipboard(std::ptr::null()) },
            Some(inner_handle) => unsafe { bindings::OpenClipboard(inner_handle) }
        };

        match call_status {
            false => Err(unsafe{ bindings::GetLastError() }),
            true => Ok(ClipboardGuard{})
        }
    }
}

impl Drop for ClipboardGuard {
    fn drop(&mut self) {
        unsafe { bindings::CloseClipboard() };
    }
}

/// Print the contents of the clipboard to filename
#[no_mangle]
pub extern "C" fn print_clipboard_file(filename: *const u16, len: usize) {
    let name_slice = unsafe { std::slice::from_raw_parts(filename, len) };
    let file_name = String::from_utf16_lossy(&name_slice);
    println!("{}", file_name);
}
