
PYTHON_CONFIG ?= python3-config

LIB_VERSION = $(shell python tools/version.py)

CPP_TEST_FILES = $(shell find ./src -type f -name '*_test.cpp')
HPP_FILES = $(shell find ./ -type f -name '*.hpp')

PY_C_FLAGS = $(shell $(PYTHON_CONFIG) --cflags)
PY_LD_FLAGS = $(shell $(PYTHON_CONFIG) --ldflags)
PY_LIBRARY_PATH = $(shell $(PYTHON_CONFIG) --prefix)/lib
PY_EXTRA_LD_FLAGS = -L$(PY_LIBRARY_PATH)

CXX ?= g++
LDSHARED ?= g++

STD = -std=c++11

wheels: clean
	python tools/build_wheels.py

test: test-cpp test-py

test-py: install
	py.test -s test

doc:
	(cd docs && make html)

install:
	python setup.py install

build:
	python setup.py build

clean: clean-doc clean-py clean-cpp clean-wheels

clean-wheels:
	-rm wheelhouse/*

clean-cpp:
	-rm *.o
	-rm run-tests
	-find ./ -name *.dSYM -exec rm -r {} ';'

clean-doc:
	(cd docs && make clean)

clean-py:
	python setup.py clean
	-rm -rf build
	-rm -rf dist
	-rm pyx/*.html
	find ./ -name *.pyc -delete
	-find ./ -name __pycache__ -exec rm -r {} ';'
	-rm -rf pytubes.egg-info

test-cpp: run-tests
	LD_LIBRARY_PATH=$(PY_LIBRARY_PATH) ./run-tests

run-tests: test-run.o $(CPP_TEST_FILES) $(HPP_FILES)
	LD_LIBRARY_PATH=$(PY_LIBRARY_PATH) $(CXX) $(STD) $(PY_C_FLAGS) -fPIC -Ivendor/  -o run-tests test-run.o $(CPP_TEST_FILES) $(PY_EXTRA_LD_FLAGS) $(PY_LD_FLAGS)

test-run.o:
	$(CXX) $(STD) $(PY_C_FLAGS) -fPIC -Ivendor/ -c -o test-run.o src/test.cpp

.PHONY: build