#include "allocators/plain_pool_chunk.h"

#include <cassert>

namespace precisegc { namespace details { namespace allocators {

plain_pool_chunk::plain_pool_chunk(byte* chunk, size_t size, size_t obj_size) noexcept
    : m_chunk(chunk)
    , m_next(0)
    , m_available(0)
{
    assert(chunk);
    assert(size > 0 && obj_size > 0 && (size % obj_size == 0));
    size_t cnt = size / obj_size;
    assert(cnt <= CHUNK_MAXSIZE);
    m_available = cnt;
    byte* ptr = m_chunk;
    for (size_t i = 1; i <= cnt; ++i, ptr += obj_size) {
        ptr[0] = i;
    }
}

byte* plain_pool_chunk::allocate(size_t obj_size) noexcept
{
    assert(is_memory_available());
    byte* ptr = m_chunk + (m_next * obj_size);
    m_next = ptr[0];
    m_available--;
    return ptr;
}

void plain_pool_chunk::deallocate(byte* ptr, size_t obj_size) noexcept
{
    ptrdiff_t ind = (ptr - m_chunk) / obj_size;
    assert(ind < CHUNK_MAXSIZE);
    ptr[0] = m_next;
    m_next = ind;
    m_available++;
}

bool plain_pool_chunk::is_memory_available() const noexcept
{
    return m_available > 0;
}

}}}