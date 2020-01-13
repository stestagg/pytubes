#pragma once

#include "aligned_alloc.hpp"

namespace ss{ namespace arrow {

template<class T> class ContiguousBuffer {
public:
	std::vector<T, AlignedAllocator<T, 64>> items;

	template <class... Args>
	void emplace(Args &&... args){
		items.emplace_back(args...);
	}

	
};

}}