#include <gtest/gtest.h>

#include <liballocgc/details/collectors/pin_set.hpp>

using namespace allocgc;
using namespace allocgc::details;
using namespace allocgc::details::collectors;

TEST(pin_set_test, test_insert)
{
    pin_set rs;

    byte* p1;
    byte* p2;

    rs.register_pin(p1);
    ASSERT_TRUE(rs.contains(p1));

    rs.register_pin(p2);
    ASSERT_TRUE(rs.contains(p1));
    ASSERT_TRUE(rs.contains(p2));
}

TEST(pin_set_test, test_remove)
{
    pin_set rs;

    byte a, b, c;

    byte* p1 = &a;
    byte* p2 = &b;
    byte* p3 = &c;

    rs.register_pin(p1);
    rs.register_pin(p2);
    rs.register_pin(p3);

    rs.deregister_pin(p3);
    ASSERT_FALSE(rs.contains(p3));

    rs.deregister_pin(p1);
    ASSERT_FALSE(rs.contains(p1));

    ASSERT_TRUE(rs.contains(p2));
}
