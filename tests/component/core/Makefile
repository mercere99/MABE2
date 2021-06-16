TEST_NAMES= MABE

EMP_DIR= ../../../source/third-party/empirical
CATCH_DIR= $(EMP_DIR)/third-party/Catch/single_include/catch2
FLAGS= -std=c++17 -pthread -Wall -Wno-unused-function -Wno-unused-private-field -I$(EMP_DIR)/include/ -I$(CATCH_DIR)

default: test

test-prep:
	mkdir -p temp

test-%: %.cpp $(CATCH_DIR)/catch.hpp
	g++ $(FLAGS) $< -o $@.out
	./$@.out

test: test-prep $(addprefix test-, $(TEST_NAMES))
	rm -rf temp

clean: 
	rm -f *.out
	rm -f *.o
	rm -f *.gcda
	rm -f *.gcno
	rm -f *.info
	rm -f *.gcov 
	rm -f ./Coverage* 
	rm -rf ./temp

