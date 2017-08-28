#ifndef CELIB_HANDLERID_TYPES_INL
#define CELIB_HANDLERID_TYPES_INL

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

#include "queue.inl"
#include "array.inl"
#include "allocator.h"
#include "container_types.inl"

struct cel_alloc;

namespace celib {
    template<typename T>
    struct Handler {
        Handler();

        Handler(cel_alloc *allocator);

        void init(cel_alloc *allocator);

        void destroy();

        Array<uint32_t> _generation;
        Queue<uint32_t> _freeIdx;
    };
}

#endif //CELIB_HANDLERID_TYPES_INL