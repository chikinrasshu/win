#include <chk/core/log.h>
#include <chk/win/win.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

/******************************************************************************/
/* Callbacks Fwd                                                              */
/******************************************************************************/

void chk_win_cb_on_error(S32 code, const char* msg);
void chk_win_cb_on_close(GLFWwindow* _h);
void chk_win_cb_on_refresh(GLFWwindow* _h);
void chk_win_cb_on_pos(GLFWwindow* _h, S32 x, S32 y);
void chk_win_cb_on_size(GLFWwindow* _h, S32 x, S32 y);
void chk_win_cb_on_fb_size(GLFWwindow* _h, S32 x, S32 y);
void chk_win_cb_on_dpi(GLFWwindow* _h, R32 x, R32 y);
void chk_win_cb_on_focus(GLFWwindow* _h, B32 v);
void chk_win_cb_on_hover(GLFWwindow* _h, B32 v);

/******************************************************************************/
/* WinConfig impl                                                             */
/******************************************************************************/

bool chk_win_config_get_default(WinConfig* c) {
    if (!c) {
        chk_warn("WinConfig", "c was NULL");
        return false;
    }

    // Data
    c->w          = 800;
    c->h          = 600;
    c->caption    = "chk_win";
    // Flags
    c->fullscreen = false;
    c->maximized  = false;
    c->minimized  = false;
    c->resizable  = true;
    c->bordered   = true;

    return true;
}

/******************************************************************************/
/* Win impl                                                                   */
/******************************************************************************/

static S32 g_win_count = 0;

bool chk_win_create(Win* w, WinConfig* c) {
    if (!w) {
        chk_warn("Win", "w was NULL");
        return false;
    }

    if (!g_win_count) {
        chk_info("Win", "Initializing Win subsystem");
        if (!glfwInit()) {
            chk_warn("Win", "Failed to initialize the Win subsystem");
            return false;
        }
    }
    ++g_win_count;

    glfwWindowHint(GLFW_RESIZABLE, c->resizable);
    glfwWindowHint(GLFW_DECORATED, c->bordered);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWmonitor* monitor = NULL;
    if (c->fullscreen) {
        // Get the monitor under the cursor
        S32           monitor_count;
        GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);

        monitor = glfwGetPrimaryMonitor();

        for (S32 monitor_i = 0; monitor_i < monitor_count; ++monitor_i) {
            //
        }
    }

    w->impl = glfwCreateWindow(c->w, c->h, c->caption, monitor, NULL);
    if (!w->impl) {
        chk_warn_f("Win", "Failed to create the Win '%s'", c->caption);
        return false;
    }

    glfwSetWindowUserPointer(w->impl, w);
    glfwSetWindowCloseCallback(w->impl, chk_win_cb_on_close);
    glfwSetWindowRefreshCallback(w->impl, chk_win_cb_on_refresh);
    glfwSetWindowPosCallback(w->impl, chk_win_cb_on_pos);
    glfwSetWindowSizeCallback(w->impl, chk_win_cb_on_size);
    glfwSetFramebufferSizeCallback(w->impl, chk_win_cb_on_fb_size);
    glfwSetWindowContentScaleCallback(w->impl, chk_win_cb_on_dpi);
    glfwSetWindowFocusCallback(w->impl, chk_win_cb_on_focus);
    glfwSetCursorEnterCallback(w->impl, chk_win_cb_on_hover);

    glfwGetWindowPos(w->impl, &w->data.x, &w->data.y);
    glfwGetWindowSize(w->impl, &w->data.w, &w->data.h);
    glfwGetFramebufferSize(w->impl, &w->data.fb_w, &w->data.fb_h);
    glfwGetWindowContentScale(w->impl, &w->data.dpi_x, &w->data.dpi_y);

    glfwShowWindow(w->impl);
    w->state.running = true;

    chk_info_f("Win", "Created Win '%s'", c->caption);

    return true;
}

bool chk_win_destroy(Win* w) {
    if (!w) {
        chk_warn("Win", "w was NULL");
        return false;
    }

    glfwDestroyWindow(w->impl);
    w->impl = NULL;

    *w = (Win){};

    --g_win_count;
    if (!g_win_count) {
        chk_info("Win", "Terminating the Win subsystem");
        glfwTerminate();
    }

    return true;
}

bool chk_win_run(Win* w) {
    if (!w) {
        chk_warn("Win", "w was NULL");
        return false;
    }

    while (w->state.running) {
        if (!chk_win_step(w, true)) {
            chk_warn("Win", "Failed to update the Win");
            break;
        }
    }

    return true;
}

bool chk_win_step(Win* w, bool process_events) {
    if (!w) {
        chk_warn("Win", "w was NULL");
        return false;
    }

    if (process_events) { glfwPollEvents(); }

    if (w->fn.on_update) { w->fn.on_update(w->data.dt, w->fn.user_ptr); }
    if (w->fn.on_render) { w->fn.on_render(w->fn.user_ptr); }
    if (w->fn.on_dbg_ui) { w->fn.on_dbg_ui(w->fn.user_ptr); }

    R64 ct     = glfwGetTime();
    w->data.dt = ct - w->data.et;
    w->data.et = ct;

    w->changed = (WinChanged){};

    return true;
}

/******************************************************************************/
/* Callbacks Impl                                                             */
/******************************************************************************/

void chk_win_cb_on_error(S32 code, const char* msg) {
    chk_info_f("Win", "[%d] %s", code, msg);
}

void chk_win_cb_on_close(GLFWwindow* _h) {
    Win* w = glfwGetWindowUserPointer(_h);
    if (!w) {
        chk_warn("Win", "w was NULL");
        return;
    }

    w->state.running = false;
}

void chk_win_cb_on_refresh(GLFWwindow* _h) {
    Win* w = glfwGetWindowUserPointer(_h);
    if (!w) {
        chk_warn("Win", "w was NULL");
        return;
    }

    chk_win_step(w, false);
}

void chk_win_cb_on_pos(GLFWwindow* _h, S32 x, S32 y) {
    Win* w = glfwGetWindowUserPointer(_h);
    if (!w) {
        chk_warn("Win", "w was NULL");
        return;
    }

    w->data.x = x, w->data.y = y;
    w->changed.pos = true;
}

void chk_win_cb_on_size(GLFWwindow* _h, S32 x, S32 y) {
    Win* w = glfwGetWindowUserPointer(_h);
    if (!w) {
        chk_warn("Win", "w was NULL");
        return;
    }

    w->data.w = x, w->data.h = y;
    w->changed.size = true;
}

void chk_win_cb_on_fb_size(GLFWwindow* _h, S32 x, S32 y) {
    Win* w = glfwGetWindowUserPointer(_h);
    if (!w) {
        chk_warn("Win", "w was NULL");
        return;
    }

    w->data.fb_w = x, w->data.fb_h = y;
    w->changed.fb = true;
}

void chk_win_cb_on_dpi(GLFWwindow* _h, R32 x, R32 y) {
    Win* w = glfwGetWindowUserPointer(_h);
    if (!w) {
        chk_warn("Win", "w was NULL");
        return;
    }

    w->data.dpi_x = x, w->data.dpi_y = y;
    w->changed.dpi = true;
}

void chk_win_cb_on_focus(GLFWwindow* _h, B32 v) {
    Win* w = glfwGetWindowUserPointer(_h);
    if (!w) {
        chk_warn("Win", "w was NULL");
        return;
    }

    w->state.focused = !!v;
    w->changed.focus = true;
}

void chk_win_cb_on_hover(GLFWwindow* _h, B32 v) {
    Win* w = glfwGetWindowUserPointer(_h);
    if (!w) {
        chk_warn("Win", "w was NULL");
        return;
    }

    w->state.hovered = !!v;
    w->changed.hover = true;
}
