#ifndef DIPLOMA_FORWARDING_HPP
#define DIPLOMA_FORWARDING_HPP

#include <cstddef>

#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace compacting {

class forwarding : private utils::noncopyable, private utils::nonmovable
{
public:
    forwarding() = default;

    void create(byte* from, byte* to, size_t size);

    void forward(gc_handle* handle);
private:
    static void move_cell(byte* from, byte* to, size_t size);
};

}}}

#endif //DIPLOMA_FORWARDING_HPP
