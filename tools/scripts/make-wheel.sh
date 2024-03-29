#!/bin/bash

set -eux

export PATH=/opt/python/$1/bin:$PATH

ROOT=$PWD

pip install --upgrade pip
pip install -r $ROOT/build_requirements.txt

make -j4 clean
make install

pip wheel . -w wheelhouse/

WHEEL=wheelhouse/pytubes-*-$1-linux_x86_64.whl
auditwheel show $WHEEL
auditwheel repair --plat manylinux2014_x86_64 -w wheelhouse/ $WHEEL
rm wheelhouse/pytubes-*-$1-linux_x86_64.whl

if [ ! -z x"$GITHUB_WORKSPACE" ]; then
	WH=$GITHUB_WORKSPACE/wheelhouse

	[ -e $WH ] || mkdir $WH

	cp wheelhouse/pytubes*.whl $WH

fi