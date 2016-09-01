#ifndef DIPLOMA_GARBAGE_COLLECTOR_HPP
#define DIPLOMA_GARBAGE_COLLECTOR_HPP

#include <memory>
#include <mutex>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/initiation_policy.hpp>
#include <libprecisegc/details/gc_manager.hpp>
#include <libprecisegc/details/managed_ptr.hpp>
#include <libprecisegc/details/recorder.hpp>
#include <libprecisegc/details/printer.hpp>
#include <libprecisegc/details/logging.hpp>
#include <libprecisegc/gc_stat.hpp>

namespace precisegc { namespace details {

class garbage_collector : private utils::noncopyable, private utils::nonmovable
{
public:
    garbage_collector();

    void init(std::unique_ptr<gc_strategy> strategy, std::unique_ptr<initiation_policy> init_policy);

    gc_strategy* get_strategy() const;
    std::unique_ptr<gc_strategy> set_strategy(std::unique_ptr<gc_strategy> strategy);

    std::pair<managed_ptr, object_meta*> allocate(size_t obj_size, size_t obj_count,
                                                      const type_meta* tmeta);

    byte* rbarrier(const gc_handle& handle);
    void  wbarrier(gc_handle& dst, const gc_handle& src);

    void interior_wbarrier(gc_handle& handle, byte* ptr);
    void interior_shift(gc_handle& handle, ptrdiff_t shift);

    byte* pin(const gc_handle& handle);
    void  unpin(byte* ptr);

    bool compare(const gc_handle& a, const gc_handle& b);

    void initiation_point(initiation_point_type ipt,
                          const initiation_point_data& ipd = initiation_point_data::create_empty_data());

    bool is_printer_enabled() const;
    void set_printer_enabled(bool enabled);

    void register_page(const byte* page, size_t size);
    void deregister_page(const byte* page, size_t size);

    gc_info  info() const;
    gc_stat  stats() const;
    gc_state state() const;
private:
    std::pair<managed_ptr, object_meta*> try_allocate(size_t obj_size, size_t obj_count, const type_meta* tmeta);

    static bool is_interior_pointer(const gc_handle& handle, byte* p);
    static bool is_interior_shift(const gc_handle& handle, ptrdiff_t shift);

    std::unique_ptr<gc_strategy> m_strategy;
    std::unique_ptr<initiation_policy> m_initiation_policy;
    gc_manager m_manager;
    std::mutex m_gc_mutex;
};

}}

#endif //DIPLOMA_GARBAGE_COLLECTOR_HPP
