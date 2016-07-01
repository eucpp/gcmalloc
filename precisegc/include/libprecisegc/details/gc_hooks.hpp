#ifndef DIPLOMA_GC_HOOKS_HPP
#define DIPLOMA_GC_HOOKS_HPP

#include <memory>

#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/initiation_policy.hpp>
#include <libprecisegc/details/managed_ptr.hpp>
#include <libprecisegc/gc_stat.hpp>

namespace precisegc { namespace details {

void gc_initialize(std::unique_ptr<gc_strategy> strategy, std::unique_ptr<initiation_policy> init_policy);

managed_ptr gc_allocate(size_t size);

void gc_initiation_point(initiation_point_type ipoint);

gc_info gc_get_info();
gc_stat gc_get_stats();
gc_state gc_get_state();

void gc_enable_print_stats();
void gc_disable_print_stats();

void gc_register_page(const byte* page, size_t size);
void gc_deregister_page(const byte* page, size_t size);
void gc_register_pause(const gc_pause_stat& pause_stat);
void gc_register_sweep(const gc_sweep_stat& sweep_stat, const gc_pause_stat& pause_stat);

}}

#endif //DIPLOMA_GC_HOOKS_HPP
