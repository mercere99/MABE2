PROJECT := MABE
FILES := Organism.h
EMP_DIR := ../../../Empirical

# Compiler type and flags
CXX := g++
CFLAGS_base := -Wall -Wno-unused-function -std=c++17 -I$(EMP_DIR)/source/
CFLAGS_debug := -g $(CFLAGS_base)
CFLAGS_release := -O3 -DNDEBUG $(CFLAGS_base)

%.h: %.emph
	$(EMP_DIR)/apps/Emphatic/Emphatic $< $@

default: $(PROJECT)
all: $(PROJECT)

# If in debug mode, swap out the optimization flags to be debug flags.
debug:	$(FILES)
			echo Done!
#			$(CXX) $(CFLAGS_debug) $(PROJECT).cc -o $(PROJECT)

$(PROJECT): $(FILES)
			echo Done!
#			$(CXX) $(CFLAGS_release) $(PROJECT).cc -o $(PROJECT)


clean:
	rm -rf $(PROJECT).dSYM/ *~ $(FILES)

test: debug
	echo Need to implment tests!
#	$(CXX) $(CFLAGS_debug) examples/ConceptTest.cc -o examples/ConceptTest

# Debugging information
print-%: ; @echo '$(subst ','\'',$*=$($*))'
