
CPP_TEST_FILES = $(shell find ./ -type f -name '*_test.cpp')
PY_C_FLAGS = $(shell python-config --cflags)
PY_LD_FLAGS = $(shell python-config --cflags --ldflags)
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
	

doc-clean:
	(cd docs && make clean)

test-cpp: run-tests
	./run-tests

run-tests: test-run.o $(CPP_TEST_FILES)
	g++ $(STD) $(PY_LD_FLAGS) -Wno-undefined-internal -Ivendor/  -o run-tests test-run.o $(CPP_TEST_FILES)

test-run.o:
	g++ $(STD) $(PY_C_FLAGS) -Ivendor/ -c -o test-run.o src/test.cpp

.PHONY: build