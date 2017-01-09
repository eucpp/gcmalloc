#include "../../common/macro.hpp"
#include "../../common/timer.hpp"

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <iostream>
#include <memory>

#ifdef PRECISE_GC
    #include "libprecisegc/libprecisegc.hpp"
    using namespace precisegc;
#endif

#ifdef BDW_GC
    #include <gc/gc.h>
#endif

#include "cord.hpp"

std::unique_ptr<char[]> str;

void init(size_t buf_size)
{
    srand(time(nullptr));
    str.reset(new char[buf_size + 1]);
    int mod = 'z' - 'a';
    for (size_t i = 0; i < buf_size; ++i) {
        str[i] = (rand() % mod) + 'a';
    }
    str[buf_size] = '\0';
}

void build_rope(size_t total_len, size_t buf_size)
{
    CORD cord = CORD_EMPTY;
    for (size_t len = 0; len < total_len; len += buf_size) {
        ptr_array_t(const char) buf = new_array_(const char, buf_size + 1);
        pin_array_t(const char) buf_pin = pin(buf);
        const char* buf_raw = raw_ptr(buf_pin);
        memcpy((void*) buf_raw, (void*) str.get(), buf_size + 1);

        cord = CORD_cat_char_star(cord, buf, buf_size);
    }
}

int main (int argc, const char* argv[])
{
    size_t len = 0;
    size_t buf_size = 127;
    bool incremental_flag = false;
    for (int i = 1; i < argc; ++i) {
        auto arg = std::string(argv[i]);
        if (arg == "--incremental") {
            incremental_flag = true;
        } else if (arg == "--len") {
            assert(i + 1 < argc);
            ++i;
            std::string len_str = std::string(argv[i]);
            len = std::stoi(len_str);
        }
    }

    init(buf_size);

    #if defined(PRECISE_GC)
        gc_init_options ops;
//        ops.heapsize    = 32 * 1024 * 1024;      // 32 Mb
        ops.algo        = incremental_flag ? gc_algo::INCREMENTAL : gc_algo::SERIAL;
        ops.initiation  = gc_initiation::SPACE_BASED;
        ops.compacting  = gc_compacting::DISABLED;
//        ops.compacting  = compacting_flag ? gc_compacting::ENABLED : gc_compacting::DISABLED;
        ops.loglevel    = gc_loglevel::SILENT;
//            ops.print_stat  = true;
    //        ops.threads_available = 1;
        gc_init(ops);
    #elif defined(BDW_GC)
        GC_INIT();
        if (incremental_flag) {
            GC_enable_incremental();
        }
    #endif

    std::cout << "Building a rope with total length = " << len << std::endl;

    timer tm;
    build_rope(len, buf_size);
    std::cout << "Completed in " << tm.elapsed<std::chrono::milliseconds>() << " ms" << std::endl;

    #if defined(PRECISE_GC)
        gc_stat stat = gc_stats();
        std::cout << "Completed " << stat.gc_count << " collections" << std::endl;
        std::cout << "Time spent in gc " << std::chrono::duration_cast<std::chrono::milliseconds>(stat.gc_time).count() << " ms" << std::endl;
        std::cout << "Average pause time " << std::chrono::duration_cast<std::chrono::microseconds>(stat.gc_time / stat.gc_count).count() << " us" << std::endl;
    #elif defined(BDW_GC)
        std::cout << "Completed " << GC_get_gc_no() << " collections" << std::endl;
        std::cout << "Heap size is " << GC_get_heap_size() << std::endl;
    #endif
}

