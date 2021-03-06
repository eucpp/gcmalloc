#include <liballocgc/gc.hpp>

namespace allocgc {

using namespace details;
using namespace details::threads;
using namespace details::collectors;

gc_handle gc_handle::null{nullptr};

void enable_logging(gc_loglevel loglevel)
{
    logging::set_loglevel(loglevel);
}

namespace serial {

void gc()
{
    gc_options opt;
    opt.kind = gc_kind::COLLECT;
    opt.gen  = -1;
    gc_facade<gc_serial>::initiation_point(details::initiation_point_type::USER_REQUEST, opt);
}

gc_stat stats()
{
    return gc_facade<gc_serial>::stats();
}

void set_heap_limit(size_t limit)
{
    gc_facade<gc_serial>::set_heap_limit(limit);
}

void set_threads_available(size_t threads_available)
{
    gc_facade<gc_serial>::set_threads_available(threads_available);
}

void register_main_thread()
{
    thread_descriptor main_thrd_descr;
    main_thrd_descr.id = std::this_thread::get_id();
    main_thrd_descr.native_handle = threads::this_thread_native_handle();
    main_thrd_descr.stack_addr = threads::get_stack_addr(threads::this_thread_native_handle());
    main_thrd_descr.stack_size = threads::get_stack_size(threads::this_thread_native_handle());

//    std::cout << "stack_start: " << (void*) main_thrd_descr.stack_start_addr << std::endl;
//    std::cout << "return address: " << (void*) threads::return_address() << std::endl;
//    std::cout << "frame address: " << (void*) threads::frame_address() << std::endl;

    register_thread(main_thrd_descr);
}

void register_thread(const thread_descriptor& descr)
{
    gc_facade<gc_serial>::register_thread(descr);
}

void deregister_thread(std::thread::id id)
{
    gc_facade<gc_serial>::deregister_thread(id);
}

}

namespace cms {

void gc()
{
    gc_options opt;
    opt.kind = gc_kind::COLLECT;
    opt.gen  = -1;
    gc_facade<gc_cms>::initiation_point(details::initiation_point_type::USER_REQUEST, opt);
}

gc_stat stats()
{
    return gc_facade<gc_cms>::stats();
}

void set_heap_limit(size_t limit)
{
    gc_facade<gc_cms>::set_heap_limit(limit);
}

void set_threads_available(size_t threads_available)
{
    gc_facade<gc_cms>::set_threads_available(threads_available);
}

void register_main_thread()
{
    thread_descriptor main_thrd_descr;
    main_thrd_descr.id = std::this_thread::get_id();
    main_thrd_descr.native_handle = threads::this_thread_native_handle();
    main_thrd_descr.stack_addr = threads::get_stack_addr(threads::this_thread_native_handle());
    main_thrd_descr.stack_size = threads::get_stack_size(threads::this_thread_native_handle());
    register_thread(main_thrd_descr);
}

void register_thread(const thread_descriptor& descr)
{
    gc_facade<gc_cms>::register_thread(descr);
}

void deregister_thread(std::thread::id id)
{
    gc_facade<gc_cms>::deregister_thread(id);
}

}

}

