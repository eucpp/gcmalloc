#include <gtest/gtest.h>

#include <cassert>
#include <mutex>

#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/bucket_allocator.hpp>
#include <libprecisegc/details/allocators/page_allocator.hpp>
#include <libprecisegc/details/allocators/debug_layer.hpp>
#include <libprecisegc/details/types.hpp>

#include "test_chunk.h"

using namespace precisegc::details;
using namespace precisegc::details::allocators;

namespace {

static const size_t OBJ_SIZE_1 = sizeof(size_t);
static const size_t OBJ_SIZE_2 = 2 * sizeof(size_t);

struct test_bucket_policy
{
    static const size_t BUCKET_COUNT = 2;

    static size_t bucket(size_t size)
    {
        if (size <= OBJ_SIZE_1) {
            return 0;
        } else if (size <= OBJ_SIZE_2) {
            return 1;
        }
        assert(false);
    }

    static size_t bucket_size(size_t ind)
    {
        if (ind == 0) {
            return OBJ_SIZE_1;
        } else if (ind == 1) {
            return OBJ_SIZE_2;
        }
        assert(false);
    }
};

}

class bucket_allocator_test : public ::testing::Test
{
public:
//    typedef debug_layer<page_allocator> allocator_t;
    typedef bucket_allocator<test_chunk, default_allocator, default_allocator, test_bucket_policy, std::mutex> bucket_allocator_t;

    bucket_allocator_t m_alloc;
};

TEST_F(bucket_allocator_test, test_allocate_1)
{
    size_t* ptr = (size_t*) (m_alloc.allocate(OBJ_SIZE_1));
    ASSERT_NE(nullptr, ptr);
    *ptr = 42;

    m_alloc.deallocate((byte*) ptr, OBJ_SIZE_1);
}

TEST_F(bucket_allocator_test, test_allocate_2)
{
    size_t* ptr1 = (size_t*) (m_alloc.allocate(OBJ_SIZE_1));
    size_t* ptr2 = (size_t*) (m_alloc.allocate(OBJ_SIZE_2));
    ASSERT_NE(nullptr, ptr2);
    ASSERT_NE(ptr1, ptr2);
    *ptr2 = 42;

    m_alloc.deallocate((byte*) ptr1, OBJ_SIZE_1);
    m_alloc.deallocate((byte*) ptr2, OBJ_SIZE_2);
}

TEST_F(bucket_allocator_test, test_allocate_3)
{
    for (size_t size = 1; size <= OBJ_SIZE_1; ++size) {
        size_t* ptr = (size_t*) (m_alloc.allocate(OBJ_SIZE_1));
        ASSERT_NE(nullptr, ptr);
        *ptr = 42;
        m_alloc.deallocate((byte*) ptr, OBJ_SIZE_1);
    }

    for (size_t size = OBJ_SIZE_1 + 1; size <= OBJ_SIZE_2; ++size) {
        size_t* ptr = (size_t*) (m_alloc.allocate(OBJ_SIZE_2));
        ASSERT_NE(nullptr, ptr);
        ptr[0] = 42;
        ptr[1] = 42;
        m_alloc.deallocate((byte*) ptr, OBJ_SIZE_2);
    }
}

//TEST_F(bucket_allocator_test, test_allocate_4)
//{
//    byte* ptr1 = m_alloc.allocate(OBJ_SIZE_1);
//    byte* ptr2 = m_alloc.allocate(OBJ_SIZE_2);
//    m_alloc.deallocate(ptr1, OBJ_SIZE_1);
//    m_alloc.deallocate(ptr2, OBJ_SIZE_2);
//
//    ASSERT_EQ(0, m_alloc.const_upstream_allocator().get_allocated_mem_size());
//}