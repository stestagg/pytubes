#!/bin/bash

set -ex

. /opt/rh/rh-python35/enable

VERSION=$(python tools/scripts/get-version.py)

make install

DEST=$GITHUB_WORKSPACE/build/docs

mkdir -p $DEST

git config --global user.email "pytubes@sta.gg"
git config --global user.name "Pytubes Builder"

REPO=https://${ACCESS_TOKEN}:@github.com/stestagg/pytubes

git clone $REPO -b gh-pages $DEST

VER_DIR=$DEST/versions/$VERSION

mkdir -p $VER_DIR

cd /pristine/docs


sphinx-build -b html . $VER_DIR

cd $DEST/versions

python /pristine/tools/version-list.py

cd $DEST

git add -A
git commit -m "Update docs for $VERSION"

git push