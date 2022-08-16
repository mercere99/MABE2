# Main (working) build options:
#  native (default) - Optimized version of code. (38 sec)
#  debug - turn on all debugging options, including asserts and pointer tracking.  (37 sec)
#  quick - no debugging or optimization; fastest compile time. (21 sec)
#  clean - remove all compilation artifacts including executable.
#
# Other build options, more rarely used:
#  grumpy - Lots of extra warnings turned on
#  noblock - Same as native, but "blocking" debug code is still allowed.

EMP_DIR := ../source/third-party/empirical/include
MABE_DIR := ../source

# Compiler information
CXX_web := emcc
CXX_native := g++

TARGET := MABE

# Flags to use regardless of compiler
FLAGS_version := -std=c++20
FLAGS_include := -I$(EMP_DIR)/ -I$(MABE_DIR)/
FLAGS_warn := -Wall -Wextra -Wno-unused-function -Wnon-virtual-dtor -Wcast-align
FLAGS_base := $(FLAGS_version) $(FLAGS_include) $(FLAGS_warn)

FLAGS_debug := -g -pedantic -DEMP_TRACK_MEM
FLAGS_nodebug := -DNDEBUG
FLAGS_EMSCRIPTEN := --js-library $(EMP_DIR)/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1  -s TOTAL_MEMORY=67108864

FLAGS_native_opt := $(FLAGS_base) $(FLAGS_nodebug) -O3
FLAGS_native_noblock := $(FLAGS_base) $(FLAGS_nodebug) -O3 -DEMP_NO_BLOCK
FLAGS_native_debug := $(FLAGS_base) $(FLAGS_debug)
FLAGS_native_grumpy := $(FLAGS_base) $(FLAGS_debug) -Wconversion -Weffc++
FLAGS_native_quick := $(FLAGS_base) $(FLAGS_nodebug)

FLAGS_web_debug := $(FLAGS_base) $(FLAGS_EMSCRIPTEN) -g4 -pedantic -Wno-dollar-in-identifier-extension -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
FLAGS_web_opt := $(FLAGS_base) $(FLAGS_nodebug) -Os $(FLAGS_EMSCRIPTEN)
#FLAGS_web := $(FLAGS_base) $(OFLAGS_web) --js-library $(EMP_DIR)/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1

CXX := $(CXX_native)
FLAGS := $(FLAGS_native_opt)

default: native

native: $(TARGET)

debug: FLAGS := $(FLAGS_native_debug)
debug: $(TARGET)

noblock: FLAGS := $(FLAGS_native_noblock)
noblock: $(TARGET)

grumpy: FLAGS := $(FLAGS_native_grumpy)
grumpy: $(TARGET)

quick: FLAGS := $(FLAGS_native_quick)
quick: $(TARGET)

web: CXX := $(CXX_web)
web: FLAGS := $(FLAGS_web_opt)
web: $(TARGET)

web-debug: CXX := $(CXX_web)
web-debug: FLAGS := $(FLAGS_web_debug)
web-debug: $(TARGET)

$(TARGET): $(TARGET).cpp ../source/modules.hpp
	$(CXX) $(FLAGS) $(TARGET).cpp -o $(TARGET)

$(JS_TARGET): %.js : %.cpp
	$(CXX_web) $(FLAGS_web) $< -o $@

debug-%: $*.cpp
	$(CXX) $(FLAGS_native_debug) $< -o $@

clean:
	rm -rf debug-* *~ *.dSYM $(TARGET)
#	rm -rf debug-* *~ *.dSYM $(JS_TARGET)

new: clean
new: native

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
