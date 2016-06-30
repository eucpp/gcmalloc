#ifndef DIPLOMA_SIGNAL_HPP
#define DIPLOMA_SIGNAL_HPP

#include <array>
#include <bitset>
#include <boost/optional.hpp>
#include <signal.h>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/threads/pending_call.hpp>

namespace precisegc { namespace details { namespace threads {

extern "C" {
void sighandler(int signum);
}

class posix_signal : private utils::noncopyable, private utils::nonmovable
{
public:
    typedef pending_call::callable_type handler_type;

    static posix_signal& instance();

    handler_type get_handler() const;
    void set_handler(handler_type handler);

    void send(pthread_t thread);

    static void lock();
    static void unlock();

    static void enter_safe_scope();
    static void leave_safe_scope();

    static bool is_locked();

    friend void call_signal_handler();
    friend void ::precisegc::details::threads::sighandler(int);
private:
    static const int SIGNUM = SIGUSR2;

    posix_signal();

    handler_type m_handler;
};

}}}

#endif //DIPLOMA_SIGNAL_HPP
