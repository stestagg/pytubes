#pragma once

#include "slice.hpp"
#include "../iter.hpp"

namespace ss{ namespace iter{

    template<class T>
    struct SliceItemFinder {
        const T sep;
        SliceItemFinder(T sep) : sep(sep) {}

        inline const T *find_next(Slice<T> &slice) const {
            return slice.find_first(sep);
        }

        inline Slice<T> get_remaining(const Slice<T> &slice, const T* const match) const{
            return slice.slice_from_ptr(match  + 1);
        }
    };

    template<class T>
    class StreamReader{
        Chain chain;
        const Slice<T> *input;

        std::basic_string<typename std::remove_cv<T>::type> buffer;
        bool buffer_pending = false;
        bool done = false;

        Slice<T> remaining_source;
    public:

        StreamReader(Chain chain, const Slice<T> *input) :
            chain(chain),
            input(input)
        {}

        template<class Finder, class... Args>
        Slice<T>
        read_until(Finder &finder, Args &&... args) {
            while(true) {
                if (remaining_source.is_empty()) {
                    // Nothing left in the upstream chunk, so clean up and stop
                    if (done) {
                        throw StopIteration;
                    }
                    try{
                        // Try to get the next chunk from upstream
                        do_next(chain);
                        remaining_source = *input;
                    } catch (StopIterationExc e) {
                        // Parent has no more to give, so set done state, but check if
                        // there's anything left over that was unterminated..
                        // just to be safe, nuke cur_slice
                        remaining_source = Slice<T>::Null();
                        done = true;
                        if (buffer_pending) {
                            // Found some, return a view onto this
                            return Slice<T>(buffer.data(), buffer.size());
                        }
                        // Bubble up the stop iteration if nothing saved
                        throw;
                    }
                    // Now we've got a chunk, but there may be some data left from the
                    // last chunk, and these have to be joined, so special case that
                    if (buffer_pending) {
                        const T *match = finder.find_next(remaining_source, args...);
                        auto this_part = remaining_source.slice_to_ptr(match);
                        buffer.insert(buffer.end(), this_part.begin(), this_part.end());
                        // Adjust the rest of the slice to compensate, and return
                        // the whole buffer
                        if (match == remaining_source.end()) {
                            // There was no match in the whole chunk, so loop again
                            remaining_source = Slice<T>::Null();
                            continue;
                        }
                        remaining_source = finder.get_remaining(remaining_source, match);
                        // We've returned the buffer, so not pending any more
                        buffer_pending = false;
                        return Slice<T>(buffer.data(), buffer.size());
                    }
                }

                // This is the happy-path: look for a match
                const T *match = finder.find_next(remaining_source, args...);
                if (match == remaining_source.end()) {
                    // We reached the end of the upstream chunk without finding
                    // a match. Set buffer to be the remaining data, for later,
                    // allowing us to call next() on the parent iter because
                    // we've got a copy of the trailer part here.
                    buffer.resize(remaining_source.len);
                    buffer_pending = true;
                    std::copy(remaining_source.begin(), remaining_source.end(), buffer.begin());
                    // remaining_source is no longer useful, so make it null so
                    // we know next time that it's exhausted
                    remaining_source = Slice<T>::Null();
                    // loop again to grab a new chunk and continue processing.
                    continue;
                }
                auto next = remaining_source.slice_to_ptr(match);
                remaining_source = finder.get_remaining(remaining_source, match);
                return next;
            }
        }

        Slice<T> read_until_char(T sep) {
            auto finder = SliceItemFinder<T>(sep);
            return read_until(finder);
        }

    };

} }