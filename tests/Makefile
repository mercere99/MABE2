TEST_NAMES= component 

default: test

test-%:
	cd $(@:test-%=%) && make test

test: $(addprefix test-, $(TEST_NAMES))
