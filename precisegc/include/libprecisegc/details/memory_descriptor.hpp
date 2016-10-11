#ifndef DIPLOMA_CELL_DESCRIPTOR_H
#define DIPLOMA_CELL_DESCRIPTOR_H

#include <mutex>

#include <libprecisegc/details/types.hpp>
#include "gc_type_meta.hpp"

namespace precisegc { namespace details {

class memory_descriptor
{
public:
    typedef std::recursive_mutex mutex_type;
    typedef std::unique_lock<mutex_type> lock_type;

    virtual ~memory_descriptor() {}

    virtual bool get_mark(byte* ptr) const = 0;
    virtual bool get_pin(byte* ptr) const = 0;

    virtual void set_mark(byte* ptr, bool mark) = 0;
    virtual void set_pin(byte* ptr, bool pin) = 0;

    virtual size_t cell_size() const = 0;
    virtual byte*  cell_start(byte* ptr) const = 0;

    virtual void set_type_meta(byte* ptr, const gc_type_meta* tmeta) = 0;
};

}}

#endif //DIPLOMA_CELL_DESCRIPTOR_H
