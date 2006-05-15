# Makefile for GRFCodec using gcc on Linux, or on Cygwin in Cygwin mode.

include Makefile.setup

# Makefile name, so we can reference it in Makefile.common
MAKEFILE = Makefile

# OS dependent variables
NASMFORMAT = $(shell [ \( $(ISCYGWIN) -eq 1 \) ] && echo coff || echo elf )
REL_EXE = $(shell [ \( $(ISCYGWIN) -eq 1 \) ] && echo .exe )

# Borland compiler settings
BCC=bcc32
COPTS=-a4 -5 -d -f- -k- -y -u -v -y -H- -K -r -j7 -I. -WC -O2 -OS -Oi -Ov
CPPOPTS=$(COPTS) -P
LOPTS=-v -y -ls -f-

# Gnu compiler settings
SHELL = /bin/sh
CC = g++
CXX = g++
# use 386 instructions but optimize for pentium II/III
CFLAGS = -g -DWIN32 -O3 -I. $(BOOST_CMD) $(CFLAGAPP) -Wall
CXXFLAGS = $(CFLAGS)
#LDOPT = -g -Wl,--subsystem,console -luser32 -lgdi32 -lwinmm -lcomdlg32 -lcomctl32
#LDOPT = -Wl,--subsystem,console,-s
#LDOPT += -Wl,-Map,$(@:%=%.map)		# to make map files

# for profiling
#CFLAGS += -pg
#LDOPT += -pg

include Makefile.common

