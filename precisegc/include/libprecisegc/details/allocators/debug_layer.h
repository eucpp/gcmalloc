#ifndef DIPLOMA_DEBUG_LAYER_H
#define DIPLOMA_DEBUG_LAYER_H

#include <cstddef>

#include "types.h"

namespace precisegc { namespace details { namespace allocators {

template <typename Alloc>
class debug_layer
{
    static_assert(std::is_same<typename Alloc::pointer_type, byte*>::value,
                  "debug_layer should be used with raw memory allocator");
public:
    typedef byte* pointer_type;

    debug_layer()
        : m_allocated_mem(0)
    {}

    debug_layer(const debug_layer&) = default;
    debug_layer(debug_layer&&) = default;

    byte* allocate(size_t size)
    {
        m_allocated_mem += size;
        return m_allocator.allocate(size);
    }

    void deallocate(byte* ptr, size_t size)
    {
        m_allocated_mem -= size;
        m_allocator.deallocate(ptr, size);
    }

    size_t get_allocated_mem_size() const
    {
        return m_allocated_mem;
    }

private:
    Alloc m_allocator;
    size_t m_allocated_mem;
};

}}}

#endif //DIPLOMA_DEBUG_LAYER_H