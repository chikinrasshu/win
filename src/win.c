#include <chk/core/log.h>
#include <chk/win/win.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#if _WIN32
#include <chk/win/win32/win.h>
#endif

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
    c->size.w     = 800;
    c->size.h     = 600;
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

    w->state.uses_opengl = c->uses_opengl;

    glfwWindowHint(GLFW_RESIZABLE, c->resizable);
    glfwWindowHint(GLFW_DECORATED, c->bordered);
    if (w->state.uses_opengl) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    } else {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

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

#if _WIN32
    chk_win_win32_create_pre(w, c);
#endif

    w->impl = glfwCreateWindow(c->size.w, c->size.h, c->caption, monitor, NULL);
    if (!w->impl) {
        chk_warn_f("Win", "Failed to create the Win '%s'", c->caption);
        return false;
    }

#if _WIN32
    chk_win_win32_create_post(w, c);
#endif

    if (w->state.uses_opengl) {
        glfwMakeContextCurrent(w->impl);
        S32 version = gladLoadGL(glfwGetProcAddress);
        if (!version) {
            chk_warn("Win", "Failed to initialize OpenGL");
            return false;
        }

        chk_info_f("Win", "Initialized OpenGL %d.%d for Win '%s'",
                   GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version),
                   c->caption);
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

    glfwGetWindowPos(w->impl, &w->data.rect.x, &w->data.rect.y);
    glfwGetWindowSize(w->impl, &w->data.rect.w, &w->data.rect.h);
    glfwGetFramebufferSize(w->impl, &w->data.fb.w, &w->data.fb.h);
    glfwGetWindowContentScale(w->impl, &w->data.dpi.x, &w->data.dpi.y);

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

#if _WIN32
    chk_win_win32_destroy(w);
#endif

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

    if (w->state.uses_opengl) { glfwMakeContextCurrent(w->impl); }
    if (process_events) { glfwPollEvents(); }

    if (w->fn.on_update) { w->fn.on_update(w->data.dt, w->fn.user_ptr); }
    if (w->fn.on_render) { w->fn.on_render(w->fn.user_ptr); }
    if (w->fn.on_dbg_ui) { w->fn.on_dbg_ui(w->fn.user_ptr); }

    R64 ct     = glfwGetTime();
    w->data.dt = ct - w->data.et;
    w->data.et = ct;

    ++w->data.frame;

    w->changed = (WinChanged){};

    if (w->state.uses_opengl) { glfwSwapBuffers(w->impl); }

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

    w->data.rect.x = x, w->data.rect.y = y;
    w->changed.pos = true;
}

void chk_win_cb_on_size(GLFWwindow* _h, S32 x, S32 y) {
    Win* w = glfwGetWindowUserPointer(_h);
    if (!w) {
        chk_warn("Win", "w was NULL");
        return;
    }

    w->data.rect.w = x, w->data.rect.h = y;
    w->changed.size = true;
}

void chk_win_cb_on_fb_size(GLFWwindow* _h, S32 x, S32 y) {
    Win* w = glfwGetWindowUserPointer(_h);
    if (!w) {
        chk_warn("Win", "w was NULL");
        return;
    }

    w->data.fb.w = x, w->data.fb.h = y;
    w->changed.fb = true;

    if (w->state.uses_opengl) { glViewport(0, 0, x, y); }
}

void chk_win_cb_on_dpi(GLFWwindow* _h, R32 x, R32 y) {
    Win* w = glfwGetWindowUserPointer(_h);
    if (!w) {
        chk_warn("Win", "w was NULL");
        return;
    }

    w->data.dpi.x = x, w->data.dpi.y = y;
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
