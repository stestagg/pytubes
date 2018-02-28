#include <catch2.hpp>

#include "count.hpp"
#include "first.hpp"

using namespace ss::iter;

TEST_CASE( "Count first", "[count]" ) {

    AnyIter it1 = std::shared_ptr<Iter>(new CountIter(0));
    FirstIter it2(it1, 2);
    const int64_t *value_ptr = it2.get_slots()[0];
    it1->next();
    it2.next();
    REQUIRE(*value_ptr == 0);
    it1->next();
    it2.next();
    REQUIRE(*value_ptr == 1);
    REQUIRE_THROWS_AS(it2.next(), ss::StopIterationExc);

}
