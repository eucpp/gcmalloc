#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>

#include <cstdint>
#include <cassert>
#include <utility>

#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/ptrs/gc_new_stack.hpp>
#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/logging.h>

namespace precisegc { namespace details { namespace ptrs {

gc_untyped_ptr::gc_untyped_ptr()
    : gc_untyped_ptr(nullptr)
{}

//gc_untyped_ptr::gc_untyped_ptr(nullptr_t) noexcept
//    : gc_untyped_ptr()
//{}

gc_untyped_ptr::gc_untyped_ptr(void* ptr)
    : m_ptr(reinterpret_cast<byte*>(ptr))
    , m_root_flag(!gc_new_stack::instance().is_active())
{
    if (m_root_flag) {
        register_root();
    } else {
        if (gc_new_stack::instance().is_meta_requsted()) {
            gc_new_stack::instance().register_child(this);
        }
    }
}

gc_untyped_ptr::gc_untyped_ptr(const gc_untyped_ptr& other)
    : gc_untyped_ptr()
{
    gc_wbarrier(m_ptr, other.m_ptr);
}

gc_untyped_ptr::gc_untyped_ptr(gc_untyped_ptr&& other)
    : gc_untyped_ptr()
{
    gc_wbarrier(m_ptr, other.m_ptr);
}

gc_untyped_ptr::~gc_untyped_ptr()
{
    if (is_root()) {
        delete_root();
    }
}

gc_untyped_ptr& gc_untyped_ptr::operator=(nullptr_t t)
{
    set(nullptr);
    return *this;
}

gc_untyped_ptr& gc_untyped_ptr::operator=(const gc_untyped_ptr& other)
{
    gc_wbarrier(m_ptr, other.m_ptr);
    return *this;
}

gc_untyped_ptr& gc_untyped_ptr::operator=(gc_untyped_ptr&& other)
{
    gc_wbarrier(m_ptr, other.m_ptr);
    return *this;
}

void gc_untyped_ptr::swap(gc_untyped_ptr& other)
{
    gc_untyped_ptr tmp = (*this);
    (*this) = other;
    other = tmp;
}

void* gc_untyped_ptr::get() const
{
    return gc_rbarrier(m_ptr);
}

void gc_untyped_ptr::set(void* ptr)
{
    gc_wbarrier(m_ptr, atomic_byte_ptr((byte*) ptr));
}

gc_untyped_ptr::operator bool() const
{
    return get() != nullptr;
}

bool gc_untyped_ptr::is_root() const
{
    return m_root_flag;
}

void gc_untyped_ptr::register_root()
{
    static thread_local root_stack_map& root_set = threads::managed_thread::this_thread().root_set();
    root_set.insert(this);
}

void gc_untyped_ptr::delete_root()
{
    static thread_local root_stack_map& root_set = threads::managed_thread::this_thread().root_set();
    root_set.remove(this);
}

void swap(gc_untyped_ptr& a, gc_untyped_ptr& b)
{
    a.swap(b);
}

}}}