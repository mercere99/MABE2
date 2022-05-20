TEST_NAMES= EvalMancala EvalPathFollow EvalDoors

MABE_DIR= ../../../../source
EMP_DIR= ../../../../source/third-party/empirical
COV_DIR=../../
CATCH_DIR= $(EMP_DIR)/third-party/Catch/single_include/catch2
FLAGS= -std=c++17 -pthread -Wall -g -DEMP_TRACK_MEM -Wno-unused-function -Wno-unused-private-field -I$(EMP_DIR)/include/ -I$(CATCH_DIR) -I$(MABE_DIR)

default: test

######## TESTING
test-prep:
	mkdir -p temp

test-%: %.cpp $(CATCH_DIR)/catch.hpp
	g++ $(FLAGS) $< -o $@.out
	./$@.out

test: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf temp

######## CODE COVERAGE
cov-%: %.cpp ${EMP_DIR}/third-party/Catch/single_include/catch2/catch.hpp
	$(CXX) $(FLAGS) $< -o $@.out
	./$@.out
	llvm-profdata merge default.profraw -o default.profdata
	llvm-cov show ./$@.out -instr-profile=default.profdata > coverage_$@.txt
	python ${EMP_DIR}/third-party/force-cover/fix_coverage.py coverage_$@.txt

coverage: FLAGS := -std=c++17 -pthread -g -Wall -Wno-unused-function -I${CATCH_DIR} -I${MABE_DIR} -I${EMP_DIR}/include -I${EMP_DIR}/third-party/cereal/include/ -DEMP_TRACK_MEM -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -ftemplate-backtrace-limit=0 -fprofile-instr-generate -fcoverage-mapping -fno-inline -fno-elide-constructors -O0

coverage: test-prep $(addprefix cov-, $(TEST_NAMES))

clean: 
	rm -f *.out
	rm -f *.o
	rm -f *.gcda
	rm -f *.gcno
	rm -f *.info
	rm -f *.gcov 
	rm -f ./Coverage* 
	rm -rf ./temp

