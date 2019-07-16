#!/bin/bash

set -ex

cd /pristine

CMD=$1

if [ -e "tools/scripts/$CMD.py" ]; then
	shift
	/opt/python/cp37-cp37m/bin/python "tools/scripts/$CMD.py" "$@"
elif [ -e "tools/scripts/$CMD.sh" ]; then
	shift
	./tools/scripts/$CMD.sh "$@"
else
	"$@"
fi