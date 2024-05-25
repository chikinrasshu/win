#pragma once

#include <chk/core/types.h>

#define CHK_WIN_ON_UPDATE(name) void name(R64 dt, void* user_ptr)
#define CHK_WIN_ON_RENDER(name) void name(void* user_ptr)
#define CHK_WIN_ON_DBG_UI(name) void name(void* user_ptr)
typedef CHK_WIN_ON_UPDATE(WinFnOnUpdate);
typedef CHK_WIN_ON_RENDER(WinFnOnRender);
typedef CHK_WIN_ON_DBG_UI(WinFnOnDbgUI);

typedef struct WinConfig {
    // Data
    S32         w, h;
    const char* caption;
    // Flags
    B32         fullscreen : 1;
    B32         maximized  : 1;
    B32         minimized  : 1;
    B32         resizable  : 1;
    B32         bordered   : 1;
} WinConfig;

bool chk_win_config_get_default(WinConfig* c);

typedef struct WinData {
    S32 x, y;
    S32 w, h;
    S32 fb_w, fb_h;
    R32 dpi_x, dpi_y;
    R64 et, dt;
} WinData;

typedef struct WinState {
    B32 running    : 1;
    B32 fullscreen : 1;
    B32 resizable  : 1;
    B32 bordered   : 1;
    B32 focused    : 1;
    B32 hovered    : 1;
} WinState;

typedef struct WinChanged {
    // Data
    B32 pos        : 1;
    B32 size       : 1;
    B32 fb         : 1;
    B32 dpi        : 1;
    // State
    B32 fullscreen : 1;
    B32 focus      : 1;
    B32 hover      : 1;
} WinChanged;

typedef struct WinFn {
    void* user_ptr;

    WinFnOnUpdate* on_update;
    WinFnOnRender* on_render;
    WinFnOnDbgUI*  on_dbg_ui;
} WinFn;

typedef struct Win {
    WinData    data;
    WinState   state;
    WinChanged changed;
    WinFn      fn;

    void* impl;
    void* impl_ex;
} Win;

bool chk_win_create(Win* w, WinConfig c);
bool chk_win_destroy(Win* w);

bool chk_win_run(Win* w);
bool chk_win_step(Win* w, bool process_events);
