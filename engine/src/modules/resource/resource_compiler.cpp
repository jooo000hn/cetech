#ifdef CETECH_CAN_COMPILE
//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

#include "include/SDL2/SDL.h"

#include <cetech/celib/array.inl>
#include <cetech/kernel/os.h>
#include <cetech/kernel/hash.h>
#include <cetech/kernel/task.h>
#include <cetech/modules/application.h>
#include <cetech/kernel/config.h>
#include <cetech/modules/resource.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/log.h>
#include <cetech/kernel/api_system.h>

#include <cetech/celib/string_stream.h>


using namespace cetech;
using namespace string_stream;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_task_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_app_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);

//==============================================================================
// Defines
//==============================================================================

#define MAX_TYPES 128
#define _G ResourceCompilerGlobal


//==============================================================================
// Globals
//==============================================================================

struct compile_task_data {
    char *source_filename;
    uint64_t type;
    uint64_t name;
    ct_vio *source;
    ct_vio *build;
    time_t mtime;
    ct_resource_compilator_t compilator;
    atomic_int completed;
};

struct G {
    uint64_t compilator_map_type[MAX_TYPES]; // TODO: MAP
    ct_resource_compilator_t compilator_map_compilator[MAX_TYPES]; // TODO: MAP

    ct_cvar cv_source_dir;
    ct_cvar cv_core_dir;
    ct_cvar cv_external_dir;
} ResourceCompilerGlobal = {0};


#include "builddb.h"
#include "resource.h"

//CE_STATIC_ASSERT(sizeof(struct compile_task_data) < 64);


//==============================================================================
// Private
//==============================================================================

void _add_dependency(const char *who_filename,
                     const char *depend_on_filename) {
    auto a = ct_memory_a0.main_allocator();

    builddb_set_file_depend(who_filename, depend_on_filename);

    char *path = ct_path_a0.join(a, 2,
                                 ct_resource_a0.compiler_get_source_dir(),
                                 depend_on_filename);

    builddb_set_file(depend_on_filename, ct_path_a0.file_mtime(path));

    CETECH_FREE(a, path);
}

static ct_compilator_api _compilator_api = {
        .add_dependency = _add_dependency
};


static void _compile_task(void *data) {
    struct compile_task_data *tdata = (compile_task_data *) data;

    ct_log_a0.info("resource_compiler.task",
                   "Compile resource \"%s\" to \"" "%" SDL_PRIX64 "%" SDL_PRIX64 "\"",
                   tdata->source_filename, tdata->type, tdata->name);


    if (tdata->compilator(tdata->source_filename, tdata->source, tdata->build,
                          &_compilator_api)) {
        builddb_set_file(tdata->source_filename, tdata->mtime);
        builddb_set_file_depend(tdata->source_filename, tdata->source_filename);

        ct_log_a0.info("resource_compiler.task",
                       "Resource \"%s\" compiled", tdata->source_filename);
    } else {
        ct_log_a0.error("resource_compiler.task",
                        "Resource \"%s\" compilation fail",
                        tdata->source_filename);
    }

    CETECH_FREE(ct_memory_a0.main_scratch_allocator(),
                tdata->source_filename);

    tdata->source->close(tdata->source->inst);
    tdata->build->close(tdata->build->inst);

    atomic_store_explicit(&tdata->completed, 1, memory_order_release);
}

ct_resource_compilator_t _find_compilator(uint64_t type) {
    for (int i = 0; i < MAX_TYPES; ++i) {
        if (_G.compilator_map_type[i] != type) {
            continue;
        }

        return _G.compilator_map_compilator[i];
    }

    return NULL;
}

void _compile_dir(Array<ct_task_item> &tasks,
                  const char *source_dir,
                  const char *build_dir_full) {

    auto a = ct_memory_a0.main_allocator();

    char **files = nullptr;
    uint32_t files_count = 0;

    ct_path_a0.list(source_dir, 1, &files, &files_count,
                    ct_memory_a0.main_scratch_allocator());

    for (int i = 0; i < files_count; ++i) {
        const char *source_filename_full = files[i];
        const char *source_filename_short = files[i] + strlen(source_dir) + 1;
        const char *resource_type = ct_path_a0.extension(
                source_filename_short);

        char resource_name[128] = {0};
        memcpy(resource_name, source_filename_short,
               strlen(source_filename_short) - 1 -
               strlen(resource_type));

        uint64_t type_id = ct_hash_a0.id64_from_str(resource_type);
        uint64_t name_id = ct_hash_a0.id64_from_str(resource_name);

        ct_resource_compilator_t compilator = _find_compilator(type_id);
        if (compilator == NULL) {
            //log_warning("resource_compilator", "Type \"%s\" does not register compilator", resource_type);
            continue;
        }

        if (!builddb_need_compile(source_dir, source_filename_short,
                                  &ct_path_a0)) {
            continue;
        }

        char build_name[33] = {0};
        snprintf(build_name, CETECH_ARRAY_LEN(build_name),
                 "%" SDL_PRIX64 "%" SDL_PRIX64, type_id, name_id);

        builddb_set_file_hash(source_filename_short, build_name);

        ct_vio *source_vio = ct_vio_a0.from_file(
                source_filename_full,
                VIO_OPEN_READ);

        if (source_vio == NULL) {

            continue;
        }

        char *build_path = ct_path_a0.join(a, 2, build_dir_full, build_name);

        ct_vio *build_vio = ct_vio_a0.from_file(build_path,
                                                VIO_OPEN_WRITE);

        CETECH_FREE(a, build_path);

        if (build_vio == NULL) {
            continue;
        }

        struct compile_task_data *data =
                CETECH_ALLOCATE(
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
                .mtime = ct_path_a0.file_mtime(source_filename_full),
                .completed = 0
        };

        ct_task_item item = {
                .name = "compiler_task",
                .work = _compile_task,
                .data = data,
                .affinity = TASK_AFFINITY_NONE
        };

        array::push_back(tasks, item);
    }

    ct_path_a0.list_free(files, files_count,
                         ct_memory_a0.main_scratch_allocator());
}


//==============================================================================
// Interface
//==============================================================================

static void _init_cvar(struct ct_config_a0 config) {
    ct_config_a0 = config;

    _G.cv_source_dir = config.new_str("src", "Resource source dir", "data/src");
    _G.cv_core_dir = config.new_str("core", "Resource application source dir",
                                    "core");
    _G.cv_external_dir = config.new_str("external", "External build dir",
                                        "externals/build");
}

static void _init(ct_api_a0 *api) {
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_resource_a0);
    CETECH_GET_API(api, ct_task_a0);
    CETECH_GET_API(api, ct_app_a0);
    CETECH_GET_API(api, ct_path_a0);
    CETECH_GET_API(api, ct_vio_a0);
    CETECH_GET_API(api, ct_log_a0);
    CETECH_GET_API(api, ct_hash_a0);
    CETECH_GET_API(api, ct_config_a0);

    _init_cvar(ct_config_a0);

    char *build_dir_full = ct_resource_a0.compiler_get_build_dir(
            ct_memory_a0.main_allocator(), ct_app_a0.platform());

    ct_path_a0.make_path(build_dir_full);
    builddb_init_db(build_dir_full, &ct_path_a0, &ct_memory_a0);

    char *tmp_dir_full = ct_path_a0.join(ct_memory_a0.main_allocator(), 2,
                                         build_dir_full, "tmp");

    ct_path_a0.make_path(tmp_dir_full);

    CETECH_FREE(ct_memory_a0.main_allocator(), tmp_dir_full);
    CETECH_FREE(ct_memory_a0.main_allocator(), build_dir_full);
}

static void _shutdown() {
    _G = (struct G) {0};
}


void resource_compiler_create_build_dir(struct ct_config_a0 config,
                                        struct ct_app_a0 app) {

    const char *platform = ct_app_a0.platform();
    char *build_dir_full = resource_compiler_get_build_dir(
            ct_memory_a0.main_allocator(), platform);

    ct_path_a0.make_path(build_dir_full);

    CETECH_FREE(ct_memory_a0.main_allocator(), build_dir_full);
}

void resource_compiler_register(uint64_t type,
                                ct_resource_compilator_t compilator) {
    for (int i = 0; i < MAX_TYPES; ++i) {
        if (_G.compilator_map_type[i] != 0) {
            continue;
        }

        _G.compilator_map_type[i] = type;
        _G.compilator_map_compilator[i] = compilator;
        return;
    }
}


void resource_compiler_compile_all() {
    const char *core_dir = ct_config_a0.get_string(_G.cv_core_dir);
    const char *source_dir = ct_config_a0.get_string(_G.cv_source_dir);

    char *build_dir_full = ct_resource_a0.compiler_get_build_dir(
            ct_memory_a0.main_allocator(),
            ct_app_a0.platform()
    );

    Array<ct_task_item> tasks(ct_memory_a0.main_allocator());

    const char *dirs[] = {source_dir, core_dir};
    for (int i = 0; i < CETECH_ARRAY_LEN(dirs); ++i) {
        _compile_dir(tasks, dirs[i], build_dir_full);
    }

    ct_task_a0.add(tasks._data, tasks._size);

    for (int i = 0; i < array::size(tasks); ++i) {
        compile_task_data *data = (compile_task_data *) tasks[i].data;

        ct_task_a0.wait_atomic(&data->completed, 0);
        CETECH_FREE(ct_memory_a0.main_allocator(), data);
    }

    CETECH_FREE(ct_memory_a0.main_allocator(), build_dir_full);
}

int resource_compiler_get_filename(char *filename,
                                   size_t max_ken,
                                   uint64_t type,
                                   uint64_t name) {
    char build_name[33] = {0};
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

char *resource_compiler_get_tmp_dir(ct_allocator *alocator,
                                    const char *platform) {

    char *build_dir = resource_compiler_get_build_dir(alocator, platform);

    return ct_path_a0.join(alocator, 2, build_dir, "tmp");
}

char *resource_compiler_external_join(ct_allocator *alocator,
                                      const char *name) {
    const char *external_dir_str = ct_config_a0.get_string(_G.cv_external_dir);

    char *tmp_dir = ct_path_a0.join(alocator, 2, external_dir_str,
                                    ct_app_a0.native_platform());

    string_stream::Buffer buffer(alocator);
    string_stream::printf(buffer, "%s64", tmp_dir);
    CETECH_FREE(alocator, tmp_dir);

    string_stream::c_str(buffer);
    return ct_path_a0.join(alocator, 4, string_stream::c_str(buffer), "release",
                           "bin", name);
}

extern "C" void resourcecompiler_load_module(ct_api_a0 *api) {
    _init(api);
}

extern "C" void resourcecompiler_unload_module(ct_api_a0 *api) {
    _shutdown();
}

#endif