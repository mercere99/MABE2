# Main (working) build options:
#  native (default) - Optimized version of code. (38 sec)
#  debug - turn on all debugging options, including asserts and pointer tracking.  (37 sec)
#  quick - no debugging or optimization; fastest compile time. (21 sec)
#  clean - remove all compilation artifacts including executable.
#
# Other build options, more rarely used:
#  grumpy - Lots of extra warnings turned on
#  noblock - Same as native, but "blocking" debug code is still allowed.

MABE_DIR := ..
include $(MABE_DIR)/Makefile-base.mk

TARGET := MABE
CLEAN_EXTRA = $(TARGET)

default: native

native: FLAGS := $(FLAGS_OPT)
native: $(TARGET)

debug: FLAGS := $(FLAGS_DEBUG)
debug: $(TARGET)

grumpy: FLAGS := $(FLAGS_GRUMPY)
grumpy: $(TARGET)

quick: FLAGS := $(FLAGS_QUICK)
quick: $(TARGET)

$(TARGET): $(TARGET).cpp ../source/modules.hpp
	$(CXX) $(FLAGS) $(TARGET).cpp -o $(TARGET)

new: clean
new: native
