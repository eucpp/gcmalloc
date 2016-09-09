#ifndef DIPLOMA_FIXSIZE_FREELIST_ALLOCATOR_HPP
#define DIPLOMA_FIXSIZE_FREELIST_ALLOCATOR_HPP

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/utils/get_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details { namespace allocators {

struct zeroing_enabled {};
struct zeroing_disabled {};

struct shrinking_enabled {};
struct shrinking_disabled {};

template <typename UpstreamAlloc, typename ShrinkingPolicy = shrinking_enabled, typename ZeroingPolicy = zeroing_disabled>
class fixsize_freelist_allocator : private utils::ebo<UpstreamAlloc>,
                                   private utils::noncopyable, private utils::nonmovable
{
public:
    typedef typename UpstreamAlloc::pointer_type pointer_type;
    typedef typename UpstreamAlloc::memory_range_type memory_range_type;
    typedef stateful_alloc_tag alloc_tag;

    fixsize_freelist_allocator()
        : m_head(nullptr)
    {}

    pointer_type allocate(size_t size)
    {
        assert(sizeof(pointer_type) <= size);
        if (m_head) {
            pointer_type ptr = m_head;
            m_head = *reinterpret_cast<pointer_type*>(utils::get_ptr(ptr));
            if (is_zeroing_enabled) {
                memset(utils::get_ptr(ptr), 0, size);
            }
            return ptr;
        }
        return mutable_upstream_allocator().allocate(size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        assert(ptr);
        assert(sizeof(pointer_type) <= size);
        pointer_type* next = reinterpret_cast<pointer_type*>(utils::get_ptr(ptr));
        *next = m_head;
        m_head = ptr;
    }

    // definitely not a good solutin:
    // we define two versions of shrink() with size argument and without;
    // second version expects to get size from pointer_type::size() method of m_head;
    // clearly, SFINAE should be used here but we're too lazy

    size_t shrink()
    {
        return shrink(m_head.size());
    }

    size_t shrink(size_t size)
    {
        if (is_shrinking_enabled) {
            while (m_head) {
                pointer_type next = *reinterpret_cast<pointer_type*>(utils::get_ptr(m_head));
                mutable_upstream_allocator().deallocate(m_head, size);
                m_head = next;
            }
        } else {
            // we just throw away whole freelist,
            // because it will be recreated during sweeping
            // (we use this strategy only in managed_heap)
            m_head = nullptr;
        }
        return mutable_upstream_allocator().shrink();
    }

    bool empty() const
    {
        return upstream_allocator().empty();
    }

    template <typename Functor>
    void apply_to_chunks(Functor&& f)
    {
        mutable_upstream_allocator().apply_to_chunks(std::forward<Functor>(f));
    }

    memory_range_type memory_range()
    {
        return mutable_upstream_allocator().memory_range();
    }

    const UpstreamAlloc& upstream_allocator() const
    {
        return this->template get_base<UpstreamAlloc>();
    }
private:
    static const bool is_zeroing_enabled = std::is_same<ZeroingPolicy, zeroing_enabled>::value;
    static const bool is_shrinking_enabled = std::is_same<ShrinkingPolicy, shrinking_enabled>::value;

    UpstreamAlloc& mutable_upstream_allocator()
    {
        return this->template get_base<UpstreamAlloc>();
    }

    pointer_type m_head;
};

}}}

#endif //DIPLOMA_FIXSIZE_FREELIST_ALLOCATOR_HPP