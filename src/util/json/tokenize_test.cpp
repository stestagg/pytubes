#include <catch2.hpp>

#include "json.hpp"

using namespace ss;
using namespace ss::json;

inline std::string to_str(Slice<uint8_t> src) {
    return std::string((char *)src.start, src.len);
}

inline ByteSlice from_cstr(const char *str) {
    return ByteSlice((uint8_t*)str, strlen(str));
}

TEST_CASE( "test tokenize null", "[json]" ) {
    REQUIRE( tokenize<uint8_t>(from_cstr("null")).type == Type::Null );
    REQUIRE( tokenize<uint8_t>(from_cstr("nule")).type == Type::Null );
    REQUIRE( tokenize<uint8_t>(from_cstr("nullover")).type == Type::Null );
}

TEST_CASE( "test tokenize false", "[json]" ) {
    const char *samples[] = {"false", "falsy", "fxxxx"};
    for (auto sample: samples) {
        auto tok = tokenize<uint8_t>(from_cstr(sample));
        REQUIRE( tok.type == Type::Bool );
    }

    REQUIRE_THROWS_WITH(
        tokenize<uint8_t>(ByteSlice((uint8_t*)"false", 2)),
        "Expected false, found 'fa'"
    );
    REQUIRE( tokenize<uint8_t>(from_cstr("falseove")).slice.len == 5 );
}

TEST_CASE( "test tokenize true", "[json]" ) {
    const char *samples[] = {"false", "trux", "txxx"};
    for (auto sample: samples) {
        auto tok = tokenize<uint8_t>(from_cstr(sample));
        REQUIRE( tok.type == Type::Bool );
    }
    REQUIRE_THROWS_WITH(
        tokenize<uint8_t>(ByteSlice((uint8_t*)"true", 2)),
        "Expected true, found 'tr'"
    );
}

TEST_CASE( "test tokenize string", "[json]" ) {
    auto hi = tokenize(from_cstr("\"hi\","));

    CHECK( hi.slice.len == 2 );
    REQUIRE( to_str(hi.slice) == "hi" );

    auto quote = tokenize(from_cstr("\"\\\"\""));
    CHECK( quote.slice.len == 2 );
    REQUIRE( to_str(quote.slice) == "\\\"" );

    auto empty = tokenize(from_cstr("\"\""));
    CHECK( empty.slice.len == 0 );
    REQUIRE( to_str(empty.slice) == "" );
}

TEST_CASE( "test tokenize invalid string", "[json]" ) {
    REQUIRE_THROWS_WITH(
        tokenize<uint8_t>(from_cstr("\"hi")),
        "Unterminated string: '\"hi'"
    );
    REQUIRE_THROWS_WITH(
        tokenize<uint8_t>(from_cstr("\"")),
        "Unterminated string: '\"'"
    );
}


TEST_CASE( "test tokenize object", "[json]" ) {
    auto empty = tokenize<uint8_t>(from_cstr("{}"));
    REQUIRE( to_str(empty.slice) == "");

    auto nested = tokenize<uint8_t>(from_cstr("{{}}"));
    REQUIRE( to_str(nested.slice) == "{}" );

    auto embedded = tokenize<uint8_t>(from_cstr("{\"\": \"}\"}"));
    REQUIRE( to_str(embedded.slice) == "\"\": \"}\"" );
}


TEST_CASE( "test tokenizing arrays", "[json]" ) {
    const char *samples[] = {"[]", "[1,2,3,4]", "[[], [[]], [[[]]]]"};
    for (auto sample: samples) {
        auto val = tokenize<uint8_t>(from_cstr(sample));
        REQUIRE( val.slice.len == strlen(sample) - 2 );
    }
}

TEST_CASE( "test tokenize array numbers", "[json]" ) {
    auto simple = tokenize<uint8_t>(from_cstr("[1,2,3,4]"));
    REQUIRE( simple.type == Type::Array );
    REQUIRE( to_str(simple.slice) == "1,2,3,4" );
}

TEST_CASE( "test tokenize array nested", "[json]" ) {
    auto nested = tokenize<uint8_t>(from_cstr("[[],[[]],null,\"]\"]"));
    REQUIRE( nested.type == Type::Array );
    REQUIRE( to_str(nested.slice) == "[],[[]],null,\"]\"" );
}


TEST_CASE( "test tokenize num", "[json]" ) {
    const char *samples[] = {"1", "10.5", "-1", "1e156", "-0.5e55"};
    for (auto sample: samples) {
        auto val = tokenize<uint8_t>(from_cstr(sample));
        REQUIRE( val.type == Type::Number );
        REQUIRE( to_str(val.slice) == std::string(sample));
    }
}

TEST_CASE( "test tokenize tricky string", "[json]" ) {
    auto val = tokenize<uint8_t>(from_cstr("\"\\uDFAA\""));
    REQUIRE(val.type == Type::String );
    REQUIRE(to_str(val.slice) == std::string("\\uDFAA"));
}

TEST_CASE( "test invalid json", "[json]" ) {
    const char *samples[] = {"z"};
    for (auto sample: samples) {
        REQUIRE_THROWS_WITH(
            tokenize<uint8_t>(from_cstr(sample)),
            "Invalid json: 'z'"
        );

    }

}

