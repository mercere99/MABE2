DIR_NAMES= component regression# Add additional directories here 

default: test

test-%:
	cd $(@:test-%=%) && make test

test: $(addprefix test-, $(DIR_NAMES))

cov-%:
	cd $(@:cov-%=%) && make coverage

coverage: coverage_conversion $(addprefix cov-, $(DIR_NAMES))

coverage_conversion:
	./convert_for_tests.sh
