.. _usage:

Getting Started
===============

Installation
------------

**From PyPi**::

    $ pip install pytubes

**From source**::

    $ pip install -r build_requirements.txt
    $ python setup.py install


Usage
-----

Usage is very simple:

#. Import ``tubes``
#. create an input tube (currently either: :class:`tube.Each` or :class:`tube.Count`) to get some data into the tube
#. call methods on the input tube to build up each step of the processing (e.g. ``read_files().split().json()``...)
#. Iterate over the tube to generate the data, by either:

   - Calling ``list(tube)``
   - looping over it in a for-loop:  ``for item in tube:``
   - or: Calling ``x = iter(tube)``, and then ``next(x)`` repeatedly.

Some Examples
~~~~~~~~~~~~~

>>> from tubes import Each, Count
>>> list(Count().first(5))
[0, 1, 2, 3, 4]

>>> from urllib.request import urlopen
>>> response = urlopen("https://dumps.wikimedia.org/other/pageviews/2019/2019-07/pageviews-20190716-140000.gz")
>>> dict(Each([response]).read_fileobj().gunzip(stream=True)  # Stream the response and gunzip it
        .tsv(sep=" ", skip_empty_rows=True)                   # Parse as a TSV file (with spaces not tabs)
        .skip_unless(lambda x: x.get(0).to(bytes).equals(b"en")) # EN wikipedia only
        .skip_unless(lambda x: x.get(2).to(int).gt(10_000))   # Only include pages with viewcount > 10,000
        .first(3)                                             # Get the first 5 only
        .multi(lambda x: (                                    # Extract Column 1(Page title) and Column 2(Page count)
            x.get(1).to(str),
            x.get(2).to(int))
        )
    )
{'-': 31066, 'Main_Page': 709331, 'Special:Search': 49869}
