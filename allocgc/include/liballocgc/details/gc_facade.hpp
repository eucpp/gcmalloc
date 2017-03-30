#ifndef ALLOCGC_GARBAGE_COLLECTOR_HPP
#define ALLOCGC_GARBAGE_COLLECTOR_HPP

#include <memory>
#include <mutex>

#include <liballocgc/details/allocators/index_tree.hpp>
#include <liballocgc/details/utils/utility.hpp>
#include <liballocgc/gc_strategy.hpp>
#include <liballocgc/details/gc_manager.hpp>
#include <liballocgc/gc_handle.hpp>
#include <liballocgc/details/logging.hpp>

namespace allocgc { namespace details {

class gc_facade : private utils::noncopyable, private utils::nonmovable
{
public:
    gc_facade();

    void init(std::unique_ptr<gc_strategy> strategy);

    gc_strategy* get_strategy() const;
    std::unique_ptr<gc_strategy> set_strategy(std::unique_ptr<gc_strategy> strategy);

    inline gc_alloc::response allocate(const gc_alloc::request& rqst)
    {
        assert(m_strategy);
        return m_strategy->allocate(rqst);
    }

    inline void abort(const gc_alloc::response& rsp)
    {
        assert(m_strategy);
        m_strategy->abort(rsp);
    }

    inline void commit(const gc_alloc::response& rsp)
    {
        assert(m_strategy);
        m_strategy->commit(rsp);
    }

    inline void commit(const gc_alloc::response& rsp, const gc_type_meta* type_meta)
    {
        assert(m_strategy);
        m_strategy->commit(rsp, type_meta);
    }

    inline gc_offsets make_offsets(const gc_alloc::response& rsp)
    {
        assert(m_strategy);
        return m_strategy->make_offsets(rsp);
    }

    inline void register_handle(gc_handle& handle, byte* ptr)
    {
        assert(m_strategy);
        m_strategy->register_handle(handle, ptr);
    }

    inline void deregister_handle(gc_handle& handle)
    {
        assert(m_strategy);
        m_strategy->deregister_handle(handle);
    }

    inline byte* register_pin(const gc_handle& handle)
    {
        assert(m_strategy);
        return m_strategy->register_pin(handle);
    }

    inline void deregister_pin(byte* ptr)
    {
        assert(m_strategy);
        m_strategy->deregister_pin(ptr);
    }

    inline byte* push_pin(const gc_handle& handle)
    {
        assert(m_strategy);
        return m_strategy->push_pin(handle);
    }

    inline void pop_pin(byte* ptr)
    {
        assert(m_strategy);
        m_strategy->pop_pin(ptr);
    }

    inline byte* rbarrier(const gc_handle& handle)
    {
        assert(m_strategy);
        return m_strategy->rbarrier(handle);
    }

    inline void wbarrier(gc_handle& dst, const gc_handle& src)
    {
        assert(m_strategy);
        m_strategy->wbarrier(dst, src);
    }

    inline void interior_wbarrier(gc_handle& handle, ptrdiff_t offset)
    {
        assert(m_strategy);
        m_strategy->interior_wbarrier(handle, offset);
    }

    bool compare(const gc_handle& a, const gc_handle& b);

    void register_thread(const thread_descriptor& descr);
    void deregister_thread(std::thread::id id);

    bool increment_heap_size(size_t alloc_size);
    void decrement_heap_size(size_t size);
    void set_heap_limit(size_t size);
    void expand_heap();

    void initiation_point(initiation_point_type ipt, const gc_options& opt);

    bool is_printer_enabled() const;
    void set_printer_enabled(bool enabled);

    void register_page(const byte* page, size_t size);
    void deregister_page(const byte* page, size_t size);

    gc_stat stats() const;
private:
    bool is_interior_pointer(const gc_handle& handle, byte* iptr);
    bool is_interior_offset(const gc_handle& handle, ptrdiff_t shift);

    static const size_t HEAP_START_LIMIT;

    static const double INCREASE_FACTOR;
    static const double MARK_THRESHOLD;
    static const double COLLECT_THRESHOLD;

    std::unique_ptr<gc_strategy> m_strategy;
    gc_manager m_manager;
    std::mutex m_gc_mutex;
    std::mutex m_heap_mutex;
    size_t m_heap_limit;
    size_t m_heap_maxlimit;
    size_t m_heap_size;
};

}}

#endif //ALLOCGC_GARBAGE_COLLECTOR_HPP