import glob
import os
from os import path
import subprocess
import sys

from Cython.Build import cythonize
from distutils import log
from distutils.core import Extension, setup

PROJECT_ROOT = path.dirname(path.dirname(path.abspath(__file__)))
        
        
            # print(result)
            # fh.write(result)
        # build.build.run(self)

tube_ops = Extension(
    "tubes",
    ["tubes.pyx"],
    language="c++",
    include_dirs = ['../vendor', '.'],
    extra_compile_args=["-std=c++1z", '-g', "-O2"],
    extra_link_args=["-std=c++1z", '-lz', '-g'],
    #undef_macros = [ "NDEBUG" ],
)

def update_iter_defs():
    """
    Cython wrapper classes are generated for each iterator, (make_cdef.py)
    This has to be done before the cythonize() command below (which happens
    at import time) so run it inline here.
    """
    log.info("Generating iter_defs.pyx")
    script = path.join(PROJECT_ROOT, "tools", "make_cdef.py")
    source_files = glob.glob(path.join(PROJECT_ROOT, "src", "iters", "*.hpp"))
    result = subprocess.check_output([sys.executable, script] + source_files)

    with open(path.join(PROJECT_ROOT, "pyx", "iter_defs.pxi"), "wb") as fh:
        fh.write(result)

update_iter_defs()

setup(
    name='tubes',
    ext_modules = cythonize(
        tube_ops, 
        compiler_directives={"language_level": 3, 'embedsignature': True},
        include_path=['.']
    ),
)
