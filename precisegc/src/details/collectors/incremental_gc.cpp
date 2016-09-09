#include <libprecisegc/details/collectors/incremental_gc.hpp>

#include <cassert>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <libprecisegc/details/threads/this_managed_thread.hpp>

namespace precisegc { namespace details { namespace collectors {

namespace internals {

incremental_gc_base::incremental_gc_base(gc_compacting compacting, size_t threads_available)
    : m_heap(compacting)
    , m_marker(&m_packet_manager)
    , m_phase(gc_phase::IDLE)
    , m_threads_available(threads_available)
{}

gc_pointer_type incremental_gc_base::allocate(size_t size)
{
    gc_pointer_type ptr = m_heap.allocate(size);
    if (m_phase == gc_phase::MARK) {
        ptr.decorated().set_mark(true);
    }
    return ptr;
}

byte* incremental_gc_base::rbarrier(const gc_handle& handle)
{
    return gc_handle_access::get<std::memory_order_acquire>(handle);
}

void incremental_gc_base::wbarrier(gc_handle& dst, const gc_handle& src)
{
    gc_unsafe_scope unsafe_scope;
    byte* p = gc_handle_access::get<std::memory_order_acquire>(src);
    gc_handle_access::set<std::memory_order_release>(dst, p);
    if (m_phase == gc_phase::MARK) {
        managed_ptr mp(p);
        if (mp && !mp.get_mark()) {
            packet_manager::mark_packet_handle& packet = threads::this_managed_thread::get_mark_packet();
            if (!packet) {
                packet = m_packet_manager.pop_output_packet();
            } else if (packet->is_full()) {
                auto new_packet = m_packet_manager.pop_output_packet();
                m_packet_manager.push_packet(std::move(packet));
                packet = std::move(new_packet);
            }
            packet->push(mp);
        }
    }
}

void incremental_gc_base::interior_wbarrier(gc_handle& handle, byte* ptr)
{
    gc_handle_access::set<std::memory_order_release>(handle, ptr);
}

void incremental_gc_base::interior_shift(gc_handle& handle, ptrdiff_t shift)
{
    gc_handle_access::advance<std::memory_order_acq_rel>(handle, shift);
}

gc_run_stats incremental_gc_base::gc(const gc_options& options)
{
    if (options.phase == gc_phase::MARK && m_phase == gc_phase::IDLE) {
        return start_marking();
    } else if (options.phase == gc_phase::COLLECT) {
        return sweep();
    }
    gc_run_stats stats = {
            .type           = gc_type::SKIP_GC,
            .mem_swept      = 0,
            .mem_copied     = 0,
            .pause_duration = gc_clock::duration(0)
    };
    return stats;
}

void incremental_gc_base::flush_threads_packets(const threads::world_snapshot& snapshot)
{
    snapshot.apply_to_threads([this] (threads::managed_thread* thread ) {
        if (thread->get_mark_packet()) {
            m_packet_manager.push_packet(std::move(thread->get_mark_packet()));
            thread->get_mark_packet() = nullptr;
        }
    });
}

gc_run_stats incremental_gc_base::start_marking()
{
    using namespace threads;
    assert(m_phase == gc_phase::IDLE);

    world_snapshot snapshot = thread_manager::instance().stop_the_world();
    m_marker.trace_roots(snapshot.get_root_tracer());
    m_phase = gc_phase::MARK;
    m_marker.concurrent_mark(std::max((size_t) 1, m_threads_available - 1));

    gc_run_stats stats = {
            .type           = gc_type::TRACE_ROOTS,
            .mem_swept      = 0,
            .mem_copied     = 0,
            .pause_duration = snapshot.time_since_stop_the_world()
    };

    return stats;
}

gc_run_stats incremental_gc_base::sweep()
{
    using namespace threads;
    assert(m_phase == gc_phase::IDLE || m_phase == gc_phase::MARK);

    gc_type type;
    world_snapshot snapshot = thread_manager::instance().stop_the_world();
    if (m_phase == gc_phase::IDLE) {
        type = gc_type::FULL_GC;
        m_marker.trace_roots(snapshot.get_root_tracer());
        m_marker.trace_pins(snapshot.get_pin_tracer());
        m_phase = gc_phase::MARK;
        m_marker.concurrent_mark(m_threads_available - 1);
        m_marker.mark();
    } else if (m_phase == gc_phase::MARK) {
        type = gc_type::COLLECT_GARBAGE;
        m_marker.trace_pins(snapshot.get_pin_tracer());
        flush_threads_packets(snapshot);
        m_marker.mark();
    }
    m_phase = gc_phase::COLLECT;
    auto collect_stats = m_heap.collect(snapshot, m_threads_available);
    m_phase = gc_phase::IDLE;

    gc_run_stats stats = gc_run_stats {
            .type           = type,
            .mem_swept      = collect_stats.mem_swept,
            .mem_copied     = collect_stats.mem_copied,
            .pause_duration = snapshot.time_since_stop_the_world()
    };

    return stats;
}

}

incremental_gc::incremental_gc(size_t threads_available)
    : incremental_gc_base(gc_compacting::DISABLED, threads_available)
{}

bool incremental_gc::compare(const gc_handle& a, const gc_handle& b)
{
    return gc_handle_access::get<std::memory_order_acquire>(a) == gc_handle_access::get<std::memory_order_acquire>(b);
}

byte* incremental_gc::pin(const gc_handle& handle)
{
    byte* ptr = gc_handle_access::get<std::memory_order_acquire>(handle);
    if (ptr) {
        threads::this_managed_thread::pin(ptr);
    }
    return ptr;
}

void incremental_gc::unpin(byte* ptr)
{
    threads::this_managed_thread::unpin(ptr);
}

gc_info incremental_gc::info() const
{
    static gc_info inf = {
            .incremental_flag           = true,
            .support_concurrent_marking    = true,
            .support_concurrent_collecting   = false
    };

    return inf;
}

incremental_compacting_gc::incremental_compacting_gc(size_t threads_available)
        : incremental_gc_base(gc_compacting::ENABLED, threads_available)
{}

bool incremental_compacting_gc::compare(const gc_handle& a, const gc_handle& b)
{
    gc_unsafe_scope unsafe_scope;
    return gc_handle_access::get<std::memory_order_acquire>(a) == gc_handle_access::get<std::memory_order_acquire>(b);
}

byte* incremental_compacting_gc::pin(const gc_handle& handle)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = gc_handle_access::get<std::memory_order_acquire>(handle);
    if (ptr) {
        threads::this_managed_thread::pin(ptr);
    }
    return ptr;
}

void incremental_compacting_gc::unpin(byte* ptr)
{
    threads::this_managed_thread::unpin(ptr);
}

gc_info incremental_compacting_gc::info() const
{
    static gc_info inf = {
            .incremental_flag                = true,
            .support_concurrent_marking      = true,
            .support_concurrent_collecting   = false
    };

    return inf;
}

}}}

