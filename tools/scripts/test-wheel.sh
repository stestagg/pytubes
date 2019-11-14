#!/bin/bash

set -eux

export PATH=/opt/python/$1/bin:$PATH

ROOT=$PWD
WH=$GITHUB_WORKSPACE/wheelhouse

pip install pytest pandas numpy
pip install $WH/pytubes-*.whl

pytest test