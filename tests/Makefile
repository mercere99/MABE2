TEST_NAMES= component 

default: test

test-%:
	cd $(@:test-%=%) && make test

test: $(addprefix test-, $(TEST_NAMES))

cov-%:
	cd $(@:cov-%=%) && make coverage

coverage: coverage_conversion $(addprefix cov-, $(TEST_NAMES))

coverage_conversion:
	./convert_for_tests.sh
