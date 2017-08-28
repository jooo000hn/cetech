#ifndef CETECH_MACHINE_WINDOW_H
#define CETECH_MACHINE_WINDOW_H


#ifdef __cplusplus
extern "C" {
#endif


//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>
#include <stddef.h>

#include "_machine_enums.h"

//==============================================================================
// Window
//==============================================================================

enum ct_window_flags {
    WINDOW_NOFLAG = (1 << 0),
    WINDOW_FULLSCREEN = (1 << 1),
    WINDOW_SHOWN = (1 << 2),
    WINDOW_HIDDEN = (1 << 3),
    WINDOW_BORDERLESS = (1 << 4),
    WINDOW_RESIZABLE = (1 << 5),
    WINDOW_MINIMIZED = (1 << 6),
    WINDOW_MAXIMIZED = (1 << 7),
    WINDOW_INPUT_GRABBED = (1 << 8),
    WINDOW_INPUT_FOCUS = (1 << 9),
    WINDOW_MOUSE_FOCUS = (1 << 10),
    WINDOW_FULLSCREEN_DESKTOP = (1 << 11),
    WINDOW_ALLOW_HIGHDPI = (1 << 12),
    WINDOW_MOUSE_CAPTURE = (1 << 13),
    WINDOW_ALWAYS_ON_TOP = (1 << 14),
    WINDOW_SKIP_TASKBAR = (1 << 15),
    WINDOW_UTILITY = (1 << 16),
    WINDOW_TOOLTIP = (1 << 17),
    WINDOW_POPUP_MENU = (1 << 18),
};

enum ct_window_pos {
    WINDOWPOS_NOFLAG = (1 << 0),
    WINDOWPOS_CENTERED = (1 << 1),
    WINDOWPOS_UNDEFINED = (1 << 2),
};

typedef void ct_window_ints;

struct ct_window {
    ct_window_ints *inst;

    void (*set_title)(ct_window_ints *w,
                      const char *title);

    const char *(*get_title)(ct_window_ints *w);

    void (*update)(ct_window_ints *w);

    void (*resize)(ct_window_ints *w,
                   uint32_t width,
                   uint32_t height);

    void (*size)(ct_window_ints *window,
                 uint32_t *width,
                 uint32_t *height);

    void *(*native_window_ptr)(ct_window_ints *w);

    void *(*native_display_ptr)(ct_window_ints *w);
};


struct ct_window_a0 {
    ct_window *(*create)(struct cel_alloc *alloc,
                         const char *title,
                         enum ct_window_pos x,
                         enum ct_window_pos y,
                         const int32_t width,
                         const int32_t height,
                         uint32_t flags);

    ct_window *(*create_from)(struct cel_alloc *alloc,
                              void *hndl);

    void (*destroy)(struct cel_alloc *alloc,
                    ct_window *w);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_MACHINE_WINDOW_H