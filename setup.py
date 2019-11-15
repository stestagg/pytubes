import fnmatch
from pathlib import Path
import glob
import os
os.environ['CFLAGS'] = os.environ.get('CFLAGS', '') + ' -msse4'
import hashlib
from os import path
import re
try:
    import numpy
except ImportError:
    np_get_include = lambda: 'src'
else:
    np_get_include = numpy.get_include

import subprocess
import sys

from Cython.Build import cythonize
from distutils.command.build_clib import build_clib
from distutils.core import setup, Extension


PROJECT_ROOT = Path(__file__).absolute().parent


with (PROJECT_ROOT / "README.rst").open() as fh:
    LONG_DESCRIPTION = fh.read()


with (PROJECT_ROOT / "pyx" / "version.pxi").open() as fh:
    VERSION = "%s.%s.%s" % re.match(r'__version__\s*=\s*\((\d+),\s*(\d+),\s*(\d+)\)', fh.read()).groups()


def hash_file(*parts):
    file_path = PROJECT_ROOT.joinpath(*parts)
    if not file_path.exists():
        return ""
    with file_path.open("rb") as fh:
        return hashlib.sha1(fh.read().strip()).hexdigest()


def update_iter_defs():
    """
    Cython wrapper classes are generated for each iterator, (make_cdef.py)
    This has to be done before the cythonize() command below (which happens
    at import time) so run it inline here.
    """
    script = str(PROJECT_ROOT / "tools" / "make_cdef.py")
    source_files = list((PROJECT_ROOT / "src" / "iters").glob("*.hpp"))
    source_files = [str(f) for f in source_files]
    result = subprocess.check_output([sys.executable, script] + source_files)

    if hash_file("pyx", "iter_defs.pxi") != hashlib.sha1(result.strip()).hexdigest():
        print("Updating iter_defs.pxi")
        with (PROJECT_ROOT / 'pyx' / 'iter_defs.pxi').open('wb') as fh:
            fh.write(result)


def update_arrow_config():
    config_source = PROJECT_ROOT / 'tools' / 'arrow.config.h'
    config_dest = PROJECT_ROOT / 'vendor' / 'arrow' / 'cpp' / 'src' / 'arrow' / 'util' / 'config.h'
    with config_source.open('rb') as from_file:
        with config_dest.open('wb') as to_file:
            to_file.write(from_file.read())



update_iter_defs()
update_arrow_config()

zlib = ('zlib', {
    'sources': glob.glob('vendor/zlib/*.c'),
    'include_dirs': ['vendor/zlib'],
})

DOUBLE_SOURCES = [
    str(f.relative_to(PROJECT_ROOT))
    for f in 
    (PROJECT_ROOT / 'vendor' / 'double-conversion').glob('**/*.cc')
    if 'test' not in f.name and 'benchmark' not in f.name
]

ARROW_SOURCE_ROOT = PROJECT_ROOT / 'vendor' / 'arrow' / 'cpp' / 'src' / 'arrow'

ALL_ARROW_SOURCES = list(ARROW_SOURCE_ROOT.glob('**/*.cc'))
PATHS_TO_EXCLUDE = {
    'filesystem',
    'flight',
    'gpu',
    'csv',
    'compute',
    'dataset',
    'dbi',
    'ipc',
    'testing',
    'json',
    'python',
    'adapters',
}
PATTERNS_TO_EXCLUDE = [
    '*test*',
    'file_parquet.cc',
    'hdfs*.cc',
    'compression*.cc',
    'feather.cc',
    'uri.cc',
    '*benchmark*',
]

def should_be_excluded(fn):
    rel = fn.relative_to(ARROW_SOURCE_ROOT)
    path_parts = {p.name for p in rel.parents if p.name}
    if path_parts & PATHS_TO_EXCLUDE:
        return True
    for pattern in PATTERNS_TO_EXCLUDE:
        if fnmatch.fnmatch(fn.name, pattern):
            return True


ARROW_SOURCES = [
    str(f.relative_to(PROJECT_ROOT)) for f in ALL_ARROW_SOURCES 
    if not should_be_excluded(f)
]


CTUBES_OPTIONS = {
    'sources': ["pyx/tubes.pyx"] + DOUBLE_SOURCES + ARROW_SOURCES,
    'language': "c++",
    'include_dirs':  [
        'vendor',
        'vendor/zlib',
        'vendor/arrow/cpp/src/',
        'vendor/double-conversion/',
        'src',
        np_get_include(),
    ],
    'libraries': [],
    'extra_compile_args': ['-std=c++11', '-g', '-O2', '-msse4'],
    'extra_link_args': ['-std=c++11', '-g'],
}


setup(
    name='pytubes',
    ext_modules = cythonize(
        Extension(
            "tubes",
            **CTUBES_OPTIONS
        ),
        compiler_directives={"language_level": 3, 'embedsignature': True},
        include_path=['.'],
    ),
    libraries=[zlib],
    py_modules=[],
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
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: Implementation :: CPython',
    ],
)
