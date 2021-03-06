#ifndef ALLOCGC_LIST_ALLOCATOR_HPP
#define ALLOCGC_LIST_ALLOCATOR_HPP

#include <memory>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range.hpp>

#include <liballocgc/details/allocators/allocator_tag.hpp>
#include <liballocgc/details/utils/locked_range.hpp>
#include <liballocgc/details/utils/utility.hpp>
#include <liballocgc/details/logging.hpp>
#include <liballocgc/gc_common.hpp>

namespace allocgc { namespace details { namespace allocators {

template <typename UpstreamAlloc, typename Lock>
class list_allocator : UpstreamAlloc, private utils::noncopyable, private utils::nonmovable
{
    struct control_block
    {
        control_block* m_next;
        control_block* m_prev;
    };

    static constexpr size_t get_memblk_size(size_t blk_size)
    {
        return blk_size - sizeof(control_block);
    }

    static constexpr control_block* get_cblk(byte* ptr)
    {
        return reinterpret_cast<control_block*>(ptr);
    }

    static constexpr control_block* get_cblk_by_memblk(byte* ptr)
    {
        return reinterpret_cast<control_block*>(ptr - sizeof(control_block));
    }

    static constexpr byte* get_blk_by_cblk(control_block* cblk)
    {
        return reinterpret_cast<byte*>(cblk);
    }

    static constexpr byte* get_memblk(byte* ptr)
    {
        return ptr + sizeof(control_block);
    }
public:
    class iterator: public boost::iterator_facade<
              iterator
            , byte*
            , boost::bidirectional_traversal_tag
            , byte*
        >
    {
    public:
        iterator(const iterator&) noexcept = default;
        iterator(iterator&&) noexcept = default;

        iterator& operator=(const iterator&) noexcept = default;
        iterator& operator=(iterator&&) noexcept = default;
    private:
        friend class list_allocator;
        friend class boost::iterator_core_access;

        iterator(control_block* cblk) noexcept
            : m_cblk(cblk)
        {
            assert(cblk);
        }

        byte* dereference() const
        {
            return get_memblk(get_blk_by_cblk(m_cblk));
        }

        void increment() noexcept
        {
            m_cblk = m_cblk->m_next;
        }

        void decrement() noexcept
        {
            m_cblk = m_cblk->m_prev;
        }

        bool equal(const iterator& other) const noexcept
        {
            return m_cblk == other.m_cblk;
        }

        control_block* m_cblk;
    };

    typedef byte* pointer_type;
    typedef stateful_alloc_tag alloc_tag;

    typedef boost::iterator_range<iterator> memory_range_type;
    typedef utils::locked_range<
              memory_range_type
            , Lock
        > locked_memory_range_type;

    static constexpr size_t get_blk_size(size_t size)
    {
        return size + sizeof(control_block);
    }

    static constexpr size_t align_size(size_t size, size_t alignment)
    {
        return get_memblk_size(((get_blk_size(size) + alignment - 1) / alignment) * alignment);
    }

    list_allocator()
    {
        init();

    }

    explicit list_allocator(const UpstreamAlloc& upstream_alloc)
        : UpstreamAlloc(upstream_alloc)
    {
        init();
    }

    explicit list_allocator(UpstreamAlloc&& upstream_alloc)
        : UpstreamAlloc(std::move(upstream_alloc))
    {
        init();
    }

    ~list_allocator()
    {
        assert(empty());
    }

    byte* allocate(size_t size)
    {
        size_t blk_size = get_blk_size(size);
        auto deleter = [this, blk_size] (byte* p) {
            upstream_deallocate(p, blk_size);
        };
        std::unique_ptr<byte, decltype(deleter)> blk(upstream_allocate(blk_size), deleter);

        if (!blk) {
            return nullptr;
        }

        control_block* fake = get_fake_block();
        control_block* cblk = get_cblk(blk.get());

        cblk->m_next = fake;
        cblk->m_prev = fake->m_prev;

        std::lock_guard<Lock> lock(m_lock);

        fake->m_prev->m_next = cblk;
        fake->m_prev = cblk;

        if (m_head == fake) {
            m_head = cblk;
        }

        return get_memblk(blk.release());
    }

    void deallocate(byte* ptr, size_t size)
    {
        std::unique_lock<Lock> lock(m_lock);

        control_block* cblk = get_cblk_by_memblk(ptr);
        if (cblk == m_head) {
            m_head = cblk->m_next;
        }
        cblk->m_next->m_prev = cblk->m_prev;
        cblk->m_prev->m_next = cblk->m_next;

        lock.unlock();

        upstream_deallocate(get_blk_by_cblk(cblk), get_blk_size(size));
    }

    bool empty() const
    {
        std::lock_guard<Lock> lock(m_lock);
        return m_head == &m_fake;
    }

    iterator begin()
    {
        return iterator(m_head);
    }

    iterator end()
    {
        return iterator(get_fake_block());
    }

    memory_range_type memory_range()
    {
        return memory_range_type(begin(), end());
    }

    locked_memory_range_type locked_memory_range()
    {
        std::unique_lock<Lock> lock(m_lock);
        return locked_memory_range_type(
                memory_range(),
                std::move(lock)
        );
    }

    const UpstreamAlloc& upstream_allocator() const
    {
        return *this;
    }
private:
    void init()
    {
        m_head        = get_fake_block();
        m_fake.m_next = get_fake_block();
        m_fake.m_prev = get_fake_block();
    }

    control_block* get_fake_block()
    {
        return &m_fake;
    }

    byte* upstream_allocate(size_t size)
    {
        return UpstreamAlloc::allocate(size);
    }

    void upstream_deallocate(byte* ptr, size_t size)
    {
        UpstreamAlloc::deallocate(ptr, size);
    }

    control_block  m_fake;
    control_block* m_head;
    mutable Lock   m_lock;
};

}}}

#endif //ALLOCGC_LIST_ALLOCATOR_HPP
