#!/bin/bash
set -ex

yum install -y zlib-devel

export PATH=/opt/python/$1/bin:$PATH

cd /pytubes

pip install -r /pytubes/build_requirements.txt
make clean-py clean-cpp test-py
pip wheel . -w wheelhouse/
auditwheel repair -w wheelhouse/ wheelhouse/pytubes-*-$1-linux_x86_64.whl
rm wheelhouse/pytubes-*-$1-linux_x86_64.whl