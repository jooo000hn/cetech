#include <cetech/array.inl>
#include <cetech/yaml.h>
#include <cetech/quatf.h>
#include <cetech/mat44f.inl>

#include <cetech/application.h>
#include <cetech/config.h>
#include <cetech/vio.h>
#include <cetech/stringid.h>
#include <cetech/resource.h>

#include <cetech/entity.h>
#include <cetech/scenegraph.h>
#include <cetech/memory.h>
#include <cetech/module.h>
#include <cetech/map.inl>
#include <cetech/world.h>


ARRAY_PROTOTYPE(vec3f_t)

ARRAY_PROTOTYPE(stringid64_t)

ARRAY_PROTOTYPE(mat44f_t)

ARRAY_PROTOTYPE(quatf_t)

typedef struct {
    MAP_T(uint32_t) ent_idx_map;

    ARRAY_T(uint32_t) first_child;
    ARRAY_T(uint32_t) next_sibling;
    ARRAY_T(uint32_t) parent;

    ARRAY_T(stringid64_t) name;
    ARRAY_T(vec3f_t) position;
    ARRAY_T(quatf_t) rotation;
    ARRAY_T(vec3f_t) scale;
    ARRAY_T(mat44f_t) world_matrix;
} world_data_t;

ARRAY_PROTOTYPE(world_data_t)

MAP_PROTOTYPE(world_data_t)


#define _G SceneGraphGlobal
static struct G {
    MAP_T(world_data_t) world;
} _G = {0};

IMPORT_API(memory_api_v0);
IMPORT_API(world_api_v0);

static void _new_world(world_t world) {
    world_data_t data = {0};

    MAP_INIT(uint32_t, &data.ent_idx_map, memory_api_v0.main_allocator());

    ARRAY_INIT(uint32_t, &data.first_child, memory_api_v0.main_allocator());
    ARRAY_INIT(uint32_t, &data.next_sibling, memory_api_v0.main_allocator());
    ARRAY_INIT(uint32_t, &data.parent, memory_api_v0.main_allocator());

    ARRAY_INIT(stringid64_t, &data.name, memory_api_v0.main_allocator());
    ARRAY_INIT(vec3f_t, &data.position, memory_api_v0.main_allocator());
    ARRAY_INIT(quatf_t, &data.rotation, memory_api_v0.main_allocator());
    ARRAY_INIT(vec3f_t, &data.scale, memory_api_v0.main_allocator());
    ARRAY_INIT(mat44f_t, &data.world_matrix, memory_api_v0.main_allocator());

    MAP_SET(world_data_t, &_G.world, world.h, data);
}

static world_data_t *_get_world_data(world_t world) {
    return MAP_GET_PTR(world_data_t, &_G.world, world.h);
}

static void _destroy_world(world_t world) {
    world_data_t *data = _get_world_data(world);

    MAP_DESTROY(uint32_t, &data->ent_idx_map);

    ARRAY_DESTROY(uint32_t, &data->first_child);
    ARRAY_DESTROY(uint32_t, &data->next_sibling);
    ARRAY_DESTROY(uint32_t, &data->parent);

    ARRAY_DESTROY(stringid64_t, &data->name);
    ARRAY_DESTROY(vec3f_t, &data->position);
    ARRAY_DESTROY(quatf_t, &data->rotation);
    ARRAY_DESTROY(vec3f_t, &data->scale);
    ARRAY_DESTROY(mat44f_t, &data->world_matrix);
}

static void _on_world_create(world_t world) {
    _new_world(world);
}

static void _on_world_destroy(world_t world) {
    _destroy_world(world);
}


static void _init(get_api_fce_t get_engine_api) {
    INIT_API(get_engine_api, memory_api_v0, MEMORY_API_ID);
    INIT_API(get_engine_api, world_api_v0, WORLD_API_ID);

    _G = (struct G) {0};


    MAP_INIT(world_data_t, &_G.world, memory_api_v0.main_allocator());

    world_api_v0.register_callback(
            (world_callbacks_t) {.on_created=_on_world_create, .on_destroy=_on_world_destroy});

}

static void _shutdown() {

    MAP_DESTROY(world_data_t, &_G.world);

    _G = (struct G) {0};
}


int scenegraph_is_valid(scene_node_t transform) {
    return transform.idx != UINT32_MAX;
}

void scene_node_transform(world_t world,
                          scene_node_t transform,
                          mat44f_t *parent) {
    world_data_t *world_data = _get_world_data(world);

    vec3f_t pos = ARRAY_AT(&world_data->position, transform.idx);
    quatf_t rot = ARRAY_AT(&world_data->rotation, transform.idx);
    vec3f_t sca = ARRAY_AT(&world_data->scale, transform.idx);

    mat44f_t rm = {0};
    mat44f_t sm = {0};
    mat44f_t m = {0};

    quatf_to_mat44f(&rm, &rot);
    mat44f_scale(&sm, sca.x, sca.y, sca.z);
    mat44f_mul(&m, &sm, &rm);

    m.w.x = pos.x;
    m.w.y = pos.y;
    m.w.z = pos.z;

    mat44f_mul(&ARRAY_AT(&world_data->world_matrix, transform.idx), &m,
                   parent);

    uint32_t child = ARRAY_AT(&world_data->first_child, transform.idx);

    scene_node_t child_transform = {.idx = child};

    while (scenegraph_is_valid(child_transform)) {
        scene_node_transform(world, child_transform,
                             &ARRAY_AT(&world_data->world_matrix,
                                       transform.idx));
        child_transform.idx = ARRAY_AT(&world_data->next_sibling,
                                       child_transform.idx);
    }
}

vec3f_t scenegraph_get_position(world_t world,
                                    scene_node_t transform) {

    world_data_t *world_data = _get_world_data(world);
    return ARRAY_AT(&world_data->position, transform.idx);
}

quatf_t scenegraph_get_rotation(world_t world,
                                    scene_node_t transform) {

    world_data_t *world_data = _get_world_data(world);
    return ARRAY_AT(&world_data->rotation, transform.idx);
}

vec3f_t scenegraph_get_scale(world_t world,
                                 scene_node_t transform) {

    world_data_t *world_data = _get_world_data(world);
    return ARRAY_AT(&world_data->scale, transform.idx);
}

mat44f_t *scenegraph_get_world_matrix(world_t world,
                                          scene_node_t transform) {
    static mat44f_t _n = MAT44F_INIT_IDENTITY;
    world_data_t *world_data = _get_world_data(world);
    return &ARRAY_AT(&world_data->world_matrix, transform.idx);
}

void scenegraph_set_position(world_t world,
                             scene_node_t transform,
                             vec3f_t pos) {
    world_data_t *world_data = _get_world_data(world);
    uint32_t parent_idx = ARRAY_AT(&world_data->parent, transform.idx);

    scene_node_t pt = {.idx = parent_idx};

    mat44f_t m = MAT44F_INIT_IDENTITY;
    mat44f_t *p =
            parent_idx != UINT32_MAX ? scenegraph_get_world_matrix(world, pt)
                                     : &m;

    ARRAY_AT(&world_data->position, transform.idx) = pos;

    scene_node_transform(world, transform, p);
}

void scenegraph_set_rotation(world_t world,
                             scene_node_t transform,
                             quatf_t rot) {
    world_data_t *world_data = _get_world_data(world);
    uint32_t parent_idx = ARRAY_AT(&world_data->parent, transform.idx);

    scene_node_t pt = {.idx = parent_idx};

    mat44f_t m = MAT44F_INIT_IDENTITY;
    mat44f_t *p =
            parent_idx != UINT32_MAX ? scenegraph_get_world_matrix(world, pt)
                                     : &m;

    quatf_t nq = {0};
    quatf_normalized(&nq, &rot);

    ARRAY_AT(&world_data->rotation, transform.idx) = nq;

    scene_node_transform(world, transform, p);
}

void scenegraph_set_scale(world_t world,
                          scene_node_t transform,
                          vec3f_t scale) {
    world_data_t *world_data = _get_world_data(world);
    uint32_t parent_idx = ARRAY_AT(&world_data->parent, transform.idx);

    scene_node_t pt = {.idx = parent_idx};

    mat44f_t m = MAT44F_INIT_IDENTITY;
    mat44f_t *p =
            parent_idx != UINT32_MAX ? scenegraph_get_world_matrix(world, pt)
                                     : &m;

    ARRAY_AT(&world_data->scale, transform.idx) = scale;

    scene_node_transform(world, transform, p);
}

int scenegraph_has(world_t world,
                   entity_t entity) {
    world_data_t *world_data = _get_world_data(world);
    return MAP_HAS(uint32_t, &world_data->ent_idx_map, entity.h);
}

scene_node_t scenegraph_get_root(world_t world,
                                 entity_t entity) {

    world_data_t *world_data = _get_world_data(world);
    uint32_t idx = MAP_GET(uint32_t, &world_data->ent_idx_map, entity.h, UINT32_MAX);
    return (scene_node_t) {.idx = idx};
}

scene_node_t scenegraph_create(world_t world,
                               entity_t entity,
                               stringid64_t *names,
                               uint32_t *parent,
                               mat44f_t *pose,
                               uint32_t count) {
    world_data_t *data = _get_world_data(world);

    scene_node_t *nodes = CETECH_ALLOCATE(memory_api_v0.main_allocator(),
                                       scene_node_t, count);

    for (int i = 0; i < count; ++i) {
        uint32_t idx = (uint32_t) ARRAY_SIZE(&data->position);
        nodes[i] = (scene_node_t) {.idx = idx};

        mat44f_t local_pose = pose[i];

        vec3f_t position = {0};
        quatf_t rotation = QUATF_IDENTITY;
        vec3f_t scale = {.x = 1.0f, .y = 1.0f, .z = 1.0f};

        ARRAY_PUSH_BACK(vec3f_t, &data->position, position);
        ARRAY_PUSH_BACK(quatf_t, &data->rotation, rotation);
        ARRAY_PUSH_BACK(vec3f_t, &data->scale, scale);

        ARRAY_PUSH_BACK(stringid64_t, &data->name, names[i]);
        ARRAY_PUSH_BACK(uint32_t, &data->parent, UINT32_MAX);
        ARRAY_PUSH_BACK(uint32_t, &data->first_child, UINT32_MAX);
        ARRAY_PUSH_BACK(uint32_t, &data->next_sibling, UINT32_MAX);

        mat44f_t m = MAT44F_INIT_IDENTITY;
        ARRAY_PUSH_BACK(mat44f_t, &data->world_matrix, m);

        scene_node_t t = {.idx = idx};
        scene_node_transform(world, t,
                             parent[i] != UINT32_MAX
                             ? scenegraph_get_world_matrix(world,
                                                           nodes[parent[i]])
                             : &m);

        if (parent[i] != UINT32_MAX) {
            uint32_t parent_idx = nodes[parent[i]].idx;

            ARRAY_AT(&data->parent, idx) = parent_idx;

            if (ARRAY_AT(&data->first_child, parent_idx) == UINT32_MAX) {
                ARRAY_AT(&data->first_child, parent_idx) = idx;
            } else {
                uint32_t first_child_idx = ARRAY_AT(&data->first_child, parent_idx);
                ARRAY_AT(&data->first_child, parent_idx) = idx;
                ARRAY_AT(&data->next_sibling, idx) = first_child_idx;
            }

            ARRAY_AT(&data->parent, idx) = parent_idx;

        }
    }

    scene_node_t root = nodes[0];
    MAP_SET(uint32_t, &data->ent_idx_map, entity.h, root.idx);
    CETECH_DEALLOCATE(memory_api_v0.main_allocator(), nodes);
    return root;
}

void scenegraph_link(world_t world,
                     scene_node_t parent,
                     scene_node_t child) {
    world_data_t *data = _get_world_data(world);

    ARRAY_AT(&data->parent, child.idx) = parent.idx;

    uint32_t tmp = ARRAY_AT(&data->first_child, parent.idx);

    ARRAY_AT(&data->first_child, parent.idx) = child.idx;
    ARRAY_AT(&data->next_sibling, child.idx) = tmp;

    mat44f_t m = MAT44F_INIT_IDENTITY;

    mat44f_t *p =
            parent.idx != UINT32_MAX ? scenegraph_get_world_matrix(world,
                                                                   parent) : &m;

    scene_node_transform(world, parent, p);
    scene_node_transform(world, child,
                         scenegraph_get_world_matrix(world, parent));
}

scene_node_t _scenegraph_node_by_name(world_data_t *data,
                                      scene_node_t root,
                                      stringid64_t name) {
    if (ARRAY_AT(&data->name, root.idx).id == name.id) {
        return root;
    }

    scene_node_t node_it = {.idx = ARRAY_AT(&data->first_child, root.idx)};
    while (scenegraph_is_valid(node_it)) {
        scene_node_t ret = _scenegraph_node_by_name(data, node_it, name);
        if (ret.idx != UINT32_MAX) {
            return ret;
        }

        node_it.idx = ARRAY_AT(&data->next_sibling, node_it.idx);
    }

    return (scene_node_t) {.idx=UINT32_MAX};
}

scene_node_t scenegraph_node_by_name(world_t world,
                                     entity_t entity,
                                     stringid64_t name) {
    world_data_t *data = _get_world_data(world);
    scene_node_t root = scenegraph_get_root(world, entity);

    return _scenegraph_node_by_name(data, root, name);
}

void *scenegraph_get_module_api(int api) {

    switch (api) {
        case PLUGIN_EXPORT_API_ID:
                {
                    static struct module_api_v0 module = {0};

                    module.init = _init;
                    module.shutdown = _shutdown;

                    return &module;
                }


        case SCENEGRAPH_API_ID:
{
                    static struct scenegprah_api_v0 api = {0};

                    //api.scenegraph_transform = scenegraph_transform;
                    api.is_valid = scenegraph_is_valid;
                    api.get_position = scenegraph_get_position;
                    api.get_rotation = scenegraph_get_rotation;
                    api.get_scale = scenegraph_get_scale;
                    api.get_world_matrix = scenegraph_get_world_matrix;
                    api.set_position = scenegraph_set_position;
                    api.set_rotation = scenegraph_set_rotation;
                    api.set_scale = scenegraph_set_scale;
                    api.has = scenegraph_has;
                    api.get_root = scenegraph_get_root;
                    api.create = scenegraph_create;
                    api.link = scenegraph_link;
                    api.node_by_name = scenegraph_node_by_name;

                    return &api;
                }


        default:
            return NULL;
    }
}