#include <stddef.h>
#include <stdio.h>

#include <cetech/celib/allocator.h>

#include <cetech/kernel/log.h>

#include <cetech/modules/luasys.h>
#include <cetech/kernel/api_system.h>
#include "../luasys_private.h"

#define API_NAME "Log"

CETECH_DECL_API(ct_log_a0);

static int _log_format(lua_State *l,
                       char *buffer,
                       size_t buffer_len) {
    char *buffer_it = buffer;
    size_t buffer_write_len = buffer_len;

    size_t frmt_len;
    const char *frmt = luasys_to_string_l(l, 2, &frmt_len);

    int arg_idx = 3;
    for (int i = 0; i < frmt_len; ++i) {
        if (frmt[i] != '%') {
            *buffer_it = frmt[i];
            ++buffer_it;
            buffer_write_len -= 1;
            continue;
        }

        ++i;
        int n = 0;
        switch (frmt[i]) {
            case 'f':
                n = snprintf(buffer_it, buffer_write_len, "%f",
                             luasys_to_float(l, arg_idx));
                buffer_it += n;
                ++arg_idx;
                break;

            case 'd':
                n = snprintf(buffer_it, buffer_write_len, "%d",
                             luasys_to_int(l, arg_idx));
                buffer_it += n;
                ++arg_idx;
                break;

            case 's':
                n = snprintf(buffer_it, buffer_write_len, "%s",
                             luasys_to_string(l, arg_idx));
                buffer_it += n;
                ++arg_idx;
                break;

            default:
                return 0;
        }
    }

    *buffer_it = '\0';

    return 1;
}

static int _log_info(lua_State *l) {
    char buffer[_4KiB]; // TODO: dynamic

    const char *where = luasys_to_string(l, 1);
    _log_format(l, buffer, _4KiB);

    ct_log_a0.info(where, "%s", buffer);
    return 0;
}

static int _log_warning(lua_State *l) {
    char buffer[_4KiB]; // TODO: dynamic

    const char *where = luasys_to_string(l, 1);
    _log_format(l, buffer, _4KiB);

    ct_log_a0.warning(where, "%s", buffer);
    return 0;
}

static int _log_error(lua_State *l) {
    char buffer[_4KiB]; // TODO: dynamic

    const char *where = luasys_to_string(l, 1);
    _log_format(l, buffer, _4KiB);

    ct_log_a0.error(where, "%s", buffer);
    return 0;
}

static int _log_debug(lua_State *l) {
    char buffer[_4KiB]; // TODO: dynamic

    const char *where = luasys_to_string(l, 1);
    _log_format(l, buffer, _4KiB);

    ct_log_a0.debug(where, "%s", buffer);
    return 0;
}

void _register_lua_log_api(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_log_a0);

    luasys_add_module_function(API_NAME, "info", _log_info);
    luasys_add_module_function(API_NAME, "warning", _log_warning);
    luasys_add_module_function(API_NAME, "error", _log_error);
    luasys_add_module_function(API_NAME, "debug", _log_debug);
}