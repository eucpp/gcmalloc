#ifndef ALLOCGC_GC_INTERFACE_HPP
#define ALLOCGC_GC_INTERFACE_HPP

#include <cassert>
#include <string>
#include <functional>

#include <liballocgc/gc_exception.hpp>
#include <liballocgc/gc_common.hpp>

namespace allocgc { namespace details {

enum class initiation_point_type {
      USER_REQUEST
    , HEAP_LIMIT_EXCEEDED
    , CONCURRENT_MARKING_FINISHED
    // for debugging
    , START_MARKING
    , START_COLLECTING
};

enum class gc_phase {
      IDLE
    , MARK
    , COLLECT
};

enum class gc_lifetime_tag {
      FREE      = 0
    , GARBAGE   = 1
    , ALLOCATED = 2
    , LIVE      = 3
};

inline gc_lifetime_tag get_lifetime_tag_by_bits(bool mark_bit, bool init_bit)
{
    return static_cast<gc_lifetime_tag>(((int) mark_bit << 1) + (int) init_bit);
}

typedef std::function<void(gc_handle*)> gc_trace_callback;
typedef std::function<void(byte*)> gc_trace_pin_callback;
typedef std::function<void(byte*, size_t)> gc_trace_obj_callback;

class gc_launcher
{
public:
    virtual ~gc_launcher() {}

    virtual gc_runstat gc(const gc_options& opt) = 0;
    virtual gc_info info() const = 0;
};

inline const char* gc_kind_to_str(gc_kind type)
{
    if (type == gc_kind::LAUNCH_CONCURRENT_MARK) {
        return "starting concurrent mark";
    } else if (type == gc_kind::COLLECT) {
        return "collect";
    } else {
        return "undefined";
    }
}

inline std::string duration_to_str(gc_clock::duration duration, int padding = 0)
{
    static const size_t ms = 1000;
    static const size_t  s = 1000 * ms;

    size_t dur_us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

    char str[13];
//    if (dur_us >= s) {
//        snprintf(str, 8, "%*lu s ", padding, dur_us / s);
//    } else if (dur_us >= ms) {
//        snprintf(str, 8, "%*lu ms", padding, dur_us / ms);
//    } else {
//        snprintf(str, 8, "%*lu us", padding, dur_us);
//    }
    snprintf(str, 13, "%*lu us", padding, dur_us);
    return std::string(str);
}

inline std::string heapsize_to_str(size_t size, int padding = 0)
{
    static const size_t Kb = 1024;
    static const size_t Mb = 1024 * Kb;
    static const size_t Gb = 1024 * Mb;

    char str[13];
//    if (size >= Gb) {
//        snprintf(str, 8, "%*lu Gb", padding, size / Gb);
//    } else if (size >= Mb) {
//        snprintf(str, 8, "%*lu Mb", padding, size / Mb);
//    } else if (size >= Kb) {
//        snprintf(str, 8, "%*lu Kb", padding, size / Kb);
//    } else {
//        snprintf(str, 8, "%*lu b ", padding, size);
//    }
    snprintf(str, 13, "%*lu", padding, size);
    return std::string(str);
}

}}

#endif //ALLOCGC_GC_INTERFACE_HPP
