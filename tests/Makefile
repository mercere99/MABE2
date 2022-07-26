.PHONY: regression unit test coverage

default: test

unit: 
	cd unit && make test

test: unit regression 

regression:
	cd regression && make test

cov-%:
	cd $(@:cov-%=%) && make coverage

coverage: coverage_conversion
	cd coverage_source && make coverage

coverage_conversion:
	./convert_for_tests.sh

