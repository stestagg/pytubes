#include <catch2.hpp>

#include "iter.hpp"

using namespace ss;
using namespace ss::iter;

TEST_CASE( "Slot pointer can be default constructed", "[slot ptr]" ) {
    REQUIRE( SlotPointer().type == ScalarType::Null );
}

TEST_CASE( "Slot pointer can be constructed from int64_t pointer", "[slot ptr]" ) {
    int64_t foo = 4;
    auto slot = SlotPointer(ScalarType::Int64, &foo);
    REQUIRE( *(const int64_t*)slot== 4 );
    REQUIRE( slot.type == ScalarType::Int64 );
}

TEST_CASE( "Slot pointer can't be constructed from invalid pointer", "[slot ptr]" ) {
    int64_t foo = 4;
    REQUIRE_THROWS_WITH(
        SlotPointer(ScalarType::Float, &foo),
        "Tried to create Float slot pointer with Int64 pointer type"
    );

}

TEST_CASE( "Cant cast to invalid pointer type", "[slot ptr]" ) {
    int64_t foo = 4;
    auto slot = SlotPointer(ScalarType::Int64, &foo);
    REQUIRE_THROWS_WITH(
        (const double *)slot,
        "Tried to dereference Int64 slot pointer as Float pointer type"
    );

}