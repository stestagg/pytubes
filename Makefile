
PYTHON = python3
PYTHON_CONFIG ?= python3-config

LIB_VERSION = $(shell $PYTHON tools/version.py)

CPP_TEST_FILES = $(shell find ./src -type f -name '*_test.cpp')
HPP_FILES = $(shell find ./ -type f -name '*.hpp')

PY_C_FLAGS = $(shell $(PYTHON_CONFIG) --cflags)
PY_LD_FLAGS = $(shell $(PYTHON_CONFIG) --ldflags)
PY_LIBRARY_PATH = $(shell $(PYTHON_CONFIG) --prefix)/lib
PY_EXTRA_LD_FLAGS = -L$(PY_LIBRARY_PATH)

BUILD_DIR = build/make

CXX ?= g++ -O2
CC ?= gcc -O2
LDSHARED ?= g++ -dead_strip_dylibs -shared
CFLAGS ?= '-msse4'

STD = -std=c++11


test: test-cpp test-py


test-py: build/tubes.so
	PYTHONPATH=build py.test -s test


test-double: build/tubes.so
	PYTHONPATH=build gdb --args python -m pytest -s test/test_pa.py -k test_doubles


doc: install
	(cd docs && make html)

install:
	${PYTHON} setup.py install

build:
	CFLAGS="-Wno-unused-variable" ${PYTHON} setup.py build

clean: clean-doc
	${PYTHON} setup.py clean
	-rm -rf build
	-rm -rf dist
	-rm pyx/iter_defs.pxi
	-rm pyx/*.html
	find ./ -name *.pyc -delete
	-find ./ -name __pycache__ -exec rm -r {} ';'
	-rm -rf pytubes.egg-info

clean-doc:
	-(cd docs && make clean)

test-cpp: build/run-tests
	./build/run-tests

build/run-tests: $(BUILD_DIR)/test-run.o $(CPP_TEST_FILES) $(HPP_FILES)
	LD_LIBRARY_PATH=$(PY_LIBRARY_PATH) $(CXX) $(STD) $(PY_C_FLAGS) \
		-fPIC \
		-Ivendor/ \
		-Ivendor/arrow/cpp/src \
		-o build/run-tests \
		$(BUILD_DIR)/test-run.o \
		$(CPP_TEST_FILES) \
		$(PY_EXTRA_LD_FLAGS) \
		$(PY_LD_FLAGS)

$(BUILD_DIR)/test-run.o: src/test.cpp $(BUILD_DIR)
	$(CXX) \
		$(STD) \
		$(PY_C_FLAGS) \
		-fPIC \
		-Ivendor/ \
		-c \
		-o $(BUILD_DIR)/test-run.o \
		src/test.cpp

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)


build/tubes.so: $(BUILD_DIR)/tubes.o $(BUILD_DIR)/zlib.a $(BUILD_DIR)/double.a $(BUILD_DIR)/arrow.a
	$(LDSHARED) \
		$(PY_LD_FLAGS) \
		-o build/tubes.so \
		$(BUILD_DIR)/tubes.o \
		$(BUILD_DIR)/arrow.a \
		$(BUILD_DIR)/double.a \
		$(BUILD_DIR)/zlib.a


$(BUILD_DIR)/tubes.o: pyx/tubes.cpp $(BUILD_DIR)
	$(eval NUMPY := $(shell python3 -c 'import numpy; print(numpy.get_include())'))
	$(CXX) \
		$(PY_C_FLAGS) \
		-I$(NUMPY) \
		-Ivendor/ \
		-Ivendor/zlib \
		-Ivendor/arrow/cpp/src \
		-msse4 \
		-fPIC \
		-c \
		-o $@ \
		$<


HPP_FILES = $(shell find src/ -type f -name '*.hpp')


pyx/tubes.cpp: $(shell find pyx/ -type f -name '*.p*') pyx/iter_defs.pxi $(HPP_FILES)
	cython --cplus -3 pyx/tubes.pyx 


ITER_FILES = $(shell find src/iters/ -type f -name '*.hpp')


pyx/iter_defs.pxi: $(ITER_FILES)
	python3 tools/make_cdef.py $(ITER_FILES) > pyx/iter_defs.pxi


ZLIB_SOURCES = $(shell find vendor/zlib -maxdepth 1 -type f -name '*.c')
ZLIB_OBJECTS = $(patsubst vendor/zlib/%.c,$(BUILD_DIR)/zlib/%.o,$(ZLIB_SOURCES))

$(BUILD_DIR)/zlib.a : $(ZLIB_OBJECTS)
	ar rcs $(BUILD_DIR)/zlib.a $(ZLIB_OBJECTS)

$(BUILD_DIR)/zlib/%.o: vendor/zlib/%.c
	mkdir -p $(shell dirname $@)
	$(CC) -msse4 -Ivendor/zlib -fPIC -c -o $@ $<

ARROW_SOURCES = $(shell find vendor/arrow/cpp/src/arrow \
	-type f \
	-name '*.cc' \
	-not -name '*test*' \
	-not -name 'file_parquet.cc' \
	-not -name 'hdfs*.cc' \
	-not -name 'compression*.cc' \
	-not -name 'feather.cc' \
	-not -name 'uri.cc' \
	-not -name '*benchmark*' \
	-not -path '*/filesystem/*' \
	-not -path '*/flight/*' \
	-not -path '*/gpu/*' \
	-not -path '*/compute/*' \
	-not -path '*/dbi/*' \
	-not -path '*/ipc/*' \
	-not -path '*/testing/*' \
	-not -path '*/json/*' \
	-not -path '*/python/*' \
	-not -path '*adapters*'\
)
ARROW_OBJECTS = $(patsubst vendor/arrow/cpp/src/arrow/%.cc,$(BUILD_DIR)/arrow/%.o,$(ARROW_SOURCES))
ARROW_CONFIG_H = vendor/arrow/cpp/src/arrow/util/config.h


$(BUILD_DIR)/arrow.a: $(ARROW_OBJECTS)
	ar rcs $(BUILD_DIR)/arrow.a $(ARROW_OBJECTS)

$(BUILD_DIR)/arrow/%.o: vendor/arrow/cpp/src/arrow/%.cc $(ARROW_CONFIG_H)
	mkdir -p $(shell dirname $@)
	$(CXX) $(STD) -Ivendor/double-conversion -Ivendor/arrow/cpp/src -fPIC -c -o $@ $<

$(ARROW_CONFIG_H): tools/arrow.config.h
	cp tools/arrow.config.h $(ARROW_CONFIG_H)


DOUBLE_SOURCES = $(shell find vendor/double-conversion/double-conversion \
	-type f \
	-name '*.cc' \
	-not -name '*test*' \
)
DOUBLE_OBJECTS = $(patsubst vendor/double-conversion/double-conversion/%.cc,$(BUILD_DIR)/double/%.o,$(DOUBLE_SOURCES))

$(BUILD_DIR)/double.a: $(DOUBLE_OBJECTS)
	ar rcs $(BUILD_DIR)/double.a $(DOUBLE_OBJECTS)

$(BUILD_DIR)/double/%.o: vendor/double-conversion/double-conversion/%.cc
	mkdir -p $(shell dirname $@)
	$(CXX) $(STD) -Ivendor/double-conversion/double-conversion -fPIC -c -o $@ $<


.PHONY: build install test
