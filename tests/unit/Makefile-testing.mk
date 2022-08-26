MABE_DIR = $(TESTING_DIR)/../..
default: test

include $(MABE_DIR)/Makefile-base.mk

FLAGS = $(FLAGS_DEBUG)

######## TESTING
test-prep:
	mkdir -p temp

test-%: %.cpp $(CATCH_DIR)/catch.hpp
	$(CXX) $(FLAGS) $< -o $@.out
	./$@.out

test: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf temp

######## CODE COVERAGE
cov-%: %.cpp ${CATCH_DIR}/catch.hpp
	$(CXX) $(FLAGS_COVERAGE) $< -o $@.out
	./$@.out
	llvm-profdata merge default.profraw -o default.profdata
	llvm-cov show ./$@.out -instr-profile=default.profdata > coverage_$@.txt
	python ${EMP_DIR}/third-party/force-cover/fix_coverage.py coverage_$@.txt

coverage: test-prep $(addprefix cov-, $(TEST_NAMES))

