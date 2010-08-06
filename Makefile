# =========================================================
# Makefile for GRFCodec
#
#	Don't put any local configuration in here
#	Change Makefile.local instead, it'll be
#	preserved when updating the sources
# =========================================================

MAKEFILELOCAL=Makefile.local

# Order of the palettes compiled in
# (note, this must match the text in grfcodec.cc and grftut.txt)
PALORDER = ttd_norm&ttw_norm&ttd_cand&ttw_cand&tt1_norm&tt1_mars&ttw_pb_pal1&ttw_pb_pal2

# Gnu compiler settings
SHELL = /bin/sh
CC = g++
CXX = g++
STRIP = strip
UPX = $(shell [ `which upx 2>/dev/null` ] && echo "upx")
AWK = awk
SRCZIP_FLAGS = -9
SRCZIP = gzip

# Default installation directories
INSTALL_DOCS_DIR := "$(INSTALL_DIR)/usr/share/doc/grfcodec"
INSTALL_BINARY_DIR := "$(INSTALL_DIR)/usr/bin"
INSTALL_MAN_DIR := "$(INSTALL_DIR)/usr/share/man/man1"

# OS detection: Cygwin vs Linux
ISCYGWIN = $(shell [ ! -d /cygdrive/ ]; echo $$?)
MACHINE = $(shell $(CC) -dumpmachine || echo '??' )

# Cygwin builds default to -mno-cygwin.
ifndef NOCYGWIN
NOCYGWIN = 1
endif

# OS dependent variables
EXE = $(shell ( [ \( $(ISCYGWIN) -eq 1 \) -o \( "$(MACHINE)" = "mingw32" \) ] ) && echo .exe)
GRFCODEC = grfcodec$(EXE)
GRFDIFF  = grfdiff$(EXE)
GRFMERGE = grfmerge$(EXE)
GRFID    = grfid$(EXE)

TYPESIZE = GCC32

-include ${MAKEFILELOCAL}

# GCC 4.5.0 has an optimisation bug that influences GRFCodec.
# As such we disable optimisation when GCC 4.5.0 is detected.
# The issue has been fixed in GCC 4.5.1
ifndef CFLAGOPT
ifeq ($(shell $(CC) -v 2>&1 | grep "4\.5\.0" || true),)
CFLAGOPT = -O2
else
CFLAGOPT = -O0
endif
endif


CFLAGS  = -g -D$(TYPESIZE) -I. -idirafter$(BOOST_INCLUDE) -D_FORTIFY_SOURCE=2
CFLAGS += -Wall -Wno-uninitialized -Wsign-compare -Wwrite-strings -Wpointer-arith -W -Wno-unused-parameter -Wformat=2 -Wredundant-decls
CFLAGS += $(CFLAGOPT) $(CFLAGAPP)

ifeq ($(DEBUG),1)
CFLAGS += -DDEBUG
endif

ifeq ($(MACHINE),mingw32)
CFLAGS += -DMINGW
endif

ifeq ($(ISCYGWIN),1)
ifeq ($(NOCYGWIN),1)
CFLAGS += -DMINGW -mno-cygwin
endif
endif

ifeq ($(shell uname),Darwin)
CFLAGS += -isystem/opt/local/include
endif

CXXFLAGS = $(CFLAGS)


# Somewhat automatic detection of the correct boost include folder
ifndef BOOST_INCLUDE
BOOST_INCLUDE=$(shell \
find /usr/include /usr/local/include /opt/local/include -maxdepth 1 -name 'boost-*' 2> /dev/null | sort -t - -k 2 | tail -n 1 )
ifeq ($(BOOST_INCLUDE),)
BOOST_INCLUDE=$(shell \
( [ -d /usr/include/boost/date_time ] && echo /usr/include ) || \
( [ -d /usr/local/include/boost/date_time ] && echo /usr/local/include ) || \
( [ -d /opt/local/include/boost/date_time ] && echo /opt/local/include ) )
endif
endif

ifeq ($(BOOST_INCLUDE),)
BOOST_ERROR = echo Error: Boost not found. Compilation will fail.
endif

ifndef V
V=0 # verbose build default off
endif

# =======================================================================
#           setup verbose/non-verbose make process
# =======================================================================

# _E = prefix for the echo [TYPE] TARGET
# _C = prefix for the actual command(s)
# _I = indentation for sub-make
# _Q = number of 'q's for UPX
# _S = sub-makes silent?
ifeq (${V},1)
	# verbose, set _C = nothing (print command), _E = comment (don't echo)
	_C=
	_E=@\#
	_Q=-qq
	_S=
else
	# not verbose, _C = @ (suppress cmd line), _E = @echo (echo type&target)
	_C=@
	_E:=@echo ${_I}
	_Q=-qqq
	_S=-s
endif

# increase indentation level for sub-makes
_I := ${_I}"	"
export _I

# standard compilation commands should be of the form
# target:	prerequisites
#	${_E} [CMD] $@
#	${_C}${CMD} ...arguments...
#
# non-standard commands (those not invoked by make all/dos/win) should
# use the regular syntax (without the ${_E} line and without the ${_C} prefix)
# because they'll be only used for testing special conditions
#
# =======================================================================

# sources to be compiled and linked
GRFCODECSRC=grfcomm.c pcxfile.c sprites.c pcxsprit.c info.c \
	error.c getopt.c path.c readinfo.c file.c grfcodec.c

GRFDIFFSRC=grfcomm.c error.c sprites.c getopt.c grfdiff.c path.c

GRFMERGESRC=grfcomm.c error.c getopt.c grfmerge.c path.c

GRFIDSRC=grfid.c

PAL_FILES = pals/$(subst &,.bcp pals/,$(PALORDER)).bcp

# deafult targets
all: $(GRFCODEC) $(GRFDIFF) $(GRFMERGE) $(GRFID)
remake:
	$(_E) [CLEAN]
	$(_C)$(MAKE) ${_S} clean
	$(_E) [REBUILD]
	$(_C)$(MAKE) ${_S} all

${MAKEFILELOCAL}:
	@/bin/sh -c "export PATH=\"/bin\" && \
	echo ${MAKEFILELOCAL} did not exist, using defaults. Please edit it if compilation fails. && \
	cp ${MAKEFILELOCAL}.sample $@"

$(GRFCODEC): $(GRFCODECSRC:%.c=objs/%.o)
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(CFLAGS) $^ $(LDOPT)

$(GRFDIFF):  $(GRFDIFFSRC:%.c=objs/%.o) objs/grfmrg.o
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(CFLAGS) $^ $(LDOPT)

$(GRFMERGE): $(GRFMERGESRC:%.c=objs/%.o)
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(CFLAGS) $^ $(LDOPT)

$(GRFID): $(GRFIDSRC:%.c=objs/%.o)
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(CFLAGS) $^ $(LDOPT)


clean:
	rm -rf objs $(GRFCODEC) $(GRFDIFF) $(GRFMERGE) $(GRFID) bundle bundles

mrproper: clean
	rm -f *.d src/version.h src/grfmrg.c
	@touch -ct 9901010000 ttdpal.h	# don't delete it, so we don't confuse svn, but force it to be remade

FORCE:
	@$(BOOST_ERROR)

include version.def

src/version.h: FORCE
	@echo // Autogenerated by make.  Do not edit.  Edit version.def or the Makefile instead. > $@.tmp
	@echo "#define GRFCODECVER \"$(VERSIONSTR)\"" >> $@.tmp
	@(diff $@.tmp $@ > /dev/null 2>&1 && rm -f $@.tmp) || (rm -f $@ ; mv $@.tmp $@ || true)

FORCE:
%_r: FORCE
	$(_E) [REBUILD] $(@:%_r=%)
	$(_C)rm -f $(@:%_r=%)
	$(_C)$(MAKE) ${_S} $(@:%_r=%)
	$(_E) [STRIP] $(@:%_r=%)
	$(_C)$(STRIP)  $(@:%_r=%)
ifneq ($(UPX),)
	$(_E) [UPX] $(@:%_r=%)
	$(_C)$(UPX) $(_Q) --best  $(@:%_r=%)
endif

release: FORCE $(GRFCODEC)_r $(GRFDIFF)_r $(GRFMERGE)_r $(GRFID)_r

# make grfmerge.exe (as grfmrgc.bin) optimized for size instead of speed
objs/grfmrgc.bin: objs/grfmerge.os $(GRFMERGESRC:%.c=objs/%.os)
	$(_C)rm -f $@
	$(_E) [LD] $@
	$(_C)$(CC) -o $@ $(CFLAGS) -Os $^
	$(_E) [STRIP] $@
	$(_C)$(STRIP) $@
ifneq ($(UPX),)
	$(_E) [UPX] $@
	$(_C)$(UPX) $(_Q) --best $@
endif

src/grfmrg.c: objs/grfmrgc.bin src/grfmrgc.pl
	$(_E) [PERL] $@
	$(_C)perl -w src/grfmrgc.pl > $@

src/ttdpal.h: $(PAL_FILES:%=src/%) src/pal2c.pl
	$(_E) [PERL] $@
	$(_C)perl src/pal2c.pl $(PAL_FILES:%=src/%) > $@

# Gnu compiler rules

objs/%.o : src/%.c
	$(_E) [CC] $@
	$(_C)$(CC) -c -o $@ -MMD -MF $@.d $(CFLAGS) $<

objs/%.o : src/%.cc
	$(_E) [CPP] $@
	$(_C)$(CXX) -c -o $@ -MMD -MF $@.d $(CXXFLAGS) $(BOOST_CMD) $<

% : objs/%.o
	$(_E) [LD] $@
	$(_C)$(CC) -o $@ $(CFLAGS) $^ $(LDOPT)
	$(_C)$(CP_TO_EXE)

objs/%.S : src/%.c
	$(_E) [CC] $@
	$(_C)$(CC) -S -o $@ $(CFLAGS) $<

objs/%.S : src/%.cc
	$(_E) [CPP] $@
	$(_C)$(CC) -S -o $@ $(CXXFLAGS) $<


# same as above but optimized for size not speed
objs/%.os : src/%.c
	$(_E) [CC] $@
	$(_C)$(CC) -c -o $@ -MMD -MF $@.d $(CFLAGS) -Os $<

% :: objs/%.os
	$(_E) [LD] $@
	$(_C)$(CC) -o $@ $(CFLAGS) $^ $(LDOPT)

# Borland compiler rules

objs/%.obj : src/%.c
	$(_E) [CC] $@
	$(_C)$(BCC) $(COPTS) -c $< -o$@

objs/%.obj : src/%.cc
	$(_E) [CPP] $@
	$(_C)$(BCC) -P $(CPPOPTS) -c $< -o$@

%.exe : objs/%.obj
	$(_E) [LD] $@
	$(_C)$(BCC) $(LOPTS) $^ -e$@

# On some installations a version.h exists in /usr/include. This one is then
# found by the dependency tracker and thus the dependencies do not contain
# a reference to version.h, so it isn't generated and compilation fails.
objs/%.o.d: src/version.h
	$(_C)mkdir -p objs
	$(_E) [CPP DEP] $@
	$(_C)$(CC) $(CFLAGS) -DMAKEDEP -MM -MG src/$*.c* -MF $@

objs/%.os.d: src/version.h
	$(_C)mkdir -p objs
	$(_E) [CPP DEP] $@
	$(_C)$(CC) $(CFLAGS) -DMAKEDEP -MM -MG -MT ${subst .d,,$@} -MF $@ src/$*.c*

objs/%.obj.d: $(wildcard src/%.c*) src/version.h
	$(_C)mkdir -p objs
	$(_E) [CPP DEP] $@
	$(_C)$(CC) $(CFLAGS) -DMAKEDEP -MM -MG -MT ${subst .d,,$@} -MF $@ src/$*.c*

ifndef NO_MAKEFILE_DEP
-include $(GRFCODECSRC:%.c=objs/%.o.d)
-include $(GRFMERGESRC:%.c=objs/%.o.d)
-include $(GRFDIFFSRC:%.c=objs/%.o.d)
-include $(GRFIDSRC:%.c=objs/%.o.d)
-include $(GRFMERGESRC:%.c=objs/%.os.d)
-include $(GRFCODECSRC:%.c=objs/%.obj.d)
-include $(GRFMERGESRC:%.c=objs/%.obj.d)
-include $(GRFDIFFSRC:%.c=objs/%.obj.d)
-include $(GRFIDSRC:%.c=objs/%.obj.d)
endif

include Makefile.bundle
