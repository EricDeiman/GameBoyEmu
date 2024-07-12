#HOST : = `hostname`

# if HOST is waterford, inculde the link flag -lstdc++fs
# ifeq ( $(HOST), waterford )
#   update link flags
# else
#   no need to update link flags
#

# look into using the g++ -MMD and -MP to create makefile entries for include dependencies

SRC_DIR := .
SRCS := $(shell find $(SRC_DIR) -name '*.cc')

.PHONY : deps clean
deps :
	g++ -E -std=c++17 -MMD -MP $(SRCS) > /dev/null
	$(MAKE) -f Makefile2

clean :
	rm -f *.o gbe

