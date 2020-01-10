#!/bin/bash

set -eux

ROOT=$PWD
WH=$GITHUB_WORKSPACE/wheelhouse

pip install pytest pandas numpy
pip install $WH/pytubes-*.whl

pytest test