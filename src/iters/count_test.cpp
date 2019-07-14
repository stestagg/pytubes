#include <catch2.hpp>

#include "count.hpp"

using namespace ss::iter;


TEST_CASE( "Count iter counts", "[count]" ) {

    CountIter it(0);
    const int64_t *value_ptr = it.get_slots()[0];
    it.next();
    REQUIRE(*value_ptr == 0);
    it.next();
    REQUIRE(*value_ptr == 1);

}


TEST_CASE( "Count iter from negative start", "[count]" ) {

    CountIter it(-100);
    const int64_t *value_ptr = it.get_slots()[0];
    it.next();
    REQUIRE(*value_ptr == -100);
    it.next();
    REQUIRE(*value_ptr == -99);

}