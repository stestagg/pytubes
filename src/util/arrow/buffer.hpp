#pragma once

#include "aligned_alloc.hpp"

namespace ss{

    template<class T>
    using aligned64_vector = std::vector<T, AlignedAllocator<T, 64> >;

    namespace arrow {

        class AnyBuffer {
        public:
            virtual void *data() = 0;
            virtual size_t size() = 0;

            AnyBuffer() = default;
            AnyBuffer & operator=(const AnyBuffer&) = delete;
            AnyBuffer(const AnyBuffer&) = delete;
            virtual ~AnyBuffer() = default;
        };

        template<class T>
        class Buffer: public AnyBuffer {
            aligned64_vector<T> vec;

        public:

            template<typename... Args>
            static std::unique_ptr<Buffer<T> > unique(Args&&... args) {
                return std::unique_ptr<Buffer<T> >(new Buffer<T>(std::forward<Args>(args)...));
            }

            Buffer(aligned64_vector<T> &&container) {
                vec = container;
            }

            Buffer& operator=(Buffer&& other){
                vec = std::move(other.vec);
            }
            Buffer& operator=(aligned64_vector<T> &&src) {
                vec = std::move(src);
            }

            void *data() { return (void*)vec.data(); }
            
            inline size_t size() {
                return vec.size() * sizeof(T);
            }
        };

}}
