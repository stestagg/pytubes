.. _usage:

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
    $ python setup.py install