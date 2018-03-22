pytubes
=======

.. image:: https://travis-ci.org/stestagg/pytubes.svg?branch=master
    :target: https://travis-ci.org/stestagg/pytubes

.. image:: https://readthedocs.org/projects/pytubes/badge/
    :target: https://pytubes.readthedocs.io/en/latest/

Source: https://github.com/stestagg/pytubes

Pytubes is a library that optimizes loading datasets into memory.

At it’s core is a set of specialized C++ classes that can be chained together to load and manipulate data using a standard iterator pattern. Around this there is a cython extension module that makes defining and configuring a tube simple and straight-forward.



Simple Example
--------------

>>> from tubes import Each
>>> import glob
>>> tube = (Each(glob.glob("*.json"))   # Iterate over some filenames
        .read_files()                   # Read each file, chunk by chunk
        .split()                        # Split the file, line-by-line
        .json()                         # parse json
        .get('country_code', 'null'))   # extract field named 'country_code'
>>> set(tube)                           # collect results in a set
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

Contents
--------

.. role:: noshow

:noshow:`If you're viewing this in github, please visit the docs at: https://pytubes.readthedocs.io/en/latest/`

.. toctree::
    :maxdepth: 3
    :name: mastertoc

    intro_usage
    tubes
    performance
    detail
