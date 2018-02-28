pytubes
=======

A library for loading data into Python

.. toctree::
    :caption: Contents
    :maxdepth: 2
    :name: mastertoc

    tubes
    detail

Simple Example
--------------

>>> from tubes import Each
>>> import glob
>>> tube = (Each(glob.glob("*.json"))   # Iterate over some filenames
        .read_files()                   # Read each file, chunk by chunk
        .split()                        # Split the file, line-by-line
        .json()                         # parse json
        .get('country_code', 'null'))   # extract field named 'country_code'
>>> set(set)                            # collect results in a set
{'A1', 'AD', 'AE', 'AF', 'AG', 'AL', 'AM', 'AO', 'AP', ...}

More Complex Example 
--------------------

>>> from tubes import Each
>>> import glob

>>> x = (Each(glob.glob('*.jsonz'))
        .map_files()
        .gunzip()
        .split(b'\n')
        .json()
        .enumerate()
        .skip_unless(lambda x: x.slot(1).get('country_code', '""').to(str).equals('GB'))
        .multi(lambda x: (
            x.slot(0),
            x.slot(1).get('timestamp', 'null'),
            x.slot(1).get('country_code', 'null'),
            x.slot(1).get('url', 'null'),
            x.slot(1).get('file', '{}').get('filename', 'null'), 
            x.slot(1).get('file', '{}').get('project'), 
            x.slot(1).get('details', '{}').get('installer', '{}').get('name', 'null'),
            x.slot(1).get('details', '{}').get('python', 'null'),
            x.slot(1).get('details', '{}').get('system', 'null'),
            x.slot(1).get('details', '{}').get('system', '{}').get('name', 'null'),
            x.slot(1).get('details', '{}').get('cpu', 'null'),
            x.slot(1).get('details', '{}').get('distro', '{}').get('libc', '{}').get('lib', 'null'),
            x.slot(1).get('details', '{}').get('distro', '{}').get('libc', '{}').get('version', 'null'),
        ))
    )
>>> print(list(x)[-3])
(15,612,767, '2017-12-14 09:33:31 UTC', 'GB', '/packages/29/9b/25ef61e948321296f029f53c9f67cc2b54e224db509eb67ce17e0df6044a/certifi-2017.11.5-py2.py3-none-any.whl', 'certifi-2017.11.5-py2.py3-none-any.whl', 'certifi', 'pip', '2.7.5', {'name': 'Linux', 'release': '2.6.32-696.10.3.el6.x86_64'}, 'Linux', 'x86_64', 'glibc', '2.17')


What is it?
-----------

Pytubes is a library that optimizes loading dataset into memory.

At it's core is a set of specialized c++ classes that can be chained together
to load and manipulate data using a standard iterator pattern.  Around this
there is a cython extension module that makes defining and configuring a tube
simple and straight-forward.

A lot of the cost of loading data using pure python is typically centered around
function call overhead and allocating/copying object data.  

Pytubes tackles these bottlenecks by using a number of strategies:

 - iterator hot-loops are pure c++ function calls
 - zero-copy views onto array data
 - strict epoch-based lifetime rules avoid reference counting or GC during iteration
 - where possible, zero allocations during iteration
 - avoiding creating python objects where possible

These optimizations lead to significant performance improvements over pure python,
despite offering complex loading functionality.

Usage
-----

Usage is very simple:

#. Import ``tubes``
#. create an input tube (currently either: :class:`tube.Each` or :class:`tube.Count`) to get some data into the tube
#. continue to methods on the input tube to build up each step of the processing (e.g. ``read_files().split().json()``...)
#. Iterate over the tube to generate the data, by either:

   - Calling ``list(tube)``
   - looping over it in a for-loop:  ``for item in tube:``
   - or: Calling ``x = iter(tube)``, and then ``next(x)`` repeatedly.
   

Installation
------------

**From PyPi**::

    $ pip install pytubes

**From source**::

    $ pip install -r build_requirements.txt
    $ cd pyx
    $ python setup.py install

API
---

All tube methods are documented here: :ref:`api`