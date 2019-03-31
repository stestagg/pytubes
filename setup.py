import glob
import os
import hashlib
from os import path
import re
try:
    import numpy
except ImportError:
    np_get_include = lambda: 'src'
else:
    np_get_include = numpy.get_include

try:
    import pyarrow
except ImportError:
    HAVE_PYARROW = False
else:
    HAVE_PYARROW = True

import subprocess
import sys

from Cython.Build import cythonize
from distutils.command.build_clib import build_clib
from distutils.core import setup, Extension


PROJECT_ROOT = path.dirname(path.abspath(__file__))



with open(path.join(PROJECT_ROOT, "README.rst")) as fh:
    LONG_DESCRIPTION = fh.read()


with open(path.join(PROJECT_ROOT, "pyx", "version.pxi")) as fh:
    VERSION = "%s.%s.%s" % re.match(r'__version__\s*=\s*\((\d+),\s*(\d+),\s*(\d+)\)', fh.read()).groups()


def hash_file(*parts):
    file_path = path.join(PROJECT_ROOT, *parts)
    if not path.exists(file_path):
        return ""
    with open(file_path, "rb") as fh:
        return hashlib.sha1(fh.read().strip()).hexdigest()


def write_pyx_defs(args):
    script = path.join(PROJECT_ROOT, "pyx", "config.pxi")
    with open(script, 'w') as fh:
        for key, value in args.items():
            fh.write("DEF %s=%s\n" % (key, value))


def update_iter_defs():
    """
    Cython wrapper classes are generated for each iterator, (make_cdef.py)
    This has to be done before the cythonize() command below (which happens
    at import time) so run it inline here.
    """
    script = path.join(PROJECT_ROOT, "tools", "make_cdef.py")
    source_files = glob.glob(path.join(PROJECT_ROOT, "src", "iters", "*.hpp"))
    result = subprocess.check_output([sys.executable, script] + source_files)

    if hash_file("pyx", "iter_defs.pxi") != hashlib.sha1(result.strip()).hexdigest():
        print("Updating iter_defs.pxi")
        with open(path.join(PROJECT_ROOT, "pyx", "iter_defs.pxi"), "wb") as fh:
            fh.write(result)


update_iter_defs()
write_pyx_defs({
    'HAVE_PYARROW': HAVE_PYARROW
})

zlib = ('zlib', {
    'sources': glob.glob('vendor/zlib/*.c'),
    'include_dirs': ['vendor/zlib'],
})

CTUBES_OPTIONS = {
    'sources': ["pyx/ctubes.pyx"],
    'language': "c++",
    'include_dirs':  [
        'vendor',
        'vendor/zlib',
        'src',
        np_get_include(),
    ],
    'libraries': [],
    'extra_compile_args': ['-std=c++11', '-g', '-O2'],
    'extra_link_args': ['-std=c++11', '-g'],
}

if HAVE_PYARROW:
    CTUBES_OPTIONS['libraries'].extend(pyarrow.get_libraries())
    CTUBES_OPTIONS['library_dirs'] = pyarrow.get_library_dirs()
    CTUBES_OPTIONS['include_dirs'].append(pyarrow.get_include())
    CTUBES_OPTIONS['define_macros'] = [('HAVE_PYARROW', )]
    CTUBES_OPTIONS['extra_compile_args'].append('-DHAVE_PYARROW')

setup(
    name='pytubes',
    ext_modules = cythonize(
        Extension(
            "ctubes",
            **CTUBES_OPTIONS
        ),
        compiler_directives={"language_level": 3, 'embedsignature': True},
        include_path=['.'],
    ),
    libraries=[zlib],
    py_modules=['tubes'],
    cmdclass = {'build_clib': build_clib},
    version=VERSION,
    description="A library for efficiently loading data into Python",
    long_description=LONG_DESCRIPTION,
    author="Stephen Stagg",
    author_email="stestagg@gmail.com",
    python_requires=">=3.4.0",
    url="https://github.com/stestagg/pytubes",
    install_requires=['numpy', 'pyarrow'],
    include_package_data=True,
    license='MIT',
    classifiers=[
        # Trove classifiers
        # Full list: https://pypi.python.org/pypi?%3Aaction=list_classifiers
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: Implementation :: CPython',
    ],
)
