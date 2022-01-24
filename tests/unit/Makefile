DIR_NAMES= analyze core evaluate orgs placement select tools Emplode 

default: test

test-%:
	cd $(@:test-%=%) && make test

test: $(addprefix test-, $(DIR_NAMES))

cov-%:
	cd $(@:cov-%=%) && make coverage

coverage: $(addprefix cov-, $(DIR_NAMES))
