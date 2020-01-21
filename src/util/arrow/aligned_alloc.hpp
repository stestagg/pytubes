#pragma once


namespace ss {

	template<int N> struct Alloc {

		static void *alloc(size_t num);
		static void free(void *obj);
	};

	template <class T, int N>
    class AlignedAllocator
    {

    public:

        typedef T value_type;
        typedef T& reference;
        typedef const T& const_reference;
        typedef T* pointer;
        typedef const T* const_pointer;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        template <class U>
            struct rebind
            {
                typedef AlignedAllocator<U,N> other;
            };

        typedef Alloc<N> alloc_type;

        inline AlignedAllocator() throw() {}
        inline AlignedAllocator(const AlignedAllocator&) throw() {}

        template <class U>
            inline AlignedAllocator(const AlignedAllocator<U,N>&) throw() {}

        inline ~AlignedAllocator() throw() {}

        inline pointer address(reference r) { return &r; }
        inline const_pointer address(const_reference r) const { return &r; }

        pointer allocate(size_type n, typename std::allocator<void>::const_pointer hint = 0){
          void * ptr = alloc_type::alloc(n * sizeof(T));
          if(!ptr) throw std::bad_alloc();
        	return reinterpret_cast<T*>(ptr);
        }
        inline void deallocate(pointer p, size_type){
          return alloc_type::free(p);
        }

        inline void construct(pointer p, const_reference value) { new (p) value_type(value); }
        inline void destroy(pointer p) { p->~value_type(); }

        inline size_type max_size() const throw() { return size_type(-1) / sizeof(T); }

        inline bool operator==(const AlignedAllocator&) { return true; }
        inline bool operator!=(const AlignedAllocator& rhs) { return !operator==(rhs); }

    };

#if defined(__GLIBC__) && ((__GLIBC__>=2 && __GLIBC_MINOR__ >= 8) || __GLIBC__>2)
    #if defined(__LP64__)
    	#define MALLOC_ALIGNMENT 16
   	#else
   		#define MALLOC_ALIGNMENT 8
    #endif
#elif defined(__APPLE__) || defined(_WIN64)
    #define MALLOC_ALIGNMENT 16
#endif

#if MALLOC_ALIGNMENT>=16
    template<> inline void *Alloc<16>::alloc(size_t n) {
   		return malloc(n);
   	}
   	template<> inline void Alloc<16>::free(void *obj) {
   		return ::free(obj);
   	}
#endif

#if MALLOC_ALIGNMENT>=8
    template<> inline void *Alloc<8>::alloc(size_t n) {
    	return malloc(n);
   	}
   	template<> inline void Alloc<8>::free(void *obj) {
   		return ::free(obj);
   	}
 #endif

 #if SSE_INSTR_SET > 0
    #pragma message("using aligned allocate fn: mm_malloc")
    template<int N> inline void *Alloc<N>::alloc(size_t n) {
   		_mm_malloc(n, N);
   }
   template<int N> inline void Alloc<N>::free(void *obj) {
   		_mm_free(obj);
	}
#elif (defined(__APPLE__)) \
      ||((defined __QNXNTO__) \
        || (defined _GNU_SOURCE) \
        || ((defined _XOPEN_SOURCE) && (_XOPEN_SOURCE >= 600))) \
 		    && (defined _POSIX_ADVISORY_INFO) && (_POSIX_ADVISORY_INFO > 0)
  #pragma message("using aligned allocate fn: posix_memalign")
 	template<int N> inline void *Alloc<N>::alloc(size_t n) {
   		void* res;
      const int failed = posix_memalign(&res,N,n);
      if(failed){
        res = 0;
      }
      return res;
   }
   template<int N> inline void Alloc<N>::free(void *obj) {
   		::free(obj);
	}
#elif defined(_WIN32)
  #include <malloc.h>
  #pragma message("using aligned allocate fn: _aligned_malloc")
  template<int N> inline void *Alloc<N>::alloc(size_t n) {
    void *ptr = _aligned_malloc(n, N);
    if(!ptr) throw std::bad_alloc();
    return ptr;
  }
  template<int N> inline void Alloc<N>::free(void *obj) {
    _aligned_free(obj);
  }
#else
  #pragma message("using aligned allocate fn: aligned_alloc")
  #include <stdlib.h>
  template<int N> inline void *Alloc<N>::alloc(size_t n) {
    if (n % N != 0) {n += n - (n % N);}
    return aligned_alloc(N, n);
  }
  template<int N> inline void Alloc<N>::free(void *obj) {
    free(obj);
  }

#endif

}