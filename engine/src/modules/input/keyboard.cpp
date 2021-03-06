//==============================================================================
// Includes
//==============================================================================

#include <cetech/celib/allocator.h>
#include <cetech/kernel/config.h>
#include <cetech/celib/eventstream.inl>
#include <cetech/modules/input.h>
#include <cetech/kernel/os.h>
#include <cetech/kernel/api_system.h>

#include "keystr.h"
#include <cetech/kernel/log.h>
#include <cetech/kernel/errors.h>

CETECH_DECL_API(ct_machine_a0);
CETECH_DECL_API(ct_log_a0);

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "keyboard"


//==============================================================================
// Globals
//==============================================================================

static struct G {
    uint8_t state[512];
    uint8_t last_state[512];
} _G = {0};


//==============================================================================
// Interface
//==============================================================================

namespace keyboard {
    uint32_t button_index(const char *button_name) {
        for (uint32_t i = 0; i < KEY_MAX; ++i) {
            if (!_key_to_str[i]) {
                continue;
            }

            if (strcmp(_key_to_str[i], button_name)) {
                continue;
            }

            return i;
        }

        return 0;
    }

    const char *button_name(const uint32_t button_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (button_index >= 0) && (button_index < KEY_MAX));

        return _key_to_str[button_index];
    }

    int button_state(uint32_t idx,
                     const uint32_t button_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (button_index >= 0) && (button_index < KEY_MAX));

        return _G.state[button_index];
    }

    int button_pressed(uint32_t idx,
                       const uint32_t button_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (button_index >= 0) && (button_index < KEY_MAX));

        return _G.state[button_index] && !_G.last_state[button_index];
    }

    int button_released(uint32_t idx,
                        const uint32_t button_index) {
        CETECH_ASSERT(LOG_WHERE,
                      (button_index >= 0) && (button_index < KEY_MAX));

        return !_G.state[button_index] && _G.last_state[button_index];
    }

    void _update() {
        ct_event_header *event = ct_machine_a0.event_begin();

        memcpy(_G.last_state, _G.state, 512);

        uint32_t size = 0;
        while (event != ct_machine_a0.event_end()) {
            size = size + 1;

            switch (event->type) {
                case EVENT_KEYBOARD_DOWN:
                    _G.state[((ct_keyboard_event *) event)->keycode] = 1;
                    break;

                case EVENT_KEYBOARD_UP:
                    _G.state[((ct_keyboard_event *) event)->keycode] = 0;
                    break;

                default:
                    break;
            }

            event = ct_machine_a0.event_next(event);
        }
    }
}

namespace keyboard_module {
    static ct_keyboard_a0 a0 = {
            .button_index = keyboard::button_index,
            .button_name = keyboard::button_name,
            .button_state = keyboard::button_state,
            .button_pressed = keyboard::button_pressed,
            .button_released = keyboard::button_released,
            .update = keyboard::_update
    };

    void _init_api(ct_api_a0 *api) {
        api->register_api("ct_keyboard_a0", &a0);
    }

    void _init(ct_api_a0 *api) {
        _init_api(api);

        CETECH_GET_API(api, ct_machine_a0);
        CETECH_GET_API(api, ct_log_a0);

        _G = (struct G) {0};

        ct_log_a0.debug(LOG_WHERE, "Init");
    }

    void _shutdown() {
        ct_log_a0.debug(LOG_WHERE, "Shutdown");

        _G = (struct G) {0};
    }


    extern "C" void keyboard_unload_module(ct_api_a0 *api) {
        _shutdown();
    }


    extern "C" void keyboard_load_module(ct_api_a0 *api) {
        _init(api);
    }

}