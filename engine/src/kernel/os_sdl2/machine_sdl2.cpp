//==============================================================================
// Includes
//==============================================================================

#include <cetech/celib/eventstream.inl>
#include <cetech/kernel/memory.h>

#include <cetech/kernel/config.h>
#include <cetech/modules/resource.h>
#include <cetech/kernel/api_system.h>

#include <cetech/kernel/log.h>
#include <cetech/modules/application.h>
#include <cetech/kernel/os.h>

#include <include/SDL2/SDL.h>

CETECH_DECL_API(ct_app_a0);
CETECH_DECL_API(ct_log_a0);

using namespace cetech;

//==============================================================================
// Extern functions
//==============================================================================
//==============================================================================
// Keyboard part
//==============================================================================

extern int sdl_keyboard_init(ct_api_a0 *api);

extern void sdl_keyboard_shutdown();

extern void sdl_keyboard_process(EventStream &stream);


//==============================================================================
// Mouse part
//==============================================================================

extern int sdl_mouse_init(ct_api_a0 *api);

extern void sdl_mouse_shutdown();

extern void sdl_mouse_process(EventStream &stream);

//==============================================================================
// Gamepad part
//==============================================================================

extern int sdl_gamepad_init(ct_api_a0 *api);

extern void sdl_gamepad_shutdown();

extern void sdl_gamepad_process(EventStream &stream);

extern void sdl_gamepad_process_event(SDL_Event *event,
                                      EventStream &stream);

int sdl_gamepad_is_active(int idx);

void sdl_gamepad_play_rumble(int gamepad,
                             float strength,
                             uint32_t length);

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "machine"


//==============================================================================
// Globals
//==============================================================================

#define _G MachineGlobals
static struct MachineGlobals {
    EventStream eventstream;
} MachineGlobals;

CETECH_DECL_API(ct_memory_a0);

//==============================================================================
// Interface
//==============================================================================
namespace machine_sdl {
    ct_event_header *machine_event_begin() {
        return (ct_event_header *) eventstream::begin(_G.eventstream);
    }


    ct_event_header *machine_event_end() {
        return (ct_event_header *) eventstream::end(_G.eventstream);
    }

    ct_event_header *machine_event_next(ct_event_header *header) {
        return (ct_event_header *) eventstream::next(_G.eventstream,
                                                     (eventstream::event_header *) header);
    }

    void _update() {
        eventstream::clear(_G.eventstream);
        SDL_Event e;

        while (SDL_PollEvent(&e) > 0) {
            switch (e.type) {
                case SDL_QUIT:
                    ct_app_a0.quit();
                    break;

                case SDL_CONTROLLERDEVICEADDED:
                case SDL_CONTROLLERDEVICEREMOVED:
                    sdl_gamepad_process_event(&e, _G.eventstream);
                    break;

                default:
                    sdl_gamepad_process(_G.eventstream);
                    sdl_mouse_process(_G.eventstream);
                    sdl_keyboard_process(_G.eventstream);
                    break;
            }
        }

    }

    static ct_machine_a0 a0 = {
            .event_begin = machine_sdl::machine_event_begin,
            .event_end = machine_sdl::machine_event_end,
            .event_next = machine_sdl::machine_event_next,
            .gamepad_is_active = sdl_gamepad_is_active,
            .gamepad_play_rumble = sdl_gamepad_play_rumble,
            .update = machine_sdl::_update,
    };

    void init(ct_api_a0 *api) {
        api->register_api("ct_machine_a0", &a0);

        CETECH_GET_API(api, ct_memory_a0);
        CETECH_GET_API(api, ct_app_a0);
        CETECH_GET_API(api, ct_log_a0);

        _G.eventstream.init(ct_memory_a0.main_allocator());

        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            ct_log_a0.error(LOG_WHERE, "Could not init sdl - %s",
                            SDL_GetError());
            return; // TODO: dksandasdnask FUCK init without return type?????
        }

        sdl_gamepad_init(api);
        sdl_mouse_init(api);
        sdl_keyboard_init(api);

    }

    void shutdown() {
        sdl_gamepad_shutdown();
        sdl_mouse_shutdown();
        sdl_keyboard_shutdown();

        SDL_Quit();

        _G.eventstream.destroy();
    }
}
