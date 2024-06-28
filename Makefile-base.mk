# This is the MABE base Makefile that sets up variables for all of the others to include.
# To use, you must first setup MABE_DIR to indicate the offset of the MABE root directory.

# Identify all directory locations
EMP_DIR   = $(MABE_DIR)/source/third-party/empirical
#EMP_DIR   = $(MABE_DIR)/../Empirical
CATCH_DIR = $(EMP_DIR)/third-party/Catch/single_include/catch2

# Specify sets of compilation flags to use
FLAGS_version := -std=c++20
FLAGS_warn    = -Wall -Wextra -Wno-unused-function -Wnon-virtual-dtor -Wcast-align -Woverloaded-virtual -pedantic
FLAGS_include = -I$(EMP_DIR)/include/ -I$(CATCH_DIR) -I$(MABE_DIR)/source/ -I${EMP_DIR}/third-party/cereal/include/
FLAGS_main    = $(FLAGS_version) $(FLAGS_warn) $(FLAGS_include) -pthread

FLAGS_QUICK  = $(FLAGS_main) -DNDEBUG
FLAGS_DEBUG  = $(FLAGS_main) -g -DEMP_TRACK_MEM
FLAGS_OPT    = $(FLAGS_main) -O3 -DNDEBUG
FLAGS_GRUMPY = $(FLAGS_main) -DNDEBUG -Wconversion -Weffc++
FLAGS_EMSCRIPTEN = --js-library $(EMP_DIR)/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1  -s TOTAL_MEMORY=67108864
FLAGS_COVERAGE = $(FLAGS_main)  -O0 -DEMP_TRACK_MEM -ftemplate-backtrace-limit=0 -fprofile-instr-generate -fcoverage-mapping -fno-inline -fno-elide-constructors

CXX = g++

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'

CLEAN_BACKUP = *~ *.dSYM
CLEAN_TEST = *.out	*.o	*.gcda	*.gcno	*.info	*.gcov	./Coverage* ./temp
CLEAN_EXTRA =

CLEAN_FILES = $(CLEAN_BACKUP) $(CLEAN_TEST) $(CLEAN_EXTRA)

clean:
	@echo About to remove:
	@echo $(wildcard $(CLEAN_FILES))
	@echo ----
	rm -rI $(wildcard $(CLEAN_FILES))
