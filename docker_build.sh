#!/usr/bin/bash

apt-get install -y build-essential
pip install -r requirements.txt

make test