DIR_NAMES= config core evaluate analyze interface placement

default: test

test-%:
	cd $(@:test-%=%) && make test

test: $(addprefix test-, $(DIR_NAMES))

cov-%:
	cd $(@:cov-%=%) && make coverage

coverage: $(addprefix cov-, $(DIR_NAMES))
