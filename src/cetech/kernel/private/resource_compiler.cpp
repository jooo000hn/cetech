#ifdef CETECH_CAN_COMPILE
//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

#include "include/SDL2/SDL.h"

#include <cetech/kernel/application.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/task.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/path.h>
#include <cetech/kernel/log.h>
#include <cetech/kernel/vio.h>
#include <cetech/kernel/hashlib.h>
#include <cetech/kernel/resource.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/watchdog.h>
#include <celib/map.inl>
#include <cetech/kernel/filesystem.h>

#include "celib/buffer.inl"


using namespace celib;
using namespace buffer;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_task_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_watchdog_a0);
CETECH_DECL_API(ct_app_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_yamlng_a0);

//==============================================================================
// Defines
//==============================================================================

#define MAX_TYPES 128
#define _G ResourceCompilerGlobal


//==============================================================================
// Globals
//==============================================================================

struct compilator {
    ct_resource_compilator_t compilator;
    ct_resource_compilator_yaml_t compilator_yaml;
};

struct compile_task_data {
    char *source_filename;
    uint64_t type;
    uint64_t name;
    ct_vio *source;
    ct_vio *build;
    time_t mtime;
    struct compilator compilator;
    atomic_int completed;
};

struct G {
    uint64_t compilator_map_type[MAX_TYPES]; // TODO: MAP
    compilator compilator_map_compilator[MAX_TYPES]; // TODO: MAP

    ct_watchdog *wd;
    ct_cvar cv_source_dir;
    ct_cvar cv_core_dir;
    ct_cvar cv_external_dir;
} ResourceCompilerGlobal = {};


#include "builddb.h"
#include "resource.h"

//CE_STATIC_ASSERT(sizeof(struct compile_task_data) < 64);


//==============================================================================
// Private
//==============================================================================

void type_name_from_filename(const char *fullname,
                             uint64_t *type,
                             uint64_t *name,
                             const char **short_name) {

    const char *filename_short = fullname;
    const char *resource_type = ct_path_a0.extension(filename_short);

    char resource_name[128] = {};

    size_t size = strlen(filename_short) - strlen(resource_type) - 1;

    memcpy(resource_name, filename_short, size);

    *type = ct_hash_a0.id64_from_str(resource_type);
    *name = ct_hash_a0.id64_from_str(resource_name);

    if (short_name) {
        *short_name = filename_short;
    }
}

void _add_dependency(const char *who_filename,
                     const char *depend_on_filename) {
    builddb_set_file_depend(who_filename, depend_on_filename);
    builddb_set_file(depend_on_filename,
                     ct_filesystem_a0.file_mtime(
                             ct_hash_a0.id64_from_str("source"),
                             depend_on_filename));
}

static ct_compilator_api _compilator_api = {
        .add_dependency = _add_dependency
};


static void _compile_task(void *data) {
    struct compile_task_data *tdata = (compile_task_data *) data;

    ct_log_a0.info("resource_compiler.task",
                   "Compile resource \"%s\" to \"" "%" SDL_PRIX64 "%" SDL_PRIX64 "\"",
                   tdata->source_filename, tdata->type, tdata->name);

    bool res = false;
    if (tdata->compilator.compilator) {
        res = tdata->compilator.compilator(tdata->source_filename,
                                           tdata->source, tdata->build,
                                           &_compilator_api) > 0;
    } else if (tdata->compilator.compilator_yaml) {
        ct_yamlng_document *d = ct_yamlng_a0.from_vio(tdata->source,
                                                      ct_memory_a0.main_allocator());
        res = tdata->compilator.compilator_yaml(tdata->source_filename, d,
                                                tdata->build,
                                                &_compilator_api) > 0;
        ct_yamlng_a0.destroy(d);
    }

    if (res) {
        builddb_set_file(tdata->source_filename, tdata->mtime);
        builddb_set_file_depend(tdata->source_filename, tdata->source_filename);

        ct_log_a0.info("resource_compiler.task",
                       "Resource \"%s\" compiled", tdata->source_filename);
    } else {
        ct_log_a0.error("resource_compiler.task",
                        "Resource \"%s\" compilation fail",
                        tdata->source_filename);
    }

    CEL_FREE(ct_memory_a0.main_scratch_allocator(),
             tdata->source_filename);

    tdata->source->close(tdata->source->inst);
    tdata->build->close(tdata->build->inst);

    atomic_store_explicit(&tdata->completed, 1, memory_order_release);
}

compilator _find_compilator(uint64_t type) {
    for (int i = 0; i < MAX_TYPES; ++i) {
        if (_G.compilator_map_type[i] != type) {
            continue;
        }

        return _G.compilator_map_compilator[i];
    }

    return {NULL};
}

void _compile_dir(Array<ct_task_item> &tasks,
                  char **files,
                  uint32_t files_count,
                  celib::Map<uint64_t> &compiled) {
    for (uint32_t i = 0; i < files_count; ++i) {
        const char *source_filename_short;

        uint64_t type_id;
        uint64_t name_id;

        type_name_from_filename(files[i], &type_id, &name_id,
                                &source_filename_short);

        compilator compilator = _find_compilator(type_id);
        if ((compilator.compilator == NULL) and
            (compilator.compilator_yaml == NULL)) {
            continue;
        }

        if (!builddb_need_compile(source_filename_short, &ct_filesystem_a0)) {
            continue;
        }

        char build_name[33] = {};
        snprintf(build_name, CETECH_ARRAY_LEN(build_name),
                 "%" SDL_PRIX64 "%" SDL_PRIX64, type_id, name_id);

        builddb_set_file_hash(source_filename_short, build_name);

        ct_vio *source_vio = ct_filesystem_a0.open(
                ct_hash_a0.id64_from_str("source"),
                source_filename_short,
                FS_OPEN_READ);

        if (source_vio == NULL) {
            continue;
        }

        auto platform = ct_config_a0.find("kernel.platform");
        char *build_full = ct_path_a0.join(
                ct_memory_a0.main_allocator(), 2,
                ct_config_a0.get_string(platform),
                build_name);

        ct_vio *build_vio = ct_filesystem_a0.open(
                ct_hash_a0.id64_from_str("build"),
                build_full,
                FS_OPEN_WRITE);

        CEL_FREE(ct_memory_a0.main_allocator(), build_full);

        if (build_vio == NULL) {
            continue;
        }

        struct compile_task_data *data =
                CEL_ALLOCATE(
                        ct_memory_a0.main_allocator(),
                        struct compile_task_data,
                        sizeof(struct compile_task_data));

        *data = (struct compile_task_data) {
                .name = name_id,
                .type = type_id,
                .build = build_vio,
                .source = source_vio,
                .compilator = compilator,
                .source_filename = ct_memory_a0.str_dup(source_filename_short,
                                                        ct_memory_a0.main_scratch_allocator()),
                .mtime = ct_filesystem_a0.file_mtime(
                        ct_hash_a0.id64_from_str("source"),
                        source_filename_short),

                .completed = 0
        };

        ct_task_item item = {
                .name = "compiler_task",
                .work = _compile_task,
                .data = data,
                .affinity = TASK_AFFINITY_NONE
        };

        array::push_back(tasks, item);
        multi_map::insert(compiled, type_id, name_id);
    }
}


//==============================================================================
// Interface
//==============================================================================


void resource_compiler_create_build_dir(struct ct_config_a0 config,
                                        struct ct_app_a0 app) {
    CEL_UNUSED(config, app);

    auto platform = ct_config_a0.find("kernel.platform");

    char *build_dir_full = resource_compiler_get_build_dir(
            ct_memory_a0.main_allocator(), ct_config_a0.get_string(platform));

    ct_path_a0.make_path(build_dir_full);

    CEL_FREE(ct_memory_a0.main_allocator(), build_dir_full);
}

void resource_compiler_register(uint64_t type,
                                ct_resource_compilator_t compilator) {
    for (int i = 0; i < MAX_TYPES; ++i) {
        if (_G.compilator_map_type[i] != 0) {
            continue;
        }

        _G.compilator_map_type[i] = type;
        _G.compilator_map_compilator[i] = {.compilator = compilator};
        return;
    }
}

void compiler_register_yaml(uint64_t type,
                            ct_resource_compilator_yaml_t compilator) {

    for (int i = 0; i < MAX_TYPES; ++i) {
        if (_G.compilator_map_type[i] != 0) {
            continue;
        }

        _G.compilator_map_type[i] = type;
        _G.compilator_map_compilator[i] = {.compilator_yaml = compilator};
        return;
    }
}

void _resource_compiler_compile_all(celib::Map<uint64_t> &compiled) {
    Array<ct_task_item> tasks(ct_memory_a0.main_allocator());
    const char *glob_patern = "**.*";
    char **files = nullptr;
    uint32_t files_count = 0;

    ct_filesystem_a0.listdir(ct_hash_a0.id64_from_str("source"),
                             "", glob_patern, false, true, &files, &files_count,
                             ct_memory_a0.main_scratch_allocator());

    _compile_dir(tasks, files, files_count, compiled);

    ct_filesystem_a0.listdir_free(files, files_count,
                                  ct_memory_a0.main_scratch_allocator());

    ct_task_a0.add(tasks._data, tasks._size);

    for (uint32_t i = 0; i < array::size(tasks); ++i) {
        compile_task_data *data = (compile_task_data *) tasks[i].data;

        ct_task_a0.wait_atomic(&data->completed, 0);
        CEL_FREE(ct_memory_a0.main_allocator(), data);
    }
}


void resource_compiler_compile_all() {
    Map<uint64_t> compieled(ct_memory_a0.main_allocator());
    _resource_compiler_compile_all(compieled);

}

int resource_compiler_get_filename(char *filename,
                                   size_t max_ken,
                                   uint64_t type,
                                   uint64_t name) {
    char build_name[33] = {};
    ct_resource_a0.type_name_string(build_name, CETECH_ARRAY_LEN(build_name),
                                    type,
                                    name);
    return builddb_get_filename_by_hash(filename, max_ken, build_name);
}

const char *resource_compiler_get_source_dir() {
    return ct_config_a0.get_string(_G.cv_source_dir);
}

const char *resource_compiler_get_core_dir() {
    return ct_config_a0.get_string(_G.cv_core_dir);
}

char *resource_compiler_get_tmp_dir(cel_alloc *alocator,
                                    const char *platform) {

    char *build_dir = resource_compiler_get_build_dir(alocator, platform);

    return ct_path_a0.join(alocator, 2, build_dir, "tmp");
}

char *resource_compiler_external_join(cel_alloc *alocator,
                                      const char *name) {
    const char *external_dir_str = ct_config_a0.get_string(_G.cv_external_dir);

    auto native_platform = ct_config_a0.find("kernel.native_platform");

    char *tmp_dir = ct_path_a0.join(alocator, 2, external_dir_str,
                                    ct_config_a0.get_string(native_platform));

    celib::Buffer buffer(alocator);
    buffer::printf(buffer, "%s64", tmp_dir);
    CEL_FREE(alocator, tmp_dir);

    buffer::c_str(buffer);
    return ct_path_a0.join(alocator, 4,
                           buffer::c_str(buffer), "release", "bin", name);
}


void _update(float dt) {
    auto *wd = _G.wd;

    _G.wd->fetch_events(_G.wd->inst);

    int need_compile = 0;
    auto *wd_it = wd->event_begin(wd->inst);
    const auto *wd_end = wd->event_end(wd->inst);

    while (wd_it != wd_end) {
        if (wd_it->type == CT_WATCHDOG_EVENT_FILE_WRITE_END) {
            need_compile = 1;
            break;
        }

        wd_it = wd->event_next(wd->inst, wd_it);
    }

    if (need_compile) {
        cel_alloc *alloc = ct_memory_a0.main_allocator();
        celib::Map<uint64_t> type_name(alloc);

        _resource_compiler_compile_all(type_name);

        auto *type_it = map::begin(type_name);
        auto *type_end = map::end(type_name);

        Array<uint64_t> name_array(ct_memory_a0.main_allocator());

        while (type_it != type_end) {
            uint64_t type_id = type_it->key;

            array::resize(name_array, 0);

            auto it = multi_map::find_first(type_name, type_id);
            while (it != nullptr) {
                array::push_back(name_array, it->value);

                it = multi_map::find_next(type_name, it);
            }


            ct_resource_a0.reload(type_id, &name_array[0],
                                  array::size(name_array));

            ++type_it;
        }
    }
}


static void _init_cvar(struct ct_config_a0 config) {
    ct_config_a0 = config;

    _G.cv_source_dir = config.new_str("src", "Resource source dir", "src");

    _G.cv_core_dir = config.new_str("core",
                                    "Resource application source dir",
                                    "core");
    _G.cv_external_dir = config.new_str("external", "External build dir",
                                        "externals/build");
}


static void _init(ct_api_a0 *api) {
    _init_cvar(ct_config_a0);

    ct_app_a0.register_on_update(_update);

    auto platform = ct_config_a0.find("kernel.platform");

    char *build_dir_full = ct_resource_a0.compiler_get_build_dir(
            ct_memory_a0.main_allocator(), ct_config_a0.get_string(platform));

    ct_path_a0.make_path(build_dir_full);
    builddb_init_db(build_dir_full, &ct_path_a0, &ct_memory_a0);

    char *tmp_dir_full = ct_path_a0.join(ct_memory_a0.main_allocator(), 2,
                                         build_dir_full, "tmp");

    ct_path_a0.make_path(tmp_dir_full);

    CEL_FREE(ct_memory_a0.main_allocator(), tmp_dir_full);
    CEL_FREE(ct_memory_a0.main_allocator(), build_dir_full);

    auto *wd = ct_watchdog_a0.create(ct_memory_a0.main_allocator());

    const char *core_dir = ct_config_a0.get_string(_G.cv_core_dir);
    const char *source_dir = ct_config_a0.get_string(_G.cv_source_dir);

    ct_filesystem_a0.map_root_dir(
            ct_hash_a0.id64_from_str("source"),
            core_dir
    );

    ct_filesystem_a0.map_root_dir(
            ct_hash_a0.id64_from_str("source"),
            source_dir
    );

    wd->add_dir(wd->inst, core_dir, true);
    wd->add_dir(wd->inst, source_dir, true);

    _G.wd = wd;
}

static void _shutdown() {
    ct_watchdog_a0.destroy(_G.wd);

    _G = {};
}


CETECH_MODULE_DEF(
        resourcecompiler,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_task_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_watchdog_a0);
            CETECH_GET_API(api, ct_app_a0);
            CETECH_GET_API(api, ct_filesystem_a0);
            CETECH_GET_API(api, ct_yamlng_a0);
        },
        {
            _init(api);
        },
        {
            CEL_UNUSED(api);

            _shutdown();

        }
)


#endif