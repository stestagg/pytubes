#!/bin/bash

set -eux

export PATH=/opt/python/$1/bin:$PATH

ROOT=$PWD
WH=$GITHUB_WORKSPACE/wheelhouse

pip install pytest
pip install $WH/pytubes-*.whl

pytest .