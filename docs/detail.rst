More Detail
===========

How does iteration work?
------------------------

Calling iter() on a tube, either explicitly, or implicitly (by a call to list() or for-loop)
causes a number of things to happen:

#. pytubes recurses over each tube, looking at its inputs to build directed dependency graph of each step
#. This DAG is then partitioned into chains (see `Concepts`) of tubes that are iterated together
#. The chains, and their contents are sorted so that each iter's input are processed in the correct order.
#. Each tube is then asked to create an iter, converting all arguments and inputs into their relevant C values
#. The resulting chain of Iters are given to a cython Iter wrapper object that has a ``__next__()`` method that
   produces python values for each iteration.

Concepts
--------

The idea of having an efficient c-based data loader library is quite straight-forward
but the detail is quite subtle.  The main issues are:

  - Python's pure dynamic typing is hard to reconcile with type information needed for c-level performant code

  - To be useful, data loaders have to handle a verywide variety of use-cases, 
    which can easily lead to quadratic code complexity without careful abstraction management

To implement a natural feeling interface to the library, the python-facing side
tries to feel pythonic and generally tries to avoid deling with types explicitly.
By the time the ``Tube`` is converted to an ``Iter``, lots of type information has
been resolved, and optimally connected.  

To achieve this while keeping the implementation managable, pytubes internally 
has a number of concepts:

Tube:
    A tube describes a number of steps to load data.  Tubes typically refer to 
    parent tubes recursively, and store any parameters that configure the behaviour
    of the resulting iterator.  By themselves, they are pure cython classes, and
    do not deal with processing the actual data directly.

    Instead, calling :func:`iter` on a tube causes it to generate an Iter
    object which is a recursive C++ class that performs the actual data processing

Iter:
    A set of C++ classes that implement the data processing implementation.

    They are entirely managed by cython wrappers and the Tube interface, so 
    knowledge of them is not required for normal usage of the library.

    Internally, ``Iter`` s expose two methods:

    - ``get_slots()`` which returns a vector
      of `SlotPointers` (glorified, type-checked c-pointers) that allow other Iterators
      to consume the data produced by this iter.

    - ``next()`` causes the iter to process the next item, and update its internal
       slots so they reflect upstream changes


Dtype:
    Each tube exposes a property ``Tube.dtype`` that defines the data type of each ``Slot`` in the iterator.

    Tubes can consume/produce multiple values per iteration, so Tube ``dtype`` s are tuples.
    Most tubes only have a single dtype entry, but some (e.g. :func:`Tube.enumerate`, :func:`Tube.multi`) can 
    have many.

    Most tubes only look at/act on the first item of a dtype when getting values.  The :func:`Tube.slot()` method
    can extract a single slot from a tube with many slots.

Slot:
    Knowledge of slots is not required for normal usage of pytubes, but may be useful for a more in-depth 
    understanding of how the library works.

    The underlying C++ Iter classes work by sharing pointers to iter-lifetime fixed
    memory locations.  Each time ``next()`` is called on an Iter, the values at these
    memory location change, but the addresses do not.  This can make iteration very efficient.

    The memory locations for each value being handled by an ``Iter`` is referred to
    both as a ``Slot``, and a ``SlotPointer`` in the code.
