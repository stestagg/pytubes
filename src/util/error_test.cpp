#include <catch2.hpp>

#include "error.hpp"

using namespace ss;

struct Foo{
    inline operator std::string() const { return std::string("foo"); }
};

TEST_CASE( "make_str concatenates strings", "[error]" ) {
    REQUIRE( make_str(1, 2, 3) == std::string("123") );
    REQUIRE( make_str("A", std::string("ba"), 'c', "us") == std::string("Abacus") );
    REQUIRE( make_str("") == std::string("") );
    REQUIRE( make_str(Foo()) == std::string("foo") );
}

TEST_CASE( "throw-if uses condition", "[error]" ) {
        REQUIRE_THROWS_AS([&](){throw_if(ValueError, true, "HI");}(), ValueError);
        REQUIRE_THROWS_WITH([&](){throw_if(ValueError, true, "HI");}(), "HI" );
        REQUIRE_NOTHROW([&](){throw_if(ValueError, false); }(), "");
}