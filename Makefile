
PYTHON_CONFIG ?= python3-config

CPP_TEST_FILES = $(shell find ./ -type f -name '*_test.cpp')
PY_C_FLAGS = $(shell $(PYTHON_CONFIG) --cflags)
PY_LD_FLAGS = $(shell $(PYTHON_CONFIG) --ldflags)
PY_LIBRARY_PATH = $(shell $(PYTHON_CONFIG) --prefix)/lib
PY_EXTRA_LD_FLAGS = -L$(PY_LIBRARY_PATH)

CXX ?= g++
LDSHARED ?= g++

STD = -std=c++11

test: test-cpp test-py

test-py: install
	py.test -s test

doc:
	(cd docs && make html)

install:
	python setup.py install

build:
	python setup.py build

upload: clean-py
	python setup.py sdist bdist_wheel
	twine upload dist/*

clean: clean-doc clean-py clean-cpp

clean-cpp:
	-rm *.o
	-rm run-tests
	-find ./ -name *.dSYM -exec rm -r {} ';'

clean-doc:
	(cd docs && make clean)

clean-py:
	python setup.py clean
	-rm dist/*
	find ./ -name *.pyc -delete	
	-find ./ -name __pycache__ -exec rm -r {} ';'
	-rm -rf pytubes.egg-info

test-cpp: run-tests
	LD_LIBRARY_PATH=$(PY_LIBRARY_PATH) ./run-tests

run-tests: test-run.o $(CPP_TEST_FILES)
	$(CXX) $(STD) $(PY_C_FLAGS) -fPIC -Ivendor/  -o run-tests test-run.o $(CPP_TEST_FILES) $(PY_EXTRA_LD_FLAGS) $(PY_LD_FLAGS)

test-run.o:
	$(CXX) $(STD) $(PY_C_FLAGS) -fPIC -Ivendor/ -c -o test-run.o src/test.cpp

.PHONY: build