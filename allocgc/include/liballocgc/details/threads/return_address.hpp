#ifndef ALLOCGC_RETURN_ADDRESS_HPP
#define ALLOCGC_RETURN_ADDRESS_HPP

#include <sys/resource.h>
#include <pthread.h>

#include <liballocgc/gc_common.hpp>
#include <liballocgc/details/constants.hpp>

namespace allocgc { namespace details { namespace threads {

inline byte* return_address()
{
    return reinterpret_cast<byte*>(__builtin_return_address(0));
}

inline byte* frame_address()
{
    return reinterpret_cast<byte*>(__builtin_frame_address(0));
}

inline byte* get_stack_addr(pthread_t thread)
{
    pthread_attr_t attr;
    byte* stackaddr;
    size_t stacksize;
    pthread_getattr_np(pthread_self(), &attr);
    pthread_attr_getstack(&attr, (void**)&stackaddr, &stacksize);
    return stackaddr;
}

inline size_t get_stack_size(pthread_t thread)
{
    pthread_attr_t attr;
    byte* stackaddr;
    size_t stacksize;
    pthread_getattr_np(pthread_self(), &attr);
    pthread_attr_getstack(&attr, (void**)&stackaddr, &stacksize);
    return stacksize;
}

inline size_t stack_maxsize()
{
    rlimit limit;
    getrlimit (RLIMIT_STACK, &limit);
    return limit.rlim_cur;
}

}}}

#endif //ALLOCGC_RETURN_ADDRESS_HPP
