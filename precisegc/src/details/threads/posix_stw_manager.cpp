#include <libprecisegc/details/threads/stw_manager.hpp>

#include <libprecisegc/details/logging.h>
#include <libprecisegc/details/threads/posix_signal.hpp>
#include <libprecisegc/details/threads/posix_thread.hpp>

namespace precisegc { namespace details { namespace threads {

void stw_manager::sighandler()
{
    stw_manager& stwm = stw_manager::instance();

    logging::debug() << "Thread " << std::this_thread::get_id() << " enters stw signal handler";

    ++stwm.m_threads_suspended_cnt;
    stwm.m_barrier.notify();

    stwm.m_event.wait();
    --stwm.m_threads_suspended_cnt;
    stwm.m_barrier.notify();

    logging::debug() << "Thread " << std::this_thread::get_id() << " leaves stw signal handler";
}

stw_manager& stw_manager::instance()
{
    static stw_manager stwm;
    return stwm;
}

stw_manager::stw_manager()
    : m_threads_cnt(0)
    , m_threads_suspended_cnt(0)
{
    posix_signal::instance().set_handler(stw_manager::sighandler);
}

bool stw_manager::is_stop_the_world_disabled() const
{
    return posix_signal::instance().is_locked();
}

void stw_manager::suspend_thread(std::thread::native_handle_type thread)
{
    logging::debug() << "Sending stw signal to thread " << std::this_thread::get_id();

    ++m_threads_cnt;
    posix_signal::instance().send(thread);
}

void stw_manager::resume_thread(std::thread::native_handle_type thread)
{
    return;
}

void stw_manager::wait_for_world_stop()
{
    m_barrier.wait(m_threads_cnt);
}

void stw_manager::wait_for_world_start()
{
    m_event.notify(m_threads_cnt);
    m_barrier.wait(m_threads_cnt);
    m_threads_cnt = 0;
}

size_t stw_manager::threads_suspended() const
{
    return m_threads_suspended_cnt;
}

}}}
