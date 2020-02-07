mod bindings;

// use std::ffi::OsString;
// use std::fs::File;
// use std::io::prelude::*;
// use std::os::windows::prelude::*;

// use bindings::*;


/// Print the contents of the clipboard to filename
#[no_mangle]
pub extern "C" fn print_clipboard_file(filename: *const u16, len: usize) {
    let name_slice = unsafe { std::slice::from_raw_parts(filename, len) };
    let file_name = String::from_utf16_lossy(&name_slice);
    println!("{}", file_name);
}
