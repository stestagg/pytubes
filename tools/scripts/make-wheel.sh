#!/bin/bash

set -eux

#export PATH=/opt/python/$1/bin:$PATH
echo $PATH
which python
python --MAJOR_VERSION
which pip
python -m pip install --upgrade pip
which pip
exit 1

ROOT=$PWD

pip install -r $ROOT/build_requirements.txt

make clean install

pip wheel . -w wheelhouse/

MAJOR_VERSION=${1%\.*}
MINOR_VERSION=${1#*\.}
SHORT_VER=${MAJOR_VERSION}${MINOR_VERSION}

BIN_VER=cp${SHORT_VER}-cp${SHORT_VER}m

WHEEL=wheelhouse/pytubes-*-$BIN_VER-linux_x86_64.whl
auditwheel show $WHEEL
auditwheel repair --plat manylinux2010_x86_64 -w wheelhouse/ $WHEEL
rm wheelhouse/pytubes-*-$BIN_VER-linux_x86_64.whl

if [ ! -z x"$GITHUB_WORKSPACE" ]; then
	WH=$GITHUB_WORKSPACE/wheelhouse

	[ -e $WH ] || mkdir $WH

	cp wheelhouse/pytubes*.whl $WH

fi