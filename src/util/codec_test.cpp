#include <catch2.hpp>

#include <memory>

#include "slice.hpp"
#include "codec.hpp"

using namespace ss;
using namespace ss::codec;

TEST_CASE("Test Codecs", "[codec]") {
    ss::codec::init_codec();


    SECTION ( "UTf8 encoder is transparent") {

        auto slice = ByteSlice("test\0me", 7);

        auto codec = std::string("utf8");
        auto encoder = ss::codec::get_encoder(codec, &slice);
        REQUIRE( encoder->is_transparent() );
    }

    SECTION ( "UTf8 encoder can encode simple strings") {
        auto slice = ByteSlice("Hello World", 11);

        auto codec = std::string("utf8");
        auto encoder = ss::codec::get_encoder(codec, &slice);
        encoder->encode();
        REQUIRE(slice == encoder->to);
    }

    SECTION ( "Latin1 encoder is not transparent") {
        auto slice = ByteSlice("test\0me", 7);

        auto codec = std::string("latin1");
        auto encoder = ss::codec::get_encoder(codec, &slice);
        REQUIRE( !encoder->is_transparent() );
    }

    SECTION ( "Latin1 encoder can encode simple strings") {
        auto slice = ByteSlice("Hello World", 11);

        auto codec = std::string("latin1");
        auto encoder = ss::codec::get_encoder(codec, &slice);
        encoder->encode();
        REQUIRE(slice == encoder->to);
    }

    SECTION ( "Latin1 encoder can encode complex strings") {
    	const uint8_t given[] = {0xfd, 0xe0, 0xfe, 0xfe, 0xff};
        auto slice = ByteSlice(given, 5);

        auto codec = std::string("latin1");
        auto encoder = ss::codec::get_encoder(codec, &slice);
        encoder->encode();
        const uint8_t expected[] = {0xc3, 0xbd, 0xc3, 0xa0, 0xc3, 0xbe, 0xc3, 0xbe, 0xc3, 0xbf};
        REQUIRE(encoder->to == ByteSlice(expected, 10));
    }

    SECTION ( "Latin1 encoder can encode multiple times") {
    	std::vector<ByteSlice> given = {
    		ByteSlice("Hi", 2),
    		ByteSlice("Hello World", 11),
    		ByteSlice("Ho", 2)
    	};

    	ByteSlice slice;

    	auto encoder = ss::codec::get_encoder("latin1", &slice);
    	for (auto src : given) {
    		slice = src;
    		encoder->encode();
    		REQUIRE(encoder->to == src);
    	}
    }
}