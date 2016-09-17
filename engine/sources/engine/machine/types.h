#ifndef CETECH_MACHINE_TYPES_H
#define CETECH_MACHINE_TYPES_H

#include <stdio.h>

#include "celib/types.h"
#include "celib/math/types.h"
#include "celib/containers/eventstream.h"

//==============================================================================
// Base events
//==============================================================================

enum event {
    EVENT_INVALID = 0,
    EVENT_KEYBOARD_UP,
    EVENT_KEYBOARD_DOWN,

    EVENT_MOUSE_MOVE,
    EVENT_MOUSE_UP,
    EVENT_MOUSE_DOWN,

    EVENT_GAMEPAD_MOVE,
    EVENT_GAMEPAD_UP,
    EVENT_GAMEPAD_DOWN,
    EVENT_GAMEPAD_CONNECT,
    EVENT_GAMEPAD_DISCONNECT,
};

//==============================================================================
// Mouse events
//==============================================================================

struct mouse_event {
    struct event_header h;
    u32 button;
};

struct mouse_move_event {
    struct event_header h;
    vec2f_t pos;
};

enum {
    MOUSE_BTN_UNKNOWN = 0,

    MOUSE_BTN_LEFT = 1,
    MOUSE_BTN_MIDLE = 2,
    MOUSE_BTN_RIGHT = 3,

    MOUSE_BTN_MAX = 8
};

enum {
    MOUSE_AXIS_UNKNOWN = 0,

    MOUSE_AXIS_ABSOULTE = 1,
    MOUSE_AXIS_RELATIVE = 2,

    MOUSE_AXIS_MAX = 8
};

//==============================================================================
// Keyboarad events
//==============================================================================

#include "_key.h"

struct keyboard_event {
    struct event_header h;
    u32 keycode;
};

//==============================================================================
// Gamepad events
//==============================================================================

enum {
    GAMEPAD_UNKNOWN = 0,
    GAMEPAD_1 = 1,
    GAMEPAD_2 = 2,
    GAMEPAD_3 = 3,
    GAMEPAD_4 = 4,
    GAMEPAD_MAX = GAMEPAD_4,
};

enum {
    GAMEPAD_BTN_INVALID = 0,
    GAMEPAD_BTN_A,
    GAMEPAD_BTN_B,
    GAMEPAD_BTN_X,
    GAMEPAD_BTN_Y,
    GAMEPAD_BTN_BACK,
    GAMEPAD_BTN_GUIDE,
    GAMEPAD_BTN_START,
    GAMEPAD_BTN_LEFTSTICK,
    GAMEPAD_BTN_RIGHTSTICK,
    GAMEPAD_BTN_LEFTSHOULDER,
    GAMEPAD_BTN_RIGHTSHOULDER,
    GAMEPAD_BTN_DPADUP,
    GAMEPAD_BTN_DPADDOWN,
    GAMEPAD_BTN_DPADLEFT,
    GAMEPAD_BTN_DPADRIGHT,
    GAMEPAD_BTN_MAX
};

enum {
    GAMEPAD_AXIS_INVALID = 0,
    GAMEPAD_AXIS_LEFT = 1,
    GAMEPAD_AXIS_RIGHT = 2,
    GAMEPAD_AXIS_TRIGER = 3,
    GAMEPAD_AXIX_MAX
};

struct gamepad_move_event {
    struct event_header h;
    u8 gamepad_id;
    u32 axis;
    vec2f_t position;
};

struct gamepad_btn_event {
    struct event_header h;
    u8 gamepad_id;
    u32 button;
};

struct gamepad_device_event {
    struct event_header h;
    u8 gamepad_id;
};

#endif //CETECH_MACHINE_TYPES_H
