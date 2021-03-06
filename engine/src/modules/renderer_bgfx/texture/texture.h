#ifndef CETECH_BGFX_TEXTURE_RESOURCE_H
#define CETECH_BGFX_TEXTURE_RESOURCE_H

namespace texture {
    int texture_init(struct ct_api_a0 *api);

    void texture_shutdown();

    bgfx::TextureHandle texture_get(uint64_t name);
}

#endif //CETECH_BGFX_TEXTURE_RESOURCE_H
