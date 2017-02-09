#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_handle.hpp>

#include <libprecisegc/details/collectors/static_root_set.hpp>
#include <libprecisegc/details/collectors/memory_index.hpp>
#include <libprecisegc/details/gc_facade.hpp>

namespace precisegc { namespace details {

static gc_facade gc_instance{};

void gc_initialize(std::unique_ptr<gc_strategy> strategy)
{
    gc_instance.init(std::move(strategy));
}

bool gc_is_heap_ptr(const gc_handle* ptr)
{
    using namespace threads;
    using namespace collectors;
    return gc_index_memory(reinterpret_cast<const byte*>(ptr)) != nullptr;
}

allocators::gc_alloc_response gc_allocate(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta)
{
    return gc_instance.allocate(obj_size, obj_cnt, tmeta);
}

void gc_commit(gc_cell& cell)
{
    gc_instance.commit(cell);
}

void gc_commit(gc_cell& cell, const gc_type_meta* type_meta)
{
    gc_instance.commit(cell, type_meta);
}

void gc_register_handle(gc_handle& handle, byte* ptr)
{
    gc_instance.register_handle(handle, ptr);
}

void gc_deregister_handle(gc_handle& handle)
{
    gc_instance.deregister_handle(handle);
}

void gc_register_stack_entry(gc_new_stack_entry* stack_entry)
{
    gc_instance.register_stack_entry(stack_entry);
}

void gc_deregister_stack_entry(gc_new_stack_entry* stack_entry)
{
    gc_instance.deregister_stack_entry(stack_entry);
}

void gc_register_thread(const thread_descriptor& descr)
{
    gc_instance.register_thread(descr);
}

void gc_deregister_thread(std::thread::id id)
{
    gc_instance.deregister_thread(id);
}

void gc_initiation_point(initiation_point_type ipoint, const gc_options& opt)
{
    gc_instance.initiation_point(ipoint, opt);
}

void gc_add_to_index(const byte* mem, size_t size, gc_memory_descriptor* entry)
{
    gc_instance.add_to_index(mem, size, entry);
}

void gc_remove_from_index(const byte* mem, size_t size)
{
    gc_instance.remove_from_index(mem, size);
}

gc_memory_descriptor* gc_index_memory(const byte* mem)
{
    return gc_instance.index_memory(mem);
}

gc_cell gc_index_object(byte* obj_start)
{
    return gc_instance.index_object(obj_start);
}

gc_stat gc_get_stats()
{
    return gc_instance.stats();
}

void gc_enable_print_stats()
{
    gc_instance.set_printer_enabled(true);
}

void gc_disable_print_stats()
{
    gc_instance.set_printer_enabled(false);
}

void gc_register_page(const byte* page, size_t size)
{
    gc_instance.register_page(page, size);
}

void gc_deregister_page(const byte* page, size_t size)
{
    gc_instance.deregister_page(page, size);
}

byte* gc_handle::rbarrier() const
{
    return gc_instance.rbarrier(*this);
}

void gc_handle::wbarrier(const gc_handle& other)
{
    gc_instance.wbarrier(*this, other);
}

void gc_handle::interior_wbarrier(ptrdiff_t offset)
{
    gc_instance.interior_wbarrier(*this, offset);
}

gc_handle::pin_guard gc_handle::pin() const
{
    return pin_guard(*this);
}

gc_handle::stack_pin_guard gc_handle::push_pin() const
{
    return stack_pin_guard(*this);
}

void gc_handle::reset()
{
    gc_instance.wbarrier(*this, null);
}

bool gc_handle::equal(const gc_handle& other) const
{
    return gc_instance.compare(*this, other);
}

bool gc_handle::is_null() const
{
    return rbarrier() == nullptr;
}

gc_handle::pin_guard::pin_guard(const gc_handle& handle)
    : m_ptr(gc_instance.register_pin(handle))
{}

gc_handle::pin_guard::pin_guard(pin_guard&& other)
    : m_ptr(other.m_ptr)
{
    other.m_ptr = nullptr;
}

gc_handle::pin_guard::~pin_guard()
{
    gc_instance.deregister_pin(m_ptr);
}

gc_handle::pin_guard& gc_handle::pin_guard::operator=(pin_guard&& other)
{
    std::swap(m_ptr, other.m_ptr);
    return *this;
}

byte* gc_handle::pin_guard::get() const noexcept
{
    return m_ptr;
}

gc_handle::stack_pin_guard::stack_pin_guard(const gc_handle& handle)
    : m_ptr(gc_instance.push_pin(handle))
{}

gc_handle::stack_pin_guard::stack_pin_guard(stack_pin_guard&& other)
    : m_ptr(other.m_ptr)
{
    other.m_ptr = nullptr;
}

gc_handle::stack_pin_guard::~stack_pin_guard()
{
    gc_instance.pop_pin(m_ptr);
}

byte* gc_handle::stack_pin_guard::get() const noexcept
{
    return m_ptr;
}

gc_handle gc_handle::null{nullptr};

}}