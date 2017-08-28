//==============================================================================
// includes
//==============================================================================

#include <celib/allocator.h>

#include <cetech/kernel/api_system.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/hashlib.h>

#include <cetech/modules/entity/entity.h>

#include <cetech/modules/renderer/renderer.h>
#include <cetech/modules/camera/camera.h>
#include <cetech/modules/renderer/mesh_renderer.h>

#include "bgfx/platform.h"

#include <cetech/modules/renderer/viewport.h>
#include "cetech/modules/renderer/scene.h"

CETECH_DECL_API(ct_viewport_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_mesh_renderer_a0);
CETECH_DECL_API(ct_camera_a0);


static void geometry_pass(viewport_instance *viewport,
                          ct_viewport viewport_id,
                          uint8_t viewid,
                          uint8_t layerid,
                          ct_world world,
                          ct_camera camera) {

    bgfx::setViewClear(viewid,
                       BGFX_CLEAR_COLOR |
                       BGFX_CLEAR_DEPTH,
                       0x66CCFFff, 1.0f, 0);

    bgfx::setViewRect(viewid, 0, 0,
                      (uint16_t) viewport->size[0],  // TODO: SHITTT
                      (uint16_t) viewport->size[1]); // TODO: SHITTT

    float view_matrix[16];
    float proj_matrix[16];

    ct_camera_a0.get_project_view(camera,
                                  proj_matrix,
                                  view_matrix,
                                  viewport->size[0],
                                  viewport->size[1]);

    auto fb = viewport->framebuffers[layerid];
    bgfx::setViewFrameBuffer(viewid, {fb});

    bgfx::setViewTransform(viewid, view_matrix, proj_matrix);

    // TODO: CULLING
    ct_mesh_renderer_a0.render_all(world, viewid,
                                   viewport->layers[layerid].name);
}

static void _init(struct ct_api_a0 *api) {
    ct_viewport_a0.register_layer_pass(
            ct_hash_a0.id64_from_str("geometry"),
            geometry_pass);
}

static void _shutdown() {

}

CETECH_MODULE_DEF(
        geometry_pass,
        {
            CETECH_GET_API(api, ct_viewport_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_mesh_renderer_a0);
            CETECH_GET_API(api, ct_camera_a0);
        },
        {
            _init(api);
        },
        {
            CEL_UNUSED(api);

            _shutdown();
        }
)