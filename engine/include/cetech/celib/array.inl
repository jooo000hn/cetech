//==============================================================================
// Based on bitsquid foundation.
// git+web: https://bitbucket.org/bitsquid/foundation
//==============================================================================


#ifndef CETECH_ARRAY2_INL_H
#define CETECH_ARRAY2_INL_H

#include <cstring>

#include <cetech/celib/allocator.h>
#include <cetech/kernel/macros.h>

#include "container_types.inl"

namespace cetech {
    namespace array {
        /// The number of elements in the array.
        template<typename T>
        uint32_t size(const Array <T> &a);

        /// Returns true if there are any elements in the array.
        template<typename T>
        bool any(const Array <T> &a);

        /// Returns true if the array is empty.
        template<typename T>
        bool empty(const Array <T> &a);

        /// Used to iterate over the array.
        template<typename T>
        T *begin(Array <T> &a);

        template<typename T>
        const T *begin(const Array <T> &a);

        template<typename T>
        T *end(Array <T> &a);

        template<typename T>
        const T *end(const Array <T> &a);

        /// Returns the first/last element of the array. Don't use these on an
        /// empty array.
        template<typename T>
        T &front(Array <T> &a);

        template<typename T>
        const T &front(const Array <T> &a);

        template<typename T>
        T &back(Array <T> &a);

        template<typename T>
        const T &back(const Array <T> &a);

        /// Changes the size of the array (does not reallocate memory unless necessary).
        template<typename T>
        void resize(Array <T> &a,
                    uint32_t new_size);

        /// Removes all items in the array (does not free memory).
        template<typename T>
        void clear(Array <T> &a);

        /// Reallocates the array to the specified capacity.
        template<typename T>
        void set_capacity(Array <T> &a,
                          uint32_t new_capacity);

        /// Makes sure that the array has at least the specified capacity.
        /// (If not, the array is grown.)
        template<typename T>
        void reserve(Array <T> &a,
                     uint32_t new_capacity);

        /// Grows the array using a geometric progression formula, so that the ammortized
        /// cost of push_back() is O(1). If a min_capacity is specified, the array will
        /// grow to at least that capacity.
        template<typename T>
        void grow(Array <T> &a,
                  uint32_t min_capacity = 0);

        /// Trims the array so that its capacity matches its size.
        template<typename T>
        void trim(Array <T> &a);

        /// Pushes the item to the end of the array.
        template<typename T>
        void push_back(Array <T> &a,
                       const T &item);

        template<typename T>
        void push(Array <T> &a,
                  const T *item,
                  uint32_t count);


        /// Pops the last item from the array. The array cannot be empty.
        template<typename T>
        void pop_back(Array <T> &a);
    }

    namespace array {
        template<typename T>
        inline uint32_t size(const Array <T> &a) { return a._size; }

        template<typename T>
        inline bool any(const Array <T> &a) { return a._size != 0; }

        template<typename T>
        inline bool empty(const Array <T> &a) { return a._size == 0; }

        template<typename T>
        inline T *begin(Array <T> &a) { return a._data; }

        template<typename T>
        inline const T *begin(const Array <T> &a) { return a._data; }

        template<typename T>
        inline T *end(Array <T> &a) { return a._data + a._size; }

        template<typename T>
        inline const T *end(const Array <T> &a) { return a._data + a._size; }

        template<typename T>
        inline T &front(Array <T> &a) { return a._data[0]; }

        template<typename T>
        inline const T &front(const Array <T> &a) { return a._data[0]; }

        template<typename T>
        inline T &back(Array <T> &a) { return a._data[a._size - 1]; }

        template<typename T>
        inline const T &back(const Array <T> &a) {
            return a._data[a._size - 1];
        }

        template<typename T>
        inline void clear(Array <T> &a) { resize(a, 0); }

        template<typename T>
        inline void trim(Array <T> &a) { set_capacity(a, a._size); }

        template<typename T>
        void resize(Array <T> &a,
                    uint32_t new_size) {
            if (new_size > a._capacity)
                grow(a, new_size);
            a._size = new_size;
        }

        template<typename T>
        inline void reserve(Array <T> &a,
                            uint32_t new_capacity) {
            if (new_capacity > a._capacity)
                set_capacity(a, new_capacity);
        }

        template<typename T>
        void set_capacity(Array <T> &a,
                          uint32_t new_capacity) {
            if (new_capacity == a._capacity)
                return;

            if (new_capacity < a._size)
                resize(a, new_capacity);

            T *new_data = 0;
            if (new_capacity > 0) {
                new_data = CETECH_ALLOCATE(a._allocator, T,
                                           sizeof(T) * new_capacity);
                memcpy(new_data, a._data, sizeof(T) * a._size);
            }

            CETECH_FREE(a._allocator, a._data);

            a._data = new_data;
            a._capacity = new_capacity;
        }

        template<typename T>
        void grow(Array <T> &a,
                  uint32_t min_capacity) {
            uint32_t new_capacity = a._capacity * 2 + 8;
            if (new_capacity < min_capacity)
                new_capacity = min_capacity;
            set_capacity(a, new_capacity);
        }

        template<typename T>
        inline void push_back(Array <T> &a,
                              const T &item) {
            if (a._size + 1 > a._capacity)
                grow(a);
            a._data[a._size++] = item;
        }

        template<typename T>
        void push(Array <T> &a,
                  const T *items,
                  uint32_t count) {
            if (a._capacity <= a._size + count) {
                grow(a, a._size + count);
            }

            memcpy(&a._data[a._size], items, sizeof(T) * count);
            a._size += count;
        }

        template<typename T>
        inline void pop_back(Array <T> &a) {
            a._size--;
        }
    }

    template<typename T>
    inline
    Array<T>::Array() : _allocator(nullptr), _size(0),
                        _capacity(0), _data(0) {}

    template<typename T>
    inline
    Array<T>::Array(struct ct_allocator *a) : _allocator(a), _size(0),
                                              _capacity(0), _data(0) {}

    template<typename T>
    inline
    void Array<T>::init(ct_allocator *a) {
        _data = nullptr;
        _allocator = a;
        _size = 0;
        _capacity = 0;
    }

    template<typename T>
    inline
    void Array<T>::destroy() {
        if (_data) {
            CETECH_FREE(_allocator, _data);
        }

        _allocator = nullptr;
        _data = nullptr;
        _size = 0;
        _capacity = 0;
    }


    template<typename T>
    inline Array<T>::~Array() {
        if (_data) {
            CETECH_FREE(_allocator, _data);
        }

        _allocator = nullptr;
        _data = nullptr;
        _size = 0;
        _capacity = 0;
    }

    template<typename T>

    Array<T>::Array(const Array <T> &other) : _allocator(other._allocator),
                                              _size(0), _capacity(0), _data(0) {
        const uint32_t n = other._size;
        array::set_capacity(*this, n);
        memcpy(_data, other._data, sizeof(T) * n);
        _size = n;
    }

    template<typename T>
    Array <T> &Array<T>::operator=(const Array <T> &other) {
        const uint32_t n = other._size;
        _allocator = other._allocator;
        array::resize(*this, n);
        memcpy(_data, other._data, sizeof(T) * n);
        return *this;
    }

    template<typename T>
    inline T &Array<T>::operator[](uint32_t i) {
        return _data[i];
    }

    template<typename T>
    inline const T &Array<T>::operator[](uint32_t i) const {
        return _data[i];
    }
}
#endif //CETECH_ARRAY2_INL_H
