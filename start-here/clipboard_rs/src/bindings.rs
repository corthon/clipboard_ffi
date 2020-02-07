extern "C" {
    pub fn OpenClipboard(handle: *const core::ffi::c_void) -> bool;
    pub fn CloseClipboard() -> bool;
    pub fn GetLastError() -> u32;
}