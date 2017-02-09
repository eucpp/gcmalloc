#ifndef DIPLOMA_CELL_DESCRIPTOR_H
#define DIPLOMA_CELL_DESCRIPTOR_H

#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/gc_type_meta.hpp>
#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/gc_common.hpp>

namespace precisegc { namespace details {

class gc_memory_descriptor
{
public:
    virtual ~gc_memory_descriptor() {}

    virtual bool get_mark(byte* ptr) const = 0;
    virtual bool get_pin(byte* ptr) const = 0;

    virtual void set_mark(byte* ptr, bool mark) = 0;
    virtual void set_pin(byte* ptr, bool pin) = 0;

    virtual gc_lifetime_tag get_lifetime_tag(byte* ptr) const = 0;

    virtual size_t cell_size(byte* ptr) const = 0;
    virtual byte*  cell_start(byte* ptr) const = 0;

    virtual size_t object_count(byte* ptr) const = 0;

    virtual const gc_type_meta* get_type_meta(byte* ptr) const = 0;

    virtual void commit(byte* ptr) = 0;
    virtual void commit(byte* ptr, const gc_type_meta* type_meta) = 0;

    virtual void trace(byte* ptr, const gc_trace_callback& cb) const = 0;
    virtual void move(byte* to, byte* from, gc_memory_descriptor* from_descr) = 0;
    virtual void finalize(byte* ptr) = 0;
};

}}

#endif //DIPLOMA_CELL_DESCRIPTOR_H
