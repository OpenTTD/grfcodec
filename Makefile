# Makefile for NFORenum

# NFORenum requires some boost headers
# As of the current writing, only current_function.hpp is required
# It is included in the source distribution.
# If you have boost, set BOOST_INCLUDE below to the directory containing
#	your boost headers.
# If you do not have boost, create a boost/ directory and place
#	current_function.hpp in it. Set BOOST_INCLUDE to the directory
#	containing the boost directory. Or get boost: www.boost.org
BOOST_INCLUDE=/usr/include/boost-1_33_1

# Gnu compiler settings
SHELL = /bin/sh
CC = g++
CXX = g++

# use 386 instructions but optimize for pentium II/III
CFLAGS = -g -DWIN32 -O3 -I$(BOOST_INCLUDE) $(CFLAGAPP) -Wall
CXXFLAGS = $(CFLAGS)
#LDOPT = -g -Wl,--subsystem,console -luser32 -lgdi32 -lwinmm -lcomdlg32 -lcomctl32
#LDOPT = -Wl,--subsystem,console,-s
#LDOPT += -Wl,-Map,$(@:%=%.map)		# to make map files

# for profiling
#CFLAGS += -pg
#LDOPT += -pg

# OS detection: Cygwin vs Linux
ISCYGWIN = $(shell [ ! -d /cygdrive/ ]; echo $$?)

# OS dependant variables
NFORENUM = $(shell [ \( $(ISCYGWIN) -eq 1 \) ] && echo renum.exe || echo renum)


# sources to be compiled and linked
NFORENUMSRC=IDs.cpp act0.cpp act123.cpp act123_classes.cpp act5.cpp act6.cpp \
  act79D.cpp actB.cpp actF.cpp command.cpp data.cpp globals.cpp inject.cpp \
  messages.cpp pseudo.cpp rangedint.cpp renum.cpp sanity.cpp strings.cpp \
  utf8.cpp getopt.cpp


# default targets
all: renum

remake: clean all

renum:	$(NFORENUMSRC:%.cpp=%.o)
	$(CXX) -o $@ $(CFLAGS) $^ $(LDOPT)


clean:
	rm -rf *.o *.exe *.EXE renum

# Gnu compiler rules

%.o : %.c
	$(CC) -c -o $@ $(CFLAGS) $<

%.o : %.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $<


.remake_deps:
	touch .remake_deps

Makefile.dep: .remake_deps
	$(CC) $(CFLAGS) -MM *.c *.cpp | sed -e "s/[-a-zA-Z0-9_/]*boost\/[-a-zA-Z0-9_/]*\.hpp//g" -e "s/[a-zA-Z0-9_]*\.cpp//g" > $@

include Makefile.dep
