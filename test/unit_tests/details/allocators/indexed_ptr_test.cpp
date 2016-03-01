#include <gtest/gtest.h>

#include <memory>

#include "libprecisegc/details/allocators/indexed_ptr.h"
#include "libprecisegc/details/allocators/paged_allocator.h"
#include "libprecisegc/details/allocators/constants.h"

using namespace precisegc::details;
using namespace precisegc::details::allocators;

namespace {
typedef indexed_ptr<byte, int, paged_allocator> indexed_ptr_t;
}

struct indexed_ptr_test : public ::testing::Test
{
    indexed_ptr_test()
        : m_mem(m_alloc.allocate(PAGE_SIZE))
    {}

    ~indexed_ptr_test()
    {
        m_alloc.deallocate(m_mem, PAGE_SIZE);
    }

    paged_allocator m_alloc;
    byte* m_mem;
};

TEST_F(indexed_ptr_test, test_construct)
{
    indexed_ptr_t ptr1;
    ASSERT_EQ(nullptr, ptr1.get_wrapped());
    ASSERT_EQ(nullptr, ptr1.get_indexed_entry());

    indexed_ptr_t ptr2(m_mem);
    ASSERT_EQ(m_mem, ptr2.get_wrapped());
    ASSERT_EQ(nullptr, ptr2.get_indexed_entry());
}

TEST_F(indexed_ptr_test, test_index)
{
    int val = 0;
    indexed_ptr_t ptr = indexed_ptr_t::index(m_mem, PAGE_SIZE, &val);

    EXPECT_EQ(m_mem, ptr.get_wrapped());
    EXPECT_EQ(&val, ptr.get_indexed_entry());

    indexed_ptr_t::remove_index(ptr, PAGE_SIZE);
}

TEST_F(indexed_ptr_test, test_remove_index)
{
    int val = 0;
    indexed_ptr_t ptr = indexed_ptr_t::index(m_mem, PAGE_SIZE, &val);
    indexed_ptr_t::remove_index(ptr, PAGE_SIZE);

    EXPECT_EQ(m_mem, ptr.get_wrapped());
    EXPECT_EQ(nullptr, ptr.get_indexed_entry());
}