
#include <cetech/celib/allocator.h>
#include <cetech/modules/entity.h>

#include <cetech/modules/luasys.h>
#include <cetech/kernel/hash.h>
#include <cetech/kernel/api_system.h>

#include "cetech/modules/scenegraph.h"
#include "../luasys_private.h"

#define API_NAME "SceneGraph"

CETECH_DECL_API(ct_scenegprah_a0);
CETECH_DECL_API(ct_hash_a0);

static int _scenegraph_node_by_name(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_entity ent = {.h = luasys_to_handler(l, 2)};
    const char *name_str = luasys_to_string(l, 3);

    uint64_t name = ct_hash_a0.id64_from_str(name_str);
    struct ct_scene_node node = ct_scenegprah_a0.node_by_name(w, ent, name);

    luasys_push_int(l, node.idx);
    return 1;
}

static int _scenegraph_has(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_entity ent = {.h = luasys_to_handler(l, 2)};

    luasys_push_bool(l, ct_scenegprah_a0.has(w, ent));
    return 1;
}


static int _scenegraph_get_position(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_scene_node t = {.idx = luasys_to_int(l, 2)};

    luasys_push_vec3f(l, ct_scenegprah_a0.get_position(w, t));
    return 1;
}

static int _scenegraph_get_rotation(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_scene_node t = {.idx = luasys_to_int(l, 2)};

    luasys_push_quat(l, ct_scenegprah_a0.get_rotation(w, t));
    return 1;
}

static int _scenegraph_get_scale(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_scene_node t = {.idx = luasys_to_int(l, 2)};

    luasys_push_vec3f(l, ct_scenegprah_a0.get_scale(w, t));
    return 1;
}

static int _scenegraph_set_position(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_scene_node t = {.idx = luasys_to_int(l, 2)};
    vec3f_t *pos = luasys_to_vec3f(l, 3);

    ct_scenegprah_a0.set_position(w, t, *pos);
    return 0;
}

static int _scenegraph_set_scale(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_scene_node t = {.idx = luasys_to_int(l, 2)};
    vec3f_t *pos = luasys_to_vec3f(l, 3);

    ct_scenegprah_a0.set_scale(w, t, *pos);
    return 0;
}

static int _scenegraph_set_rotation(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_scene_node t = {.idx = luasys_to_int(l, 2)};
    quatf_t *rot = luasys_to_quat(l, 3);

    ct_scenegprah_a0.set_rotation(w, t, *rot);
    return 0;
}

static int _scenegraph_get_world_matrix(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_scene_node t = {.idx = luasys_to_int(l, 2)};

    mat44f_t *wm = ct_scenegprah_a0.get_world_matrix(w, t);

    luasys_push_mat44f(l, *wm);
    return 1;
}

static int _scenegraph_link(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_scene_node root = {.idx = luasys_to_int(l, 2)};
    struct ct_scene_node child = {.idx = luasys_to_int(l, 3)};

    ct_scenegprah_a0.link(w, root, child);
    return 0;
}

void _register_lua_scenegraph_api(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_scenegprah_a0);
    CETECH_GET_API(api, ct_hash_a0);

    luasys_add_module_function(API_NAME, "has", _scenegraph_has);
    luasys_add_module_function(API_NAME, "node_by_name",
                               _scenegraph_node_by_name);

    luasys_add_module_function(API_NAME, "get_position",
                               _scenegraph_get_position);
    luasys_add_module_function(API_NAME, "get_rotation",
                               _scenegraph_get_rotation);
    luasys_add_module_function(API_NAME, "get_scale", _scenegraph_get_scale);
    luasys_add_module_function(API_NAME, "get_world_matrix",
                               _scenegraph_get_world_matrix);

    luasys_add_module_function(API_NAME, "set_position",
                               _scenegraph_set_position);
    luasys_add_module_function(API_NAME, "set_rotation",
                               _scenegraph_set_rotation);
    luasys_add_module_function(API_NAME, "set_scale", _scenegraph_set_scale);
}