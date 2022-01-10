DIR_NAMES= analyze core evaluate interface orgs placement schema select tools #config
# TODO: Re add config after we merge and write new tests

default: test

test-%:
	cd $(@:test-%=%) && make test

test: $(addprefix test-, $(DIR_NAMES))

cov-%:
	cd $(@:cov-%=%) && make coverage

coverage: $(addprefix cov-, $(DIR_NAMES))
