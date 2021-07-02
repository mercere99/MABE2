TEST_NAMES= core evaluate

default: test

test-%:
	cd $(@:test-%=%) && make test

test: $(addprefix test-, $(TEST_NAMES))

cov-%:
	cd $(@:cov-%=%) && make coverage

coverage: $(addprefix cov-, $(TEST_NAMES))