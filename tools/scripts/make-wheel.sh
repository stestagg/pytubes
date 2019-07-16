#!/bin/bash

set -ex

export PATH=/opt/python/$1/bin:$PATH

ROOT=$PWD

pip install -r $ROOT/build_requirements.txt

pip uninstall -y pyarrow

make clean-py clean-cpp test-py
pip wheel . -w wheelhouse/

WHEEL=wheelhouse/pytubes-*-$1-linux_x86_64.whl
auditwheel show $WHEEL
auditwheel repair --plat manylinux2010_x86_64 -w wheelhouse/ $WHEEL
rm wheelhouse/pytubes-*-$1-linux_x86_64.whl

if [ ! -z x"$GITHUB_WORKSPACE" ]; then
	WH=$GITHUB_WORKSPACE/wheelhouse

	[ -e $WH ] || mkdir $WH

	cp wheelhouse/pytubes*.whl $WH

fi