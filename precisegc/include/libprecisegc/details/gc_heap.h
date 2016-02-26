#ifndef DIPLOMA_HEAP_H
#define DIPLOMA_HEAP_H

#include <utility>

#include "../object.h"
#include "object_meta.h"
#include "segregated_list.h"
#include "mutex.h"
#include "allocators/constants.h"
#include "util.h"

namespace precisegc { namespace details {

class gc_heap : public noncopyable, public nonmovable
{
public:

    typedef std::pair<void*, size_t> allocate_result;

    static gc_heap& instance()
    {
        static gc_heap h;
        return h;
    }

    allocate_result allocate(size_t size);

    void compact();

    size_t size() noexcept;
private:
    static const size_t MIN_ALLOC_SIZE_BITS = 4;
    static const size_t SEGREGATED_STORAGE_SIZE = (POINTER_BITS_CNT - RESERVED_BITS_CNT - 1);
    static size_t align_size(size_t size);

    gc_heap();
    gc_heap(const gc_heap&&) = delete;
    gc_heap& operator=(const gc_heap&&) = delete;

    forwarding_list compact_memory();
    void fix_pointers(const forwarding_list& frwd);

    //void fix_pointers(const forwarding_list& forwarding);

    segregated_list m_storage[SEGREGATED_STORAGE_SIZE];
    mutex m_mutex;
    size_t m_size;
};

}}

#endif //DIPLOMA_HEAP_H
