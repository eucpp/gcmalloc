#ifndef DIPLOMA_PENDING_CALL_HPP
#define DIPLOMA_PENDING_CALL_HPP

#include <csignal>

#include <libprecisegc/details/util.h>

namespace precisegc { namespace details { namespace threads {

class pending_call : private noncopyable, private nonmovable
{
public:
    typedef void (*callable_type)(void);

    pending_call(callable_type callable);

    void operator()();

    void lock();
    void unlock();
private:
    static const sig_atomic_t PENDING = 1;
    static const sig_atomic_t NOT_PENDING = 0;

    callable_type m_callable;
    volatile sig_atomic_t m_depth;
    volatile sig_atomic_t m_pending_flag;
};

}}}

#endif //DIPLOMA_PENDING_CALL_HPP
