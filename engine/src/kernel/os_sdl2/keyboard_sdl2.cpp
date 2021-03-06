//==============================================================================
// Includes
//==============================================================================

#include <include/SDL2/SDL.h>

#include <cetech/celib/eventstream.inl>
#include <cetech/kernel/os.h>
#include <cetech/kernel/api_system.h>

using namespace cetech;

//==============================================================================
// Defines
//==============================================================================

#define is_button_down(now, last) ((now) && !(last))
#define is_button_up(now, last)   (!(now) && (last))


//==============================================================================
// Globals
//==============================================================================

static struct G {
    uint8_t state[KEY_MAX];
} _G = {0};


//==============================================================================
// Interface
//==============================================================================

int sdl_keyboard_init(ct_api_a0 *api) {
    _G = (struct G) {0};

    return 1;
}

void sdl_keyboard_shutdown() {
    _G = (struct G) {0};
}

void sdl_keyboard_process(EventStream &stream) {
    const uint8_t *state = SDL_GetKeyboardState(NULL);
    ct_keyboard_event keyboard_ev;

    for (uint32_t i = 0; i < KEY_MAX; ++i) {
        if (is_button_down(state[i], _G.state[i])) {
            keyboard_ev.keycode = i;
            eventstream::push(stream, EVENT_KEYBOARD_DOWN, keyboard_ev);

        } else if (is_button_up(state[i], _G.state[i])) {
            keyboard_ev.keycode = i;
            eventstream::push(stream, EVENT_KEYBOARD_UP, keyboard_ev);

        }

        _G.state[i] = state[i];
    }
}
