#pragma once

#include <chk/core/types.h>

typedef struct WinConfig {
    S32         w, h;
    const char* caption;

    B32 fullscreen : 1;
    B32 maximized  : 1;
    B32 minimized  : 1;
    B32 resizable  : 1;
    B32 bordered   : 1;
} WinConfig;

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

typedef struct Win {
    WinData    data;
    WinState   state;
    WinChanged changed;

    void* impl;
    void* impl_ex;
} Win;

bool chk_win_create(Win* w, WinConfig c);
bool chk_win_destroy(Win* w);

bool chk_win_run(Win* w);
bool chk_win_step(Win* w, bool process_events);
