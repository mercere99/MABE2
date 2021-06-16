TEST_NAMES= core

default: test

test-%:
	cd $(@:test-%=%) && make test

test: $(addprefix test-, $(TEST_NAMES))
