#ifndef DIPLOMA_GC_HANDLE_HPP
#define DIPLOMA_GC_HANDLE_HPP

#include <cstddef>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details {

class gc_handle_access;

class gc_handle : private utils::noncopyable, private utils::nonmovable
{
public:
    class pin_guard : private utils::noncopyable
    {
    public:
        pin_guard(pin_guard&& other);
        ~pin_guard();

        pin_guard& operator=(pin_guard&& other);

        byte* get() const noexcept;

        friend class gc_handle;
    private:
        pin_guard(const gc_handle& handle);

        byte* m_ptr;
    };

    gc_handle();
    gc_handle(byte* ptr);

    byte* rbarrier() const;
    void  wbarrier(const gc_handle& other);

    // reset handle to point to some different location inside same cell that was pointed to before
    void interior_wbarrier(byte* ptr);
    void interior_shift(ptrdiff_t shift);

    pin_guard pin() const;

    void reset();

    bool equal(const gc_handle& other) const;
    bool is_null() const;
private:
    friend class gc_handle_access;

    // these two methods don't invoke read/write barriers,
    // they are used by garbage collector itself
    byte* load(std::memory_order order) const;
    void  store(byte* ptr, std::memory_order order);
    void  fetch_advance(ptrdiff_t n, std::memory_order order);

    atomic_byte_ptr m_ptr;
};

class gc_handle_access
{
public:
    static byte* load(const gc_handle& handle, std::memory_order order);
    static void  store(gc_handle& handle, byte* ptr, std::memory_order order);
    static void  fetch_advance(gc_handle& handle, ptrdiff_t n, std::memory_order order);
};

}}

#endif //DIPLOMA_GC_HANDLE_HPP