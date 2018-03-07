import glob
import os
import hashlib
from os import path
import re
import numpy
import subprocess
import sys

from Cython.Build import cythonize
from setuptools import setup
from distutils.command.build_clib import build_clib
from setuptools.extension import Extension


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

zlib = ('zlib', {
    'sources': glob.glob('vendor/zlib/*.c'),
    'include_dirs':['vendor/zlib'],
})

setup(
    name='pytubes',
    ext_modules = cythonize(
        Extension(
            "tubes",
            sources=["pyx/tubes.pyx"],
            language="c++",
            include_dirs = ['vendor', 'pyx', 'src', numpy.get_include()],
            extra_compile_args=["-std=c++11", '-g', "-O2"],
            extra_link_args=["-std=c++11", '-g'],
        ), 
        compiler_directives={"language_level": 3, 'embedsignature': True},
        include_path=['.']
    ),
    libraries=[zlib],
    cmdclass = {'build_clib': build_clib},
    version=VERSION,
    description="A library for efficiently loading data into Python",
    long_description=LONG_DESCRIPTION,
    author="Stephen Stagg",
    author_email="stestagg@gmail.com",
    python_requires=">=3.4.0",
    url="https://github.com/stestagg/pytubes",
    install_requires=[],
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
