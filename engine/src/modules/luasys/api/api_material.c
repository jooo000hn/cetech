#include "cetech/modules/entity.h"
#include <cetech/celib/allocator.h>

#include "cetech/modules/transform.h"
#include "cetech/modules/renderer.h"
#include "../luasys_private.h"

#include <cetech/kernel/hash.h>
#include <cetech/kernel/api_system.h>

#define API_NAME "Material"

CETECH_DECL_API(ct_material_a0);
CETECH_DECL_API(ct_hash_a0);

static int _set_texture(lua_State *l) {
    struct ct_material m = {.idx = luasys_to_handler(l, 1)};
    const char *slot_name = luasys_to_string(l, 2);
    const char *texture_name = luasys_to_string(l, 3);

    ct_material_a0.set_texture(m, slot_name,
                               ct_hash_a0.id64_from_str(texture_name));
    return 0;
}


static int _set_vec4f(lua_State *l) {
    struct ct_material m = {.idx = luasys_to_handler(l, 1)};
    const char *slot_name = luasys_to_string(l, 2);
    vec4f_t *v = luasys_to_vec4f(l, 3);

    ct_material_a0.set_vec4f(m, slot_name, *v);

    return 0;
}

void _register_lua_material_api(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_material_a0);
    CETECH_GET_API(api, ct_hash_a0);

    luasys_add_module_function(API_NAME, "set_texture", _set_texture);
    luasys_add_module_function(API_NAME, "set_vec4f", _set_vec4f);
}