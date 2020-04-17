MAKEFLAGS += --no-print-directory

CXX = $(shell sst-config --CXX)
CXXFLAGS  = $(shell sst-config --ELEMENT_CXXFLAGS)
INCLUDES  =
LDFLAGS   = $(shell sst-config --ELEMENT_LDFLAGS)
LIBRARIES =

SRC = $(wildcard *.cc */*.cc)
#Exclude these files from default compilation
SRCS = $(filter-out generators/stake.cc, $(SRC))
OBJ = $(SRCS:%.cc=.build/%.o)
DEP = $(OBJ:%.o=%.d)

.PHONY: all checkOptions install uninstall clean

all: checkOptions install

checkOptions:
ifdef stake
    INCLUDES  += -I$(stake)
    LIBRARIES += -L$(stake) -lstake
    $(shell sst-register stake stake_LIBDIR=$(stake))
    SRCS += generators/stake.cc
endif

.PHONY: install
.ONESHELL:
install:
	mkdir -p build
	cd build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. && make
	sst-register miranda miranda_LIBDIR=$(CURDIR)

uninstall:
	sst-register -u miranda

clean: uninstall
	rm -rf build libmiranda.so
