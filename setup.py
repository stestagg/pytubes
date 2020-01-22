import fnmatch
import glob
import hashlib
import os
import re
import subprocess
import sys
from distutils.command.build_clib import build_clib
from distutils.core import Extension, setup
from os import path
from pathlib import Path

from Cython.Build import cythonize

def get_compiler():
    import distutils.ccompiler
    return distutils.ccompiler.get_default_compiler()


IS_MSVC = get_compiler() == 'msvc'

def c_arg(gcc_ver, win_ver):
    return win_ver if IS_MSVC else gcc_ver


os.environ['CFLAGS'] = os.environ.get('CFLAGS', '') + c_arg(' -msse4', '/arch:SSE2')

try:
    import numpy
except ImportError:
    # Pytubes requires numpy to build, but
    # doing this allows setup.py to be imported
    # even if numpy isn't installed, something that's required to, for example
    # work out that numpy is a dependency
    np_get_include = lambda: 'src'
else:
    np_get_include = numpy.get_include

    
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



update_iter_defs()

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


def should_be_excluded(fn):
    rel = fn.relative_to(ARROW_SOURCE_ROOT)
    path_parts = {p.name for p in rel.parents if p.name}
    if path_parts & PATHS_TO_EXCLUDE:
        return True
    for pattern in PATTERNS_TO_EXCLUDE:
        if fnmatch.fnmatch(fn.name, pattern):
            return True


CTUBES_OPTIONS = {
    'sources': ["pyx/tubes.pyx"] + DOUBLE_SOURCES,
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
    'extra_compile_args': [
        c_arg('-std=c++11', '/std:c++14'),
        c_arg('-g', '/DEBUG:FASTLINK'),
        c_arg('-O3', '/O2'),
        c_arg('-msse4', '')
    ],
    'extra_link_args': [
        c_arg('-std=c++11', ''),
        c_arg('-g', '/Zi'),
    ],
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
    install_requires=['numpy'],
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
