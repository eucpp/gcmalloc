#include <libprecisegc/details/collectors/incremental_gc.hpp>

#include <cassert>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_unsafe_scope.h>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>

namespace precisegc { namespace details { namespace collectors {

namespace internals {

incremental_gc_base::incremental_gc_base(gc_compacting compacting)
    : m_heap(compacting)
    , m_phase(gc_phase::IDLE)
{}

managed_ptr incremental_gc_base::allocate(size_t size)
{
    gc_unsafe_scope unsafe_scope;
    managed_ptr mp = m_heap.allocate(size);
    if (m_phase == gc_phase::MARK) {
        mp.set_mark(true);
    }
    return mp;
}

byte* incremental_gc_base::rbarrier(const gc_handle& handle)
{
    return gc_handle_access::load(handle, std::memory_order_acquire);
}

void incremental_gc_base::wbarrier(gc_handle& dst, const gc_handle& src)
{
    gc_unsafe_scope unsafe_scope;
    byte* p = gc_handle_access::load(src, std::memory_order_acquire);
    gc_handle_access::store(dst, p, std::memory_order_release);
    if (m_phase == gc_phase::MARK) {
        bool res = shade(p);
        while (!res) {
            m_marker.trace_barrier_buffers();
            std::this_thread::yield();
            res = shade(p);
        }
    }
}

void incremental_gc_base::interior_wbarrier(gc_handle& handle, byte* ptr)
{
    gc_handle_access::store(handle, ptr, std::memory_order_release);
}

void incremental_gc_base::interior_shift(gc_handle& handle, ptrdiff_t shift)
{
    gc_handle_access::fetch_advance(handle, shift, std::memory_order_acq_rel);
}

void incremental_gc_base::gc(gc_phase phase)
{
    if (phase == gc_phase::MARK && m_phase == gc_phase::IDLE) {
        start_marking();
    } else if (phase == gc_phase::SWEEP) {
        sweep();
    }
}

void incremental_gc_base::start_marking()
{
    using namespace threads;
    assert(m_phase == gc_phase::IDLE);

    world_snapshot snapshot = thread_manager::instance().stop_the_world();
    m_marker.trace_roots(snapshot.get_root_tracer());
    m_phase = gc_phase::MARK;
    m_marker.start_marking();

    gc_pause_stat pause_stat = {
            .type       = gc_pause_type::TRACE_ROOTS,
            .duration   = snapshot.time_since_stop_the_world()
    };
    gc_register_pause(pause_stat);
}

void incremental_gc_base::sweep()
{
    using namespace threads;
    assert(m_phase == gc_phase::IDLE || m_phase == gc_phase::MARK);

    gc_pause_type pause_type;
    if (m_phase == gc_phase::MARK) {
        m_marker.pause_marking();
    }
    world_snapshot snapshot = thread_manager::instance().stop_the_world();
    if (m_phase == gc_phase::IDLE) {
        pause_type = gc_pause_type::GC;
        m_marker.trace_roots(snapshot.get_root_tracer());
        m_marker.trace_pins(snapshot.get_pin_tracer());
        m_phase = gc_phase::MARK;
        m_marker.mark();
    } else if (m_phase == gc_phase::MARK) {
        pause_type = gc_pause_type::SWEEP_HEAP;
        m_marker.trace_pins(snapshot.get_pin_tracer());
        m_marker.trace_barrier_buffers(snapshot);
        m_marker.mark();
    }
    m_phase = gc_phase::SWEEP;

    gc_sweep_stat sweep_stat = m_heap.sweep(snapshot, std::thread::hardware_concurrency());
    gc_pause_stat pause_stat = {
            .type       = pause_type,
            .duration   = snapshot.time_since_stop_the_world()
    };

    gc_register_sweep(sweep_stat, pause_stat);
    m_phase = gc_phase::IDLE;
}

}

incremental_gc::incremental_gc()
    : incremental_gc_base(gc_compacting::DISABLED)
{}

bool incremental_gc::compare(const gc_handle& a, const gc_handle& b)
{
    return gc_handle_access::load(a, std::memory_order_acquire) == gc_handle_access::load(b, std::memory_order_acquire);
}

byte* incremental_gc::pin(const gc_handle& handle)
{
    return gc_handle_access::load(handle, std::memory_order_acquire);
}

void incremental_gc::unpin(byte* ptr)
{
    return;
}

gc_info incremental_gc::info() const
{
    static gc_info inf = {
            .incremental                = true,
            .support_concurrent_mark    = true,
            .support_concurrent_sweep   = false
    };

    return inf;
}

incremental_compacting_gc::incremental_compacting_gc()
        : incremental_gc_base(gc_compacting::ENABLED)
{}

bool incremental_compacting_gc::compare(const gc_handle& a, const gc_handle& b)
{
    gc_unsafe_scope unsafe_scope;
    return gc_handle_access::load(a, std::memory_order_acquire) == gc_handle_access::load(b, std::memory_order_acquire);
}

byte* incremental_compacting_gc::pin(const gc_handle& handle)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = gc_handle_access::load(handle, std::memory_order_acquire);
    if (ptr) {
        static thread_local pin_stack_map& pin_set = threads::managed_thread::this_thread().pin_set();
        pin_set.insert(ptr);
    }
    return ptr;
}

void incremental_compacting_gc::unpin(byte* ptr)
{
    static thread_local pin_stack_map& pin_set = threads::managed_thread::this_thread().pin_set();
    pin_set.remove(ptr);
}

gc_info incremental_compacting_gc::info() const
{
    static gc_info inf = {
            .incremental                = true,
            .support_concurrent_mark    = true,
            .support_concurrent_sweep   = false
    };

    return inf;
}

}}}
