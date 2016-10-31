#ifndef DIPLOMA_CELL_DESCRIPTOR_H
#define DIPLOMA_CELL_DESCRIPTOR_H

#include <mutex>

#include <libprecisegc/details/types.hpp>
#include "gc_type_meta.hpp"

namespace precisegc { namespace details {

class memory_descriptor
{
public:
    virtual ~memory_descriptor() {}

    virtual void set_initialized(byte* ptr) = 0;
    virtual bool is_initialized(byte* ptr) const = 0;

    virtual bool get_mark(byte* ptr) const = 0;
    virtual bool get_pin(byte* ptr) const = 0;

    virtual void set_mark(byte* ptr, bool mark) = 0;
    virtual void set_pin(byte* ptr, bool pin) = 0;

    virtual size_t cell_size(byte* ptr) const = 0;
    virtual byte*  cell_start(byte* ptr) const = 0;

    virtual size_t object_count(byte* ptr) const = 0;
    virtual void   set_object_count(byte* ptr, size_t cnt) const = 0;

    virtual const gc_type_meta* get_type_meta(byte* ptr) const = 0;
    virtual void  set_type_meta(byte* ptr, const gc_type_meta* tmeta) = 0;
};

}}

#endif //DIPLOMA_CELL_DESCRIPTOR_H
