#pragma once

#include "buffer.hpp"

namespace ss{

    namespace arrow{

        class NullContainer {
            size_t count;
        public:
            NullContainer(): count(0) {};

            inline void increment() {
                count += 1;
            }

            inline size_t size() const {
                return count;
            }

        };

        class BoolContainer{
        public:
            aligned64_vector<uint8_t> contents;
            size_t count;
            using value_type = bool;

            BoolContainer(): contents(), count(0) {};

            inline void emplace_back(const bool value) {
                uint8_t shift = (count % 8);
                if (shift == 0) {
                    contents.emplace_back(0);
                }
                uint8_t bit = (value & 0x1) << shift;
                contents[count / 8] |= bit;
                count += 1;
            }

            inline size_t size() const {
                return count;
            }
        };

        template<class T> class StringContainer {
        public:
            aligned64_vector<T> contents;
            aligned64_vector<int32_t> offsets;

            using value_type = T;

            StringContainer() : contents(), offsets() {
                contents.reserve(16384);
                offsets.reserve(4096);
                offsets.push_back(0);
            }

            inline void emplace_back(const Slice<T> &data) {
                contents.insert(contents.end(), data.cbegin(), data.cend());
                offsets.emplace_back(contents.size());
            }

            inline size_t size() const {
                return offsets.size() - 1;
            }

        };

    }

}