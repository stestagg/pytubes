#include <catch2.hpp>

#include "error.hpp"

using namespace ss;
using namespace std::string_literals;

struct Foo{
    inline operator std::string() const { return "foo"s; }
};

TEST_CASE( "make_str concatenates strings", "[error]" ) {
    REQUIRE( make_str(1, 2, 3) == "123"s );
    REQUIRE( make_str("A", "ba"s, 'c', "us") == "Abacus"s );
    REQUIRE( make_str("") == ""s );
    REQUIRE( make_str(Foo()) == "foo"s );
}

TEST_CASE( "throw-if uses condition", "[error]" ) {
        REQUIRE_THROWS_AS([&](){throw_if(ValueError, true, "HI");}(), ValueError);
        REQUIRE_THROWS_WITH([&](){throw_if(ValueError, true, "HI");}(), "HI" );
        REQUIRE_NOTHROW([&](){throw_if(ValueError, false); }(), "");
}