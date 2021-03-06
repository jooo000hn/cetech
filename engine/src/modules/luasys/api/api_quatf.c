
#include <cetech/celib/quatf.inl>
#include <cetech/modules/luasys.h>

#include <cetech/kernel/api_system.h>
#include "../luasys_private.h"

#define API_NAME "Quatf"

static int _ctor(lua_State *l) {
    float x = luasys_to_float(l, 1);
    float y = luasys_to_float(l, 2);
    float z = luasys_to_float(l, 3);
    float w = luasys_to_float(l, 4);

    luasys_push_quat(l, (quatf_t) {.x=x, .y=y, .z=z, .w=w});
    return 1;
}

static int _is(lua_State *l) {
    luasys_push_bool(l, _is_quat(l, 1));
    return 1;
}

static int _from_axis_angle(lua_State *l) {
    quatf_t result = {0};

    const vec3f_t *axis = luasys_to_vec3f(l, 1);
    float angle = luasys_to_float(l, 2);

    quatf_from_axis_angle(&result, axis, angle);

    luasys_push_quat(l, result);
    return 1;
}

static int _from_euler(lua_State *l) {
    quatf_t result = {0};

    const float heading = luasys_to_float(l, 1);
    const float attitude = luasys_to_float(l, 2);
    const float bank = luasys_to_float(l, 3);

    quatf_from_euler(&result, heading, attitude, bank);

    luasys_push_quat(l, result);
    return 1;
}

static int _to_mat44f(lua_State *l) {
    const quatf_t *q = luasys_to_quat(l, 1);
    mat44f_t result = {0};

    quatf_to_mat44f(&result, q);

    luasys_push_mat44f(l, result);
    return 1;
}

static int _to_euler_angle(lua_State *l) {
    const quatf_t *q = luasys_to_quat(l, 1);
    vec3f_t result = {0};

    quatf_to_eurel_angle(&result, q);

    luasys_push_vec3f(l, result);
    return 1;
}

static int _length(lua_State *l) {
    quatf_t *v = luasys_to_quat(l, 1);
    luasys_push_float(l, quatf_length(v));
    return 1;
}

static int _length_squared(lua_State *l) {
    quatf_t *v = luasys_to_quat(l, 1);
    luasys_push_float(l, quatf_length_squared(v));
    return 1;
}

static int _normalized(lua_State *l) {
    quatf_t *v = luasys_to_quat(l, 1);
    quatf_t res = {0};

    quatf_normalized(&res, v);

    luasys_push_quat(l, res);
    return 1;
}

void _register_lua_quatf_api(struct ct_api_a0 *api) {
    luasys_add_module_function(API_NAME, "make", _ctor);
    luasys_add_module_function(API_NAME, "is", _is);
    luasys_add_module_function(API_NAME, "from_axis_angle", _from_axis_angle);
    luasys_add_module_function(API_NAME, "from_euler", _from_euler);

    luasys_add_module_function(API_NAME, "to_mat44f", _to_mat44f);
    luasys_add_module_function(API_NAME, "to_euler_angle", _to_euler_angle);

    luasys_add_module_function(API_NAME, "length", _length);
    luasys_add_module_function(API_NAME, "length_squared", _length_squared);
    luasys_add_module_function(API_NAME, "normalized", _normalized);
}

