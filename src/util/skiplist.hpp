#pragma once

namespace ss {

    template<class T>
    struct SkipListItem{
        const size_t skip;
        T * const destination;

        SkipListItem(size_t skip, T *destination):
            skip(skip),
            destination(destination) {}
    };

    template<class T>
    using SkipList = std::vector<SkipListItem<T>>;


}