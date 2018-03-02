#include <catch2.hpp>

#include <numeric>

#include "slice.hpp"

using namespace ss;

TEST_CASE( "Default constructed slices are empty", "[slice]" ) {
    Slice<uint8_t> slice = Slice<uint8_t>();

    REQUIRE( slice.len == 0 );
    REQUIRE( slice.is_empty() == true );
    REQUIRE( slice.is_null() == true );
    REQUIRE( slice.start == &slice::empty_array[0] );
}

TEST_CASE( "Slices can be constructed from buffers", "[slice]" ) {
    const char *data = "test\0me";
    REQUIRE( Slice<char>(data, 4).len == 4 );
    REQUIRE( Slice<char>(data, 4).start == data );
}

TEST_CASE( "Slices can be constructed from iterators", "[slice]" ) {
    const char *data = "test\0me";
    REQUIRE( Slice<char>(data, &data[2], true).len == 2 );
    REQUIRE( Slice<char>(&data[2], &data[4], true).start == &data[2] );
}

TEST_CASE( "Slices can be of non POD types", "[slice]" ) {
    class X{
        void *y;
    };
    Slice<X> foo;
    REQUIRE( foo.len == 0 );
    REQUIRE( foo.is_null() );
}


TEST_CASE( "Slices are std containers", "[slice]" ) {
    const char *data = "test\0me";
    auto test = Slice<char>(data, 4);
    auto me = Slice<char>(&data[5], 2);
    auto all = Slice<char>(data, 7);

    REQUIRE( std::string(test.begin(), test.end()) == "test" );
    REQUIRE( std::string(me.begin(), me.end()) == "me" );
    REQUIRE( std::string(all.begin(), all.end()).length() == 7 );

    auto counter = Slice<char>("\0\1\2\3", 4);
    REQUIRE( std::accumulate(counter.begin(), counter.end(), 0) == 6 );

    int nums[4] = {5, 60, 700, 8000};
    auto count2 = Slice<int>(&nums[0], 4);
    REQUIRE( std::accumulate(count2.begin(), count2.end(), 0) == 8765 );
}

TEST_CASE( "Byte Slices convert to std::string", "[slice]" ) {
    auto slice = Slice<char>("test\0me", 7);
    auto str = std::string(slice);
    REQUIRE( str.length() == 7 );
    REQUIRE( str == std::string(slice.begin(), slice.end()) );

    std::string other = Slice<char>("these", 3);
    REQUIRE( other == "the");

    std::string other2 = std::string("The ") + Slice<char>("cat dog", 4) + std::string("sat on the ") + "mat.";
    REQUIRE (other2 == "The cat sat on the mat.");
}


TEST_CASE( "Slices are views onto buffers", "[slice]" ) {
    char data[7] = "banana";
    auto slice = Slice<char>(data, 6);

    REQUIRE( std::string(slice.begin(), slice.end()) == "banana" );
    data[2] = 'b';
    data[4] = 'b';
    REQUIRE( std::string(slice.begin(), slice.end()) == "bababa" );
}


TEST_CASE( "Zero length slices arent always null", "[slice]" ) {
    const char *data = "test";
    Slice<char> slice = Slice<char>(data, 0);

    REQUIRE( slice.len == 0 );
    REQUIRE( slice.is_empty() == true );
    REQUIRE( slice.is_null() == false );
    REQUIRE( slice.start == data );
}


TEST_CASE( "static_startswith compares elements", "[slice]" ) {
    auto abacus = Slice<char>("abacus", 6);
    
    CHECK( abacus.static_startswith<'a'>() == true );
    CHECK( abacus.static_startswith<'b'>() == false );
    CHECK( abacus.static_startswith<'a', 'b', 'a', 'c', 'u', 's'>() == true );
    CHECK( abacus.static_startswith<'a', 'b', 'a', 'c', 'u', 'b'>() == false );

    CHECK( abacus.static_startswith<'a'>() == true );
    CHECK( abacus.static_startswith<'a', 'b'>() == true );
    CHECK( abacus.static_startswith<'a', 'b', 'c'>() == false );
}

TEST_CASE( "find_first returns pointer to first match", "[slice]" ) {
    auto abacus = Slice<char>("abacus", 6);
    
    CHECK( abacus.find_first('b') == abacus.start+1 );
    CHECK( abacus.find_first('c') == abacus.start+3 );
    CHECK( abacus.find_first('a') == abacus.start );
    CHECK( abacus.find_first('z') == abacus.end() );

    int nums[4] = {5, 60, 700, 8000};
    auto num_slice = Slice<int>(&nums[0], 4);
    CHECK( num_slice.find_first(5) == num_slice.start );
    CHECK( num_slice.find_first(60) == num_slice.start+1 );
    CHECK( num_slice.find_first(61) == num_slice.end() );

}

TEST_CASE( "Slicing a slice using indexes", "[slice]" ) {
    const char *data = "null,end";
    REQUIRE( std::string(Slice<char>(data, 8).slice_to(4)) == std::string("null" ) );
    REQUIRE( std::string(Slice<char>(data, 8).slice_from(4)) == std::string(",end" ) );
}
