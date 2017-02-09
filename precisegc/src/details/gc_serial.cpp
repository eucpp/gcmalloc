#include <libprecisegc/details/collectors/gc_serial.hpp>

#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/collectors/gc_tagging.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <netinet/in.h>

namespace precisegc { namespace details { namespace collectors {

gc_serial::gc_serial(size_t threads_available, const thread_descriptor& main_thrd_descr)
    : gc_core(main_thrd_descr)
    , m_marker(&m_packet_manager, nullptr)
    , m_threads_available(threads_available)
{}

void gc_serial::commit(gc_cell& cell)
{
    cell.commit();
}

void gc_serial::commit(gc_cell& cell, const gc_type_meta* type_meta)
{
    cell.commit(type_meta);
}

void gc_serial::wbarrier(gc_handle& dst, const gc_handle& src)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = gc_tagging::clear_root_bit(gc_handle_access::get<std::memory_order_relaxed>(src));
    bool root_bit = gc_tagging::is_root(gc_handle_access::get<std::memory_order_relaxed>(dst));
    gc_handle_access::set<std::memory_order_relaxed>(dst, gc_tagging::set_root_bit(ptr, root_bit));
}

gc_run_stats gc_serial::gc(const gc_options& options)
{
    if ((options.kind != gc_kind::MARK_COLLECT) && (options.kind != gc_kind::COLLECT)) {
        return gc_run_stats();
    }

    gc_run_stats stats = sweep();
    allocators::gc_core_allocator::shrink();
    return stats;
}

gc_run_stats gc_serial::sweep()
{
    auto snapshot = stop_the_world();
    m_marker.trace_roots(snapshot, get_static_roots());
    m_marker.trace_pins(snapshot);
    m_marker.concurrent_mark(m_threads_available - 1);
    m_marker.mark();
    destroy_unmarked_dptrs();

    gc_run_stats stats;
    stats.heap_stat = collect(snapshot, m_threads_available);

    stats.pause_stat.type       = gc_pause_type::MARK_COLLECT;
    stats.pause_stat.duration   = snapshot.time_since_stop_the_world();

    return stats;
}

gc_info gc_serial::info() const
{
    static gc_info inf = {
            .incremental_flag                = false,
            .support_concurrent_marking      = false,
            .support_concurrent_collecting   = false
    };
    return inf;
}

}}}

