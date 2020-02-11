mod bindings;

// use std::ffi::OsString;
// use std::fs::File;
// use std::io::prelude::*;
// use std::os::windows::prelude::*;

// use bindings::*;

use winapi::um::{winbase, winuser};
use winapi::shared::{ntdef, minwindef};

unsafe fn str16_len(str16_ptr: *const u16) -> usize {
    let mut len: usize = 0;
    while (str16_ptr.add(len).read() != 0) {
        len += 1;
    }
    len
}

struct ClipboardGuard;

impl ClipboardGuard {
    pub fn new(handle: Option<*const core::ffi::c_void>) -> Result<Self, u32> {
        let call_status = match handle {
            None => unsafe { bindings::OpenClipboard(std::ptr::null()) },
            Some(inner_handle) => unsafe { bindings::OpenClipboard(inner_handle) }
        };

        match call_status {
            false => Err(unsafe{ bindings::GetLastError() }),
            true => Ok(Self{})
        }
    }

    pub fn get_text(&self) -> Result<String, u32> {
        match unsafe { winuser::GetClipboardData(winuser::CF_UNICODETEXT) } {
            ntdef::NULL => Err(unsafe{ bindings::GetLastError() }),
            data_handle => {
                match unsafe { winbase::GlobalLock(data_handle) } {
                    ntdef::NULL => Err(0),  // TODO: What should actually happen here?
                    clipboard_os_text => {
                        // Okay... so...
                        // clipboard_os_text is a pointer to a u16 string.
                        // We need to figure out how long it is if we want a String.
                        let str_len = unsafe { str16_len(clipboard_os_text as *const u16) };
                        let text_slice = unsafe { std::slice::from_raw_parts(clipboard_os_text as *const u16, str_len) };
                        Ok(String::from_utf16_lossy(&text_slice))
                    }
                }
            }
        }
    }
}

impl Drop for ClipboardGuard {
    fn drop(&mut self) {
        // println!("Closing Clipboard on Drop");
        unsafe { bindings::CloseClipboard() };
    }
}

/// Print the contents of the clipboard to filename
#[no_mangle]
pub extern "C" fn print_clipboard_file(filename: *const u16, len: usize) {
    let name_slice = unsafe { std::slice::from_raw_parts(filename, len) };
    let file_name = String::from_utf16_lossy(&name_slice);

    let my_clipboard = ClipboardGuard::new(None).unwrap();
    println!("{}", my_clipboard.get_text().unwrap());
}
