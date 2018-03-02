
PYTHON_CONFIG ?= python3-config

CPP_TEST_FILES = $(shell find ./ -type f -name '*_test.cpp')
PY_C_FLAGS = $(shell $(PYTHON_CONFIG) --cflags)
PY_LD_FLAGS = $(shell $(PYTHON_CONFIG) --ldflags)
PY_LIBRARY_PATH = $(shell $(PYTHON_CONFIG) --prefix)/lib
PY_EXTRA_LD_FLAGS = -L$(PY_LIBRARY_PATH)

CXX ?= g++
LDSHARED ?= g++

STD = -std=c++1z

test: test-cpp test-py

test-py: install
	py.test -s tests

doc:
	(cd docs && make html)

install:
	(cd pyx && python setup.py install)

build:
	(cd pyx && python setup.py build)

clean: doc-clean
	(cd pyx && python setup.py clean)
	find ./ -name *.pyc -delete	
	rm *.o || true
	rm run-tests || true

doc-clean:
	(cd docs && make clean)

test-cpp: run-tests
	LD_LIBRARY_PATH=$(PY_LIBRARY_PATH) ./run-tests

run-tests: test-run.o $(CPP_TEST_FILES)
	$(CXX) $(STD) $(PY_C_FLAGS) -fPIC -Wno-undefined-internal -Ivendor/  -o run-tests test-run.o $(CPP_TEST_FILES) $(PY_EXTRA_LD_FLAGS) $(PY_LD_FLAGS)

test-run.o:
	$(CXX) $(STD) $(PY_C_FLAGS) -fPIC -Ivendor/ -c -o test-run.o src/test.cpp

.PHONY: build