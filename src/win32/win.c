#include <chk/core/log.h>
#include <chk/win/win32/win.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN        1
#define NOMINMAX            1
#define STRICT              1

#include <Windows.h>

#include <dwmapi.h>
#include <tchar.h>

#pragma comment(lib, "dwmapi.lib")

#undef near
#undef far

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

bool chk_win_win32_update_theme(HWND handle) {
    U8    data[4];
    DWORD count = sizeof(data) / sizeof(data[0]);

    LSTATUS status = RegGetValue(HKEY_CURRENT_USER,
                                 TEXT("Software\\Microsoft\\Windows\\CurrentVer"
                                      "sion\\Themes\\Personalize"),
                                 TEXT("AppsUseLightTheme"), RRF_RT_REG_DWORD,
                                 NULL, data, &count);
    if (status != ERROR_SUCCESS) { return false; }

    S32 i = (data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0]);

    BOOL    is_dark = (i == 0);
    HRESULT hr = DwmSetWindowAttribute(handle, DWMWA_USE_IMMERSIVE_DARK_MODE,
                                       &is_dark, sizeof(is_dark));
    if (hr != S_OK) {
        chk_warn_f("Win<Win32>", "Failed to set %s",
                   is_dark ? "dark mode" : "light mode");
        return false;
    }

    // Update the Win darkmode flag
    chk_info("Win<Win32>", "Updating theme...");
    Win* w = (Win*)GetWindowLongPtr(handle, GWLP_USERDATA);
    if (w) {
        w->state.dark   = is_dark;
        w->changed.dark = true;
    }

    return true;
}

CALLBACK LRESULT chk_win_win32_custom_proc(HWND handle, UINT msg, WPARAM wp,
                                           LPARAM lp) {
    void* user_data = (void*)GetWindowLongPtr(handle, GWLP_USERDATA);
    Win*  w         = user_data;

    switch (msg) {
        case WM_SETTINGCHANGE: {
            LPCTSTR key = (LPCTSTR)lp;

            if (key) {
                if (_tcscmp(key, _T("ImmersiveColorSet")) == 0) {
                    chk_win_win32_update_theme(handle);
                }
            }
        } break;
    }

    return CallWindowProcW((WNDPROC)w->impl_ex, handle, msg, wp, lp);
}

bool chk_win_win32_create_pre(Win* w, WinConfig* c) {
    if (!w) { return false; }

    // HINSTANCE instance = GetModuleHandle(NULL);

    return true;
}

bool chk_win_win32_create_post(Win* w, WinConfig* c) {
    if (!w) { return false; }
    if (!w->impl) { return false; }

    HWND handle = glfwGetWin32Window(w->impl);
    chk_win_win32_update_theme(handle);

    // Hook the GLFW wndproc with ours to run first
    w->impl_ex = (WNDPROC)GetWindowLongPtr(handle, GWLP_WNDPROC);
    SetWindowLongPtr(handle, GWLP_WNDPROC, (LONG_PTR)chk_win_win32_custom_proc);
    SetWindowLongPtr(handle, GWLP_USERDATA, (LONG_PTR)w);

    return true;
}

bool chk_win_win32_destroy(Win* w) {
    if (!w) { return false; }

    if (w->impl_ex) { w->impl_ex = NULL; }

    return true;
}
