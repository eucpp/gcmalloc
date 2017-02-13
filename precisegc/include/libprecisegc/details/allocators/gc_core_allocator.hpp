#ifndef DIPLOMA_GC_CORE_ALLOCATOR_HPP
#define DIPLOMA_GC_CORE_ALLOCATOR_HPP

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <atomic>
#include <mutex>

#include <boost/range/iterator_range.hpp>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/allocators/sys_allocator.hpp>
#include <libprecisegc/details/allocators/bucket_allocator.hpp>
#include <libprecisegc/details/allocators/freelist_allocator.hpp>
#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/logging.hpp>
#include <libprecisegc/gc_common.hpp>

namespace precisegc { namespace details { namespace allocators {

class gc_core_allocator
{
    static const size_t MAX_BUCKETIZE_SIZE = MANAGED_CHUNK_OBJECTS_COUNT * LARGE_CELL_SIZE;

    class page_bucket_policy
    {
    public:
        static const size_t BUCKET_COUNT = MAX_BUCKETIZE_SIZE / LARGE_CELL_SIZE;

        static size_t bucket(size_t size);
        static size_t bucket_size(size_t i);
    };
public:
    typedef byte* pointer_type;
    typedef stateless_alloc_tag alloc_tag;
    typedef boost::iterator_range<byte*> memory_range_type;

    gc_core_allocator() = default;
    gc_core_allocator(const gc_core_allocator&) = default;
    gc_core_allocator(gc_core_allocator&&) = default;

    gc_core_allocator& operator=(const gc_core_allocator&) = default;
    gc_core_allocator& operator=(gc_core_allocator&&) = default;

    static byte* allocate(size_t size);

    static void deallocate(byte* ptr, size_t size);

    static size_t shrink();

    static memory_range_type memory_range();
private:
    typedef freelist_allocator<sys_allocator> freelist_alloc_t;
    typedef freelist_allocator<sys_allocator> fixsize_page_alloc_t;
    typedef bucket_allocator<fixsize_page_alloc_t, page_bucket_policy> bucket_alloc_t;

    typedef std::mutex mutex_t;

    static bucket_alloc_t bucket_alloc;
    static freelist_alloc_t freelist;
    static mutex_t mutex;
};

}}}

#endif //DIPLOMA_GC_CORE_ALLOCATOR_HPP
