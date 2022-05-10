EMP_DIR := ../../third-party/empirical/include
MABE_DIR := ../../
UAT_DIR := ../../../../../universal_avida_checks/interpreters

# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wextra -Wno-unused-function -I$(EMP_DIR)/ -I$(MABE_DIR)/ -I$(UAT_DIR)/
CFLAGS_version := -std=c++17

CXX_native := g++

OFLAGS_native_opt := -O3 -DNDEBUG -DEMP_DNDEBUG
OFLAGS_native_debug := -g -pedantic -Wnon-virtual-dtor -Wcast-align -DEMP_TRACK_MEM
OFLAGS_native_grumpy := -g -pedantic -Wnon-virtual-dtor -Wcast-align -Wconversion -Weffc++

CFLAGS_native_opt := $(CFLAGS_all) $(OFLAGS_native_opt)
CFLAGS_native_noblock := $(CFLAGS_all) $(OFLAGS_native_opt) -DEMP_NO_BLOCK
CFLAGS_native_debug := $(CFLAGS_all) $(OFLAGS_native_debug)
CFLAGS_native_grumpy := $(CFLAGS_all) $(OFLAGS_native_grumpy)

TARGETS := UATInterpreter

default: native

CXX := $(CXX_native)
CFLAGS := $(CFLAGS_native_opt)

debug: CFLAGS := $(CFLAGS_native_debug)
debug: all

profile: CFLAGS := $(CFLAGS_native_opt) -pg
profile: all

noblock: CFLAGS := $(CFLAGS_native_noblock)
noblock: all

grumpy: CFLAGS := $(CFLAGS_native_grumpy)
grumpy: all


native: all

all: $(TARGETS)

$(TARGETS): % : %.cpp
	$(CXX) $(CFLAGS_version) $(CFLAGS) $< -o $@

debug-%: $*.cpp
	$(CXX) $(CFLAGS_version) $(CFLAGS_native_debug) $< -o $@

clean:
	rm -rf debug-* *~ *.dSYM $(TARGETS)
#	rm -rf debug-* *~ *.dSYM $(JS_TARGETS)

new: clean
new: native

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
