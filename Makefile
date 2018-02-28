
test: test-py

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

.PHONY: build