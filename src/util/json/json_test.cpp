#include <catch2.hpp>

#include <limits>

#include "json.hpp"
#include "token.hpp"

#include <iostream>

using namespace ss;
using namespace ss::json;
using namespace ss::json::parse;

inline ByteSlice from_cstr(const char *str) {
    return ByteSlice((uint8_t *)str, strlen(str));
}

inline std::string to_str(Slice<uint8_t> src) {
    return std::string((char *)src.start, src.len);
}


TEST_CASE( "test convert to int", "[json]" ) {
    const char *samples[] = {"1", "65535", "9223372036854775807", "0", "-0", "-9223372036854775807"};
    const int64_t expecteds[] = {1, 65535, 9223372036854775807, 0, -0, -9223372036854775807};
    size_t index = 0;
    for (auto sample: samples) {
        auto expected = expecteds[index++];
        auto tok = tokenize<uint8_t>(from_cstr(sample));
        int64_t actual = parse::OptimisticParser<uint8_t>::parse_int(tok);
        REQUIRE( actual == expected );
    }
}

TEST_CASE( "test parse int", "[json]" ) {
    const char *bads[] = {"1.1", "1.", "1e1"};
    for (auto bad: bads) {
        auto tok = tokenize<uint8_t>(from_cstr(bad));
        std::string msg = std::string("Number '") + bad + "' could not be interpreted as integer";
        REQUIRE_THROWS_WITH(
            OptimisticParser<uint8_t>::parse_int(tok),
            msg.c_str()
        );
    }
}

TEST_CASE( "test parse double", "[json]" ) {
    const char *samples[] = {"1", "1.00001", "-1.", "1e10", "1e999"};
    const double expecteds[] = {1, 1.00001, -1, 1e10, std::numeric_limits<double>::infinity()};
    size_t index = 0;
    for (auto sample: samples) {
        auto expected = expecteds[index++];
        auto tok = tokenize<uint8_t>(from_cstr(sample));
        REQUIRE( tok.type == Type::Number );
        double val = OptimisticParser<uint8_t>::parse_double(tok);
        REQUIRE( val == expected );
    }
}

TEST_CASE( "test invalid double conversion", "[json]" ) {
    const char *bads[] = {"1ee", "-e", "1.2.3.4"};
    for (auto bad: bads) {
        auto tok = tokenize<uint8_t>(from_cstr(bad));
        REQUIRE_THROWS(OptimisticParser<uint8_t>::parse_double(tok));
        PyErr_Clear();
    }
}

TEST_CASE( "test string conversion", "[json]" ) {
    const char *samples[] = {
        "hi",
        "hi\\tho",
        u8"私は",
        "\\u03B3\\u03BB\\u03CE\\u03C3\\u03C3\\u03B1",
        "\\uD801\\uDC37"
    };
    const char *expecteds[] = {"hi", "hi\tho", u8"私は", u8"γλώσσα", u8"𐐷"};
    size_t index = 0;
    for (auto sample: samples) {
        auto input = std::string("\"") + sample + std::string("\"");
        auto expected = expecteds[index++];
        auto tok = tokenize<uint8_t>(from_cstr(input.c_str()));
        REQUIRE( tok.type == Type::String );
        std::basic_string<uint8_t> buffer;
        auto actual = OptimisticParser<uint8_t>::parse_string(tok,  buffer);
        REQUIRE( to_str(actual) == expected );
    }
}

TEST_CASE( "test ascii string optimization", "[json]" ) {
    const char *ascii = "\"hi\"";
    std::basic_string<uint8_t> buffer;
    auto tok = tokenize<uint8_t>(from_cstr(ascii));
    auto ascii_slice = OptimisticParser<uint8_t>::parse_string(tok, buffer);
    REQUIRE( ascii_slice.start == (uint8_t*)&ascii[1] );
    REQUIRE( to_str(ascii_slice) == "hi" );
}


TEST_CASE( "test escaped string isnt optimized", "[json]" ) {
    const char *ascii = "\"hi\\tho\"";
    std::basic_string<uint8_t> buffer;
    auto tok = tokenize<uint8_t>(from_cstr(ascii));
    auto ascii_slice = OptimisticParser<uint8_t>::parse_string(tok, buffer);
    REQUIRE( ascii_slice.start == (uint8_t*)buffer.c_str());
    REQUIRE( to_str(ascii_slice) == "hi\tho" );
}

TEST_CASE( "test invalid string conversion", "[json]" ) {
    const char *bads[] = {"\"\\u123\"", "\"\\u12\"", "\"\\u123g\""};
    for (auto bad: bads) {
        auto tok = tokenize<uint8_t>(from_cstr(bad));
        std::basic_string<uint8_t> buffer;
        REQUIRE_THROWS(OptimisticParser<uint8_t>::parse_string(tok, buffer));
    }
}


TEST_CASE( "test array iteration", "[json]" ) {
    auto tok = tokenize<uint8_t>(from_cstr("[1,2,3,4,5]"));
    std::vector<int64_t> results({});
    const std::vector<int64_t> expected({1, 2, 3, 4, 5});
    for (auto child: OptimisticParser<uint8_t>::parse_array(tok)) {
        REQUIRE( child.type == Type::Number);
        results.push_back(OptimisticParser<uint8_t>::parse_int(child));
    }
    REQUIRE(results == expected );
}

TEST_CASE( "test empty array iteration", "[json]" ) {
    auto tok = tokenize<uint8_t>(from_cstr("[]"));
    for (auto child: OptimisticParser<uint8_t>::parse_array(tok)) {
        REQUIRE(false);
        REQUIRE( child.type == Type::Number); // To make child be used
    }
}

TEST_CASE( "test object iteration", "[json]" ) {
    auto tok = tokenize<uint8_t>(from_cstr("{\"a\": 1, \"b\": 2}"));
    std::vector<std::pair<Slice<uint8_t>, int64_t>> expecteds({
        {from_cstr("a"), 1},
        {from_cstr("b"), 2},
    });
    std::basic_string<uint8_t> buffer;
    size_t index = 0;
    for (auto child: OptimisticParser<uint8_t>::parse_object(tok)) {
        auto expected = expecteds[index++];
        auto key_str = OptimisticParser<uint8_t>::parse_string(child.first, buffer);
        auto value = OptimisticParser<uint8_t>::parse_int(child.second);
        REQUIRE( to_str(key_str) == to_str(expected.first));
        REQUIRE(value == expected.second);
    }
}