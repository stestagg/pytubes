#include <vector>

#include <catch2.hpp>

#include "stream_reader.hpp"
#include "../bytes.hpp"

using namespace ss;
using namespace ss::iter;

struct SnakeFinder{
    bool currently_upper = true;

    inline const bytes *find_next(ByteSlice &slice) {
        for (auto &cur : slice) {
            if (isupper(cur)) {
                if (!currently_upper) {
                    currently_upper = true;
                    return &cur;
                }
            } else {
                if (currently_upper) {
                    currently_upper = false;
                }
            }
        }
        return slice.end();
    }

    inline ByteSlice get_remaining(const ByteSlice &slice, const bytes* const match) const{
        return slice.slice_from_ptr(match);
    }
};

class StringIter : public Iter {
    using Vec = std::vector<const char *>;
    Vec items;
    Vec::iterator cur_item;

    ByteSlice cur;
    SlotPointer slot;
public:
    StringIter(Vec &items) :
        items(items),
        cur_item(this->items.begin()),
        slot(&cur)
    {}

    Slice<SlotPointer> get_slots(){ return Slice<SlotPointer>(&slot, 1); }

    void next(){
        if(cur_item == items.end()) {
            throw StopIteration;
        }
        auto next_item = *cur_item;
        cur = ByteSlice((const bytes*)next_item, strlen(next_item));
        ++ cur_item;
    }
};

StreamReader<bytes> make_reader(std::vector<const char*> items) {
    auto iter = new StringIter(items);
    auto any = to_any(iter);
    Chain chain(&any, 1);
    return StreamReader<bytes>(chain, iter->get_slots()[0]);
}



TEST_CASE("can split streams", "[stream reader]") {
    // std::vector<const char*> items = {"a", "b", "c", "d"};
    auto data = "abc,def,ghi";
    auto reader = make_reader({data});
    REQUIRE(reader.read_until_char(',').is(ByteSlice(data, 3)));
    REQUIRE(reader.read_until_char(',').is(ByteSlice(data+4, 3)));
    // The last value is always copied because the reader
    // doesn't know there isn't anything next yet
    REQUIRE(reader.read_until_char(',') == ByteSlice("ghi", 3));
    REQUIRE_THROWS(reader.read_until_char(','));
}

TEST_CASE("can split across chunks", "[stream reader]") {
    auto reader = make_reader({"ab", ",cd,", "ef"});

    REQUIRE(reader.read_until_char(',') == ByteSlice("ab", 2));
    REQUIRE(reader.read_until_char(',') == ByteSlice("cd", 2));
    REQUIRE(reader.read_until_char(',') == ByteSlice("ef", 2));
    REQUIRE_THROWS(reader.read_until_char(','));
}

TEST_CASE("can read different sized chunks", "[stream reader]") {
    auto reader = make_reader({"a,bc,def,ghi,", "jklm,n,op", ",qrs"});
    std::vector<const char *> expected = {"a", "bc", "def", "ghi", "jklm", "n", "op", "qrs"};
    for (auto item : expected) {
        REQUIRE(reader.read_until_char(',') == ByteSlice(item, strlen(item)));
    }
    REQUIRE_THROWS(reader.read_until_char(','));
}

TEST_CASE("can store data across chunks", "[stream reader]") {
    const char * full = "The quick brown fox jumped over the lazy dog";
    auto reader = make_reader({"The quick ",  "brown", " fox jumped ", "over the lazy", " dog."});
    REQUIRE(reader.read_until_char('.') == ByteSlice(full, strlen(full)));
    REQUIRE(reader.read_until_char('.') == ByteSlice("", 0));
    REQUIRE_THROWS(reader.read_until_char(','));
}

TEST_CASE("Handles chunk boundaries and separators", "[stream reader]") {
    auto reader = make_reader({"a.b.c", "..de.", ".", "", "fg", "", "h.i.", "", ".j"});
    std::vector<const char *> expected = {"a", "b", "c", "", "de", "", "fgh", "i", "", "j"};
    for (auto item : expected) {
        REQUIRE(reader.read_until_char('.') == ByteSlice(item, strlen(item)));
    }
    REQUIRE_THROWS(reader.read_until_char('.'));
}

TEST_CASE("Custom find function works", "[stream reader]") {
    auto reader = make_reader({"UpperCase"});
    auto finder = SnakeFinder();
    REQUIRE(reader.read_until(finder) == ByteSlice("Upper", 5));
    REQUIRE(reader.read_until(finder) == ByteSlice("Case", 4));
    REQUIRE_THROWS(reader.read_until(finder));
}

TEST_CASE("Custom find function works repeated uppercase", "[stream reader]") {
    auto reader = make_reader({"A", "M", "CsTheW", "alking", "DEAD"});
    auto finder = SnakeFinder();
    REQUIRE(reader.read_until(finder) == ByteSlice("AMCs", 4));
    REQUIRE(reader.read_until(finder) == ByteSlice("The", 3));
    REQUIRE(reader.read_until(finder) == ByteSlice("Walking", 7));
    REQUIRE(reader.read_until(finder) == ByteSlice("DEAD", 4));
    REQUIRE_THROWS(reader.read_until(finder));
}