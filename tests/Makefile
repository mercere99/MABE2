.PHONY: regression unit test coverage

default: test

unit: $(addprefix test-, $(DIR_NAMES))
	cd unit && make test

test: unit regression 

regression:
	cd regression && make test

cov-%:
	cd $(@:cov-%=%) && make coverage

coverage: coverage_conversion $(addprefix cov-, $(DIR_NAMES))

coverage_conversion:
	./convert_for_tests.sh
