#ifndef DIPLOMA_SERIAL_GC_MOCK_HPP
#define DIPLOMA_SERIAL_GC_MOCK_HPP

#include <gmock/gmock.h>

#include <libprecisegc/gc_strategy.hpp>

//class serial_gc_mock : public precisegc::details::gc_strategy
//{
//    typedef precisegc::details::byte byte;
//    typedef precisegc::details::gc_cell gc_cell;
//    typedef precisegc::details::gc_handle gc_handle;
//    typedef precisegc::details::gc_type_meta gc_type_meta;
//    typedef precisegc::details::allocators::gc_alloc_response gc_alloc_response;
//    typedef precisegc::details::initiation_point_type initation_point_type;
//    typedef precisegc::details::gc_options gc_options;
//    typedef precisegc::details::gc_run_stats gc_stats;
//    typedef precisegc::details::gc_phase gc_phase;
//    typedef precisegc::details::gc_info gc_info;
//public:
//    MOCK_METHOD3(allocate, gc_alloc_response (size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta));
//
//    MOCK_METHOD1(commit, void(gc_cell&));
//    MOCK_METHOD2(commit, void(gc_cell&, const gc_type_meta*));
//
//    MOCK_METHOD1(rbarrier, byte*(const gc_handle&));
//    MOCK_METHOD2(wbarrier, void(gc_handle&, const gc_handle&));
//
//    MOCK_METHOD2(interior_wbarrier, void(gc_handle& handle, ptrdiff_t offset));
//
//    MOCK_METHOD1(pin, byte*(const gc_handle& handle));
//    MOCK_METHOD1(unpin, void(byte* ptr));
//
//    MOCK_METHOD2(compare, bool(const gc_handle& a, const gc_handle& b));
//
//    MOCK_METHOD1(gc, gc_stats(const gc_options&));
//
//    gc_info info() const override
//    {
//        static gc_info inf = {
//                .incremental_flag                = false,
//                .support_concurrent_marking      = false,
//                .support_concurrent_collecting   = false
//        };
//        return inf;
//    }
//};

#endif //DIPLOMA_SERIAL_GC_MOCK_HPP
