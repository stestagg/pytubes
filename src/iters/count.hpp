#pragma once

#include "../iter.hpp"

namespace ss{ namespace iter{

    class CountIter : public Iter {
        /*<-
        Iter:
            CountIter: [size_t]
        Tube:
            Count:
                props: [{type: size_t, name: _start, default: 0}]
                dtype: return (Int64,)
                iter: [CountIter, [self._start]]
                docstring: |
                    Iterator that behaves similarly to :func:`itertools.count`.

                    Takes an optional numeric argument ``start`` that sets the
                    first number returned by Count()  [default:0]

                    >>> list(Count().first(5))
                    [0, 1, 2, 3, 4]
                    >>> list(Count(10).first(5))
                    [10, 11, 12, 13, 14]
        ->*/

        int64_t cur;
        SlotPointer slot;

    public:
        CountIter(int64_t start) : cur(start - 1), slot(&cur) {}

        Slice<SlotPointer> get_slots() {
            return Slice<SlotPointer>(&slot, 1);
        }
        void next(){
            cur += 1;
        }


    };

}}