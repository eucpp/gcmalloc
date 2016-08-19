#include <nonius/nonius.h++>//

#include <vector>

#include <libprecisegc/details/threads/stack_map.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>

#include "deoptimize.hpp"

using namespace precisegc::details::threads;

NONIUS_BENCHMARK("stack_map.insert", [](nonius::chronometer meter)
{
    stack_map<void*> map;
    meter.measure([&map] {
        void* p;
        map.insert(p);
    });
});

NONIUS_BENCHMARK("stack_map.remove", [](nonius::chronometer meter)
{
    stack_map<void*> map;
    std::vector<void*> ps(meter.runs());
    size_t size = ps.size();
    for (auto p: ps) {
        map.insert(p);
    }
    meter.measure([&map, &ps, size] (size_t i) {
        map.remove(ps[size - 1 - i]);
        escape(&map);
    });
});
