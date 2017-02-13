#ifndef DIPLOMA_GC_THREAD_DESCRIPTOR_HPP
#define DIPLOMA_GC_THREAD_DESCRIPTOR_HPP

#include <thread>
#include <cassert>

#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/logging.hpp>
#include <libprecisegc/details/allocators/memory_index.hpp>
#include <libprecisegc/details/collectors/gc_new_stack_entry.hpp>
#include <libprecisegc/details/collectors/static_root_set.hpp>

namespace precisegc { namespace details { namespace threads {

class gc_thread_descriptor
{
public:
    virtual ~gc_thread_descriptor() {}

    virtual void register_stack_entry(collectors::gc_new_stack_entry* stack_entry) = 0;
    virtual void deregister_stack_entry(collectors::gc_new_stack_entry* stack_entry) = 0;

    virtual void register_root(gc_handle* handle) = 0;
    virtual void deregister_root(gc_handle* handle) = 0;

    virtual void register_heap_ptr(gc_handle* handle) = 0;
    virtual void deregister_heap_ptr(gc_handle* handle) = 0;

    virtual void register_pin(byte* pin) = 0;
    virtual void deregister_pin(byte* pin) = 0;
    virtual void push_pin(byte* pin) = 0;
    virtual void pop_pin(byte* pin) = 0;

    virtual void trace_roots(const gc_trace_callback& cb) const = 0;
    virtual void trace_pins(const gc_trace_pin_callback& cb) const = 0;

    virtual std::thread::id get_id() const = 0;
    virtual std::thread::native_handle_type native_handle() const = 0;
};

template <typename StackRootSet, typename PinSet, typename PinStack, typename InitStack>
class gc_thread_descriptor_impl : public gc_thread_descriptor, private utils::noncopyable, private utils::nonmovable
{
public:
    gc_thread_descriptor_impl(const thread_descriptor& descr)
        : m_stack_roots(descr.stack_start_addr)
        , m_id(descr.id)
        , m_native_handle(descr.native_handle)
    {
        assert(m_stack_roots.stack_size() % PAGE_SIZE == 0);
        allocators::memory_index::index_stack_memory(m_stack_roots.stack_min_addr(),
                                                     m_stack_roots.stack_size(),
                                                     m_stack_roots.stack_start_addr());
    }

    ~gc_thread_descriptor_impl()
    {
        allocators::memory_index::deindex(m_stack_roots.stack_min_addr(),
                                          m_stack_roots.stack_size());
    }

    void register_stack_entry(collectors::gc_new_stack_entry* stack_entry) override
    {
        m_init_stack.register_stack_entry(stack_entry);
    }

    void deregister_stack_entry(collectors::gc_new_stack_entry* stack_entry) override
    {
        m_init_stack.deregister_stack_entry(stack_entry);
    }

    void register_root(gc_handle* handle) override
    {
        m_stack_roots.register_root(handle);
    }

    void deregister_root(gc_handle* handle) override
    {
        m_stack_roots.deregister_root(handle);
    }

    void register_heap_ptr(gc_handle* handle) override
    {
        m_init_stack.register_child(handle);
    }

    void deregister_heap_ptr(gc_handle* handle) override
    {
        return;
    }

    void register_pin(byte* pin) override
    {
        m_pin_set.insert(pin);
    }

    void deregister_pin(byte* pin) override
    {
        m_pin_set.remove(pin);
    }

    void push_pin(byte* ptr) override
    {
        if (!m_pin_stack.is_full()) {
            m_pin_stack.push_pin(ptr);
        } else {
            m_pin_set.insert(ptr);
        }
    }

    void pop_pin(byte* ptr) override
    {
        if (m_pin_stack.top() == ptr) {
            m_pin_stack.pop_pin();
        } else {
            m_pin_set.remove(ptr);
        }
    }

    void trace_roots(const gc_trace_callback& cb) const override
    {
        logging::info() << "Thread "        << m_id << " "
                        << "roots count = " << m_stack_roots.size();
        m_stack_roots.trace(cb);
    }

    void trace_pins(const gc_trace_pin_callback& cb) const override
    {
        logging::info() << "Thread "       << m_id << " "
                        << "pins count = " << m_pin_set.size() + m_pin_stack.size() + m_init_stack.size();
        m_pin_set.trace(cb);
        m_pin_stack.trace(cb);
        m_init_stack.trace(cb);
    }

    std::thread::id get_id() const override
    {
        return m_id;
    }

    std::thread::native_handle_type native_handle() const override
    {
        return m_native_handle;
    }
private:
    StackRootSet m_stack_roots;
    PinSet m_pin_set;
    PinStack m_pin_stack;
    InitStack m_init_stack;
    std::thread::id m_id;
    std::thread::native_handle_type m_native_handle;
};

}}}

#endif //DIPLOMA_GC_THREAD_DESCRIPTOR_HPP
