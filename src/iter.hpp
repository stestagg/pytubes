#pragma once


#include <vector>

#include "bytes.hpp"
#include "slot.hpp"
#include "scalar.hpp"
#include "undefined.hpp"

// This is quite ugly, but I'm unsure of a better way to do this
// CppExn2Pyerr is the default Cython C++>Python exception converter
// We call this if next() raises an exception that isn't StopIteration
// to allow default exception handling behaviour to kick in.
static void __Pyx_CppExn2PyErr();

namespace ss{ namespace iter{

    template<class T> using vector = std::vector<T>;

    class Iter{

    public:
        virtual Slice<SlotPointer> get_slots() = 0;
        virtual void next() = 0;
        virtual ~Iter() = default;
    };

    using AnyIter = std::shared_ptr<Iter>;

    inline vector<Iter *> iters_from_anyiters(const vector<AnyIter> &chain) {
        vector<Iter *>iters;
        iters.reserve(chain.size());
        for (auto item : chain) {
            iters.push_back(item.get());
        }
        return iters;
    }

    using Chain = Array<AnyIter>;

    inline void do_next(const Chain &chain) {
        for (auto &iter : chain) {
            iter->next();
        }
    }

    template<class T>
    inline AnyIter to_any(T *source) {
        return AnyIter(source, [](T *p){ delete p;});
    }

    inline void convert_stop_iteration() {
      try {
        throw;
      } catch (const StopIterationExc &si) {
        PyErr_SetString(PyExc_StopIteration, "Iteration Finished");
      } catch(const MissingValue &mv) {
        PyErr_SetString(PyExc_KeyError, mv.what());
      } catch(...) {
        __Pyx_CppExn2PyErr();
      }
    }

    inline void init_pytubes_internal() {
        init_undefined();
    }

}}