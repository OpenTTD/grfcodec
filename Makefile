# =========================================================
# Makefile for GRFCodec
#
#	Don't put any local configuration in here
#	Change Makefile.local instead, it'll be
#	preserved when updating the sources
# =========================================================

MAKEFILELOCAL=Makefile.local

PACKAGE_NAME = grfcodec

# Order of the palettes compiled in
# (note, this must match the text in grfcodec.cpp and grftut.txt)
PALORDER = ttd_norm&ttw_norm&ttd_cand&ttw_cand&tt1_norm&tt1_mars&ttw_pb_pal1&ttw_pb_pal2

# Gnu compiler settings
SHELL = /bin/sh
CXX = g++
STRIP =
UPX =
AWK = awk
SRCZIP_FLAGS = -9
SRCZIP = gzip

# OS detection: Cygwin vs Linux
ISCYGWIN = $(shell [ ! -d /cygdrive/ ]; echo $$?)
MACHINE = $(shell $(CXX) -dumpmachine || echo '??' )

# OS dependent variables
EXE = $(shell ( [ \( $(ISCYGWIN) -eq 1 \) -o \( "$(MACHINE)" = "mingw32" \) ] ) && echo .exe)
GRFCODEC = grfcodec$(EXE)
GRFDIFF  = grfdiff$(EXE)
GRFMERGE = grfmerge$(EXE)
GRFID    = grfid$(EXE)

ENDIAN_CHECK = endian_check$(EXE)

TYPESIZE = GCC32

FLAGS  = -O2 -D$(TYPESIZE) -idirafter$(BOOST_INCLUDE) -D_FORTIFY_SOURCE=2
FLAGS += -Wall -Wno-uninitialized -Wsign-compare -Wwrite-strings -Wpointer-arith -W -Wno-unused-parameter -Wformat=2 -Wredundant-decls

ifeq ($(DEBUG),1)
FLAGS += -DDEBUG
endif

ifeq ($(MACHINE),mingw32)
FLAGS += -DMINGW
endif

ifeq ($(shell uname),Darwin)
FLAGS += -isystem/opt/local/include
endif

# GCC 4.5.0 has an optimisation bug that influences GRFCodec.
# As such we disable optimisation when GCC 4.5.0 is detected.
# The issue has been fixed in GCC 4.5.1
ifneq ($(shell $(CXX) -v 2>&1 | grep "4\.5\.0" || true),)
FLAGS += -O0
endif

-include ${MAKEFILELOCAL}

CXXFLAGS := $(FLAGS) $(CXXFLAGS)

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
GRFCODECSRC=grfcomm.cpp pcxfile.cpp sprites.cpp pcxsprit.cpp info.cpp \
	error.cpp path.cpp readinfo.cpp file.cpp grfcodec.cpp

GRFDIFFSRC=grfcomm.cpp error.cpp sprites.cpp grfdiff.cpp path.cpp

GRFMERGESRC=grfcomm.cpp error.cpp grfmerge.cpp path.cpp

GRFIDSRC=grfid.cpp

PAL_FILES = pals/$(subst &,.bcp pals/,$(PALORDER)).bcp

# deafult targets
all: $(GRFCODEC) $(GRFDIFF) $(GRFMERGE) $(GRFID)
remake:
	$(_E) [CLEAN]
	$(_C)$(MAKE) ${_S} clean
	$(_E) [REBUILD]
	$(_C)$(MAKE) src/version.h src/endian.h
	$(_C)$(MAKE) ${_S} all

${MAKEFILELOCAL}:
	@/bin/sh -c "export PATH=\"/bin\" && \
	echo ${MAKEFILELOCAL} did not exist, using defaults. Please edit it if compilation fails. && \
	cp ${MAKEFILELOCAL}.sample $@"

$(GRFCODEC): $(GRFCODECSRC:%.cpp=objs/%.o)
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(CXXFLAGS) $^ $(LDOPT)

$(GRFDIFF):  $(GRFDIFFSRC:%.cpp=objs/%.o) objs/grfmrg.o
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(CXXFLAGS) $^ $(LDOPT)

$(GRFMERGE): $(GRFMERGESRC:%.cpp=objs/%.o)
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(CXXFLAGS) $^ $(LDOPT)

$(GRFID): $(GRFIDSRC:%.cpp=objs/%.o)
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(CXXFLAGS) $^ $(LDOPT)


clean:
	rm -rf objs $(GRFCODEC) $(GRFDIFF) $(GRFMERGE) $(GRFID) bundle bundles grfcodec-* src/endian.h

mrproper: clean
	rm -f src/version.h src/grfmrg.cpp
	@touch -ct 9901010000 ttdpal.h	# don't delete it, so we don't confuse svn, but force it to be remade

distclean: mrproper
	rm -rf Makefile.local

FORCE:
	@$(BOOST_ERROR)

include version.def

src/version.h: FORCE
	@echo // Autogenerated by make.  Do not edit.  Edit version.def or the Makefile instead. > $@.tmp
	@echo "#define GRFCODECVER \"$(VERSIONSTR)\"" >> $@.tmp
	@(diff $@.tmp $@ > /dev/null 2>&1 && rm -f $@.tmp) || (rm -f $@ ; mv $@.tmp $@ || true)

objs/$(ENDIAN_CHECK): src/endian_check.cpp
	$(_C) mkdir -p objs
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(CXXFLAGS) src/endian_check.cpp $(LDOPT)

src/endian.h: objs/$(ENDIAN_CHECK)
	$(_E) [ENDIAN] Determining endianness
	$(_C)objs/$(ENDIAN_CHECK) $(ENDIAN_PARAMS) > src/endian.h || rm src/endian.h

FORCE:
%_r: FORCE
	$(_E) [REBUILD] $(@:%_r=%)
	$(_C)rm -f $(@:%_r=%)
	$(_C)$(MAKE) ${_S} $(@:%_r=%)
ifneq ($(STRIP),)
	$(_E) [STRIP] $(@:%_r=%)
	$(_C)$(STRIP)  $(@:%_r=%)
endif
ifneq ($(UPX),)
	$(_E) [UPX] $(@:%_r=%)
	$(_C)$(UPX) $(_Q) --best  $(@:%_r=%)
endif

release: FORCE $(GRFCODEC)_r $(GRFDIFF)_r $(GRFMERGE)_r $(GRFID)_r

# make grfmerge.exe (as grfmrgc.bin) optimized for size instead of speed
objs/grfmrgc.bin: objs/grfmerge.os $(GRFMERGESRC:%.cpp=objs/%.os)
	$(_C)rm -f $@
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(CXXFLAGS) -Os $^
ifneq ($(STRIP),)
	$(_E) [STRIP] $@
	$(_C)$(STRIP) $@
endif
ifneq ($(UPX),)
	$(_E) [UPX] $@
	$(_C)$(UPX) $(_Q) --best $@
endif

src/grfmrg.cpp: objs/grfmrgc.bin src/grfmrgc.pl
	$(_E) [PERL] $@
	$(_C)perl -w src/grfmrgc.pl > $@

src/ttdpal.h: $(PAL_FILES:%=src/%) src/pal2c.pl
	$(_E) [PERL] $@
	$(_C)perl src/pal2c.pl $(PAL_FILES:%=src/%) > $@

# Gnu compiler rules

objs/%.o : src/%.cpp Makefile
	$(_E) [CPP] $@
	$(_C)$(CXX) -c -o $@ $(CXXFLAGS) -MMD -MF $@.d -MT $@ $<

% : objs/%.o Makefile
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(CXXFLAGS) $^ $(LDOPT)
	$(_C)$(CP_TO_EXE)

# same as above but optimized for size not speed
objs/%.os : src/%.cpp Makefile
	$(_E) [CPP] $@
	$(_C)$(CXX) -c -o $@ $(CXXFLAGS) -Os -MMD -MF $@.d -MT $@ $<

% :: objs/%.os Makefile
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(CXXFLAGS) $^ $(LDOPT)

# On some installations a version.h exists in /usr/include. This one is then
# found by the dependency tracker and thus the dependencies do not contain
# a reference to version.h, so it isn't generated and compilation fails.
objs/grfcodec.o: src/version.h
objs/grfmerge.o: src/version.h
objs/grfmerge.os: src/version.h
objs/grfdiff.o: src/version.h
objs/grfidc.o: src/version.h

objs/%.o.d: src/%.cpp Makefile src/endian.h
	$(_E) [CPP DEP] $@
	$(_C)$(CXX) $(CXXFLAGS) -DMAKEDEP -MM -MG src/$*.cpp -MF $@

ifndef NO_MAKEFILE_DEP
-include $(GRFCODECSRC:%.cpp=objs/%.o.d)
-include $(GRFMERGESRC:%.cpp=objs/%.o.d)
-include $(GRFDIFFSRC:%.cpp=objs/%.o.d)
-include $(GRFIDSRC:%.cpp=objs/%.o.d)
-include $(GRFMERGESRC:%.cpp=objs/%.os.d)
-include $(GRFCODECSRC:%.cpp=objs/%.obj.d)
-include $(GRFMERGESRC:%.cpp=objs/%.obj.d)
-include $(GRFDIFFSRC:%.cpp=objs/%.obj.d)
-include $(GRFIDSRC:%.cpp=objs/%.obj.d)
endif

include Makefile.bundle
