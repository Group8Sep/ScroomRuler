#define BOOST_TEST_MODULE Ruler tests
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include "../src/ruler.hh"

BOOST_AUTO_TEST_SUITE(Ruler_Tests)

    BOOST_AUTO_TEST_CASE(Ruler_dummy_test)
    {
        BOOST_CHECK(1 == 1);
    }

BOOST_AUTO_TEST_SUITE_END()