//==============================================================================
// Based on bitsquid foundation.
// git+web: https://bitbucket.org/bitsquid/foundation
//==============================================================================

#ifndef CETECH_ARRAY_H
#define CETECH_ARRAY_H

//==============================================================================
// Includes
//==============================================================================


#include <stdint.h>
#include <stddef.h>
#include <memory.h>

#include "cetech/kernel/log.h"
#include "cetech/kernel/errors.h"
#include "allocator.h"

//==============================================================================
// Interface macros
//==============================================================================

#define ARRAY_T(N)                 struct array_##N

#define ARRAY_INIT(N, a, alloc)    array_init_##N(a, alloc)
#define ARRAY_DESTROY(N, a)        array_destroy_##N(a)

#define ARRAY_SIZE(a)              (a)->size
#define ARRAY_CAPACITY(a)          (a)->capacity
#define ARRAY_BEGIN(a)             (a)->data
#define ARRAY_END(a)               ((a)->data + (a)->size)
#define ARRAY_AT(a, i)             (a)->data[i]

#define ARRAY_RESIZE(N, a, s)       array_resize_##N(a, s)
#define ARRAY_RESERVE(N, a, s)      array_reserve_##N(a, s)

#define ARRAY_PUSH_BACK(N, a, i)   array_push_back_##N(a, i)
#define ARRAY_PUSH(N, a, i, c)     array_push_##N(a, i, c)
#define ARRAY_POP_BACK(N, a)       array_pop_back_##N(a)


//==============================================================================
// Prototypes macro
//==============================================================================

#define ARRAY_PROTOTYPE(T)  ARRAY_PROTOTYPE_N(T, T)
#define ARRAY_PROTOTYPE_N(T, N)                                                  \
    struct array_##N {                                                           \
        struct allocator* allocator;                                         \
        T *data;                                                                 \
        size_t size;                                                             \
        size_t capacity;                                                         \
    };                                                                           \
                                                                                 \
    static inline void array_init_##N(struct array_##N *array,                   \
                                      struct allocator* allocator) {         \
        CETECH_ASSERT("array_"#T, array != NULL);                                   \
        CETECH_ASSERT("array_"#T, allocator != NULL);                               \
        array->data = NULL;                                                      \
        array->size = 0;                                                         \
        array->capacity = 0;                                                     \
        array->allocator = allocator;                                            \
    }                                                                            \
                                                                                 \
    static inline  void array_destroy_##N(struct array_##N *a) {                 \
        CETECH_ASSERT("array_"#T, a != NULL);                                       \
        CETECH_ASSERT("array_"#T, a->allocator != NULL);                            \
        allocator_deallocate(a->allocator, a->data);                             \
        a->data = NULL;                                                          \
        a->size = 0;                                                             \
        a->capacity = 0;                                                         \
        a->allocator = NULL;                                                     \
    }                                                                            \
                                                                                 \
    static inline  void array_resize_##N(struct array_##N *a, size_t newsize);   \
    static inline  void array_grow_##N(struct array_##N *a, size_t mincapacity); \
                                                                                 \
    static inline  void array_setcapacity_##N(struct array_##N *a,               \
                                              size_t  newcapacity) {             \
        CETECH_ASSERT("array_"#T, a != NULL);                                       \
        CETECH_ASSERT("array_"#T, a->allocator != NULL);                            \
                                                                                 \
        if (newcapacity == a->capacity) {                                        \
            return;                                                              \
        }                                                                        \
                                                                                 \
        if (newcapacity < a->size) {                                             \
            array_resize_##N(a, newcapacity);                                    \
        }                                                                        \
                                                                                 \
        T* newdata = 0;                                                          \
        if (newcapacity > 0) {                                                   \
            newdata = (T*) CETECH_ALLOCATE(a->allocator, T,                         \
                                       sizeof(T) * newcapacity);                 \
            CETECH_ASSERT("array_"#T, newdata !=NULL);                              \
            memcpy(newdata, a->data, sizeof(T) * a->size);                  \
        }                                                                        \
                                                                                 \
        allocator_deallocate(a->allocator, a->data);                             \
                                                                                 \
        a->data = newdata;                                                       \
        a->capacity = newcapacity;                                               \
    }                                                                            \
                                                                                 \
    static inline  void array_grow_##N(struct array_##N *a,                      \
                                       size_t mincapacity) {                     \
        CETECH_ASSERT("array_"#T, a != NULL);                                       \
        size_t newcapacity = a->capacity * 2 + 8;                                \
                                                                                 \
        if (newcapacity < mincapacity) {                                         \
            newcapacity = mincapacity;                                           \
        }                                                                        \
                                                                                 \
        array_setcapacity_##N(a, newcapacity);                                   \
    }                                                                            \
                                                                                 \
    static inline  void array_resize_##N(struct array_##N *a, size_t newsize) {  \
        CETECH_ASSERT("array_"#T, a != NULL);                                       \
        if (newsize > a->capacity) {                                             \
            array_grow_##N(a, newsize);                                          \
        }                                                                        \
                                                                                 \
        a->size = newsize;                                                       \
    }                                                                            \
                                                                                 \
    static inline  void array_reserve_##N(struct array_##N *a,                   \
                                          size_t new_capacity) {                 \
        CETECH_ASSERT("array_"#T, a != NULL);                                       \
        if (new_capacity > a->capacity) {                                        \
            array_setcapacity_##N(a, new_capacity);                              \
        }                                                                        \
    }                                                                            \
                                                                                 \
    static inline  void array_push_back_##N(struct array_##N *a, T item) {       \
        CETECH_ASSERT("array_"#T, a != NULL);                                       \
        if (a->size + 1 > a->capacity) {                                         \
            array_grow_##N(a, 0);                                                \
        }                                                                        \
        a->data[a->size++] = item;                                               \
    }                                                                            \
                                                                                 \
    static inline  void array_push_##N(struct array_##N *a,                      \
                                       T* items,                                 \
                                       size_t count) {                           \
        CETECH_ASSERT("array_"#T, a != NULL);                                       \
        if (a->capacity <= a->size + count) {                                    \
            array_grow_##N(a, a->size + count);                                  \
        }                                                                        \
                                                                                 \
        memcpy(&a->data[a->size], items, sizeof(T) * count);                \
        a->size += count;                                                        \
    }                                                                            \
                                                                                 \
    static inline  void array_pop_back_##N(struct array_##N *a) {                \
        CETECH_ASSERT("array_"#T, a != NULL);                                       \
        CETECH_ASSERT("array_"#T, a->size != 0);                                    \
                                                                                 \
        --a->size;                                                               \
    }                                                                            \

#endif //CETECH_ARRAY_H
