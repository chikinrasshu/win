#pragma once

#include <chk/win/win.h>

bool chk_win_win32_create_pre(Win* w, WinConfig* c);
bool chk_win_win32_create_post(Win* w, WinConfig* c);
bool chk_win_win32_destroy(Win* w);
