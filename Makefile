# =========================================================
# Makefile for the GRF development tools
#
#       Don't put any local configuration in here
#       Change Makefile.local instead, it'll be
#       preserved when updating the sources
# =========================================================

PACKAGE_NAME = grfcodec

-include Makefile.local

# Gnu compiler settings
SHELL        ?= /bin/sh
CXX          ?= g++
STRIP        ?=
UPX          ?=
AWK          ?= awk
SRCZIP_FLAGS ?= -9
SRCZIP       ?= gzip
LIBPNG_CONFIG?= libpng-config

# OS detection: Cygwin vs Linux
ISCYGWIN     ?= $(shell [ ! -d /cygdrive/ ]; echo $$?)
MACHINE      ?= $(shell $(CXX) -dumpmachine || echo '??' )

# OS dependent variables
EXE          ?= $(shell ( [ \( $(ISCYGWIN) -eq 1 \) -o \( "$(MACHINE)" = "mingw32" \) ] ) && echo .exe)
GRFCODEC     ?= grfcodec$(EXE)
GRFID        ?= grfid$(EXE)
GRFSTRIP     ?= grfstrip$(EXE)
NFORENUM     ?= nforenum$(EXE)
ENDIAN_CHECK ?= endian_check$(EXE)

TYPESIZE     ?= GCC32

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

ifndef FLAGS
ifeq ($(ISCYGWIN),1)
FLAGS = -I $(BOOST_INCLUDE) -O2
else
FLAGS = -idirafter$(BOOST_INCLUDE) -O2
endif
FLAGS += -D$(TYPESIZE) -D_FORTIFY_SOURCE=2
FLAGS += -Wall -Wextra -Wno-format-nonliteral

ifeq ($(DEBUG),1)
FLAGS += -DDEBUG
endif

ifeq ($(MACHINE),mingw32)
FLAGS += -DMINGW
endif

# Sadly enough fmemopen is
ifneq ($(shell grep fmemopen /usr/include/stdio.h 2> /dev/null | wc -l | sed "s/ *//"),0)
ifndef NO_FMEMOPEN
FLAGS += -DWITH_FMEMOPEN
endif
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

# Complication of png support in grfcodec
ifneq ($(shell $(LIBPNG_CONFIG) --version 2>/dev/null),)
WITH_PNG = 0
FLAGS += -DWITH_PNG $(shell $(LIBPNG_CONFIG) --cflags)
endif
endif

MY_CXXFLAGS ?= $(FLAGS) $(CXXFLAGS)

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
GRFCODECSRC=grfcomm.cpp pcxfile.cpp sprites.cpp pcxsprit.cpp pngsprit.cpp \
	info.cpp globals.cpp mapescapes.cpp error.cpp path.cpp readinfo.cpp \
	file.cpp grfcodec.cpp

GRFIDSRC=grfid.cpp md5.cpp

GRFSTRIPSRC=grfstrip.cpp

NFORENUMSRC=IDs.cpp act0.cpp act123.cpp act123_classes.cpp act5.cpp act6.cpp \
  act79D.cpp actB.cpp actF.cpp act14.cpp command.cpp data.cpp globals.cpp \
  inject.cpp messages.cpp pseudo.cpp rangedint.cpp nforenum.cpp sanity.cpp \
  strings.cpp utf8.cpp help.cpp message_mgr.cpp language_mgr.cpp \
  mapescapes.cpp pseudo_seq.cpp

PALORDER = ttd_norm&ttw_norm&ttd_cand&ttw_cand&tt1_norm&tt1_mars&ttw_pb_pal1&ttw_pb_pal2
PAL_FILES = pals/$(subst &,.bcp pals/,$(PALORDER)).bcp

# default targets
all: $(GRFCODEC) $(GRFID) $(GRFSTRIP) $(NFORENUM)

remake:
	$(_E) [CLEAN]
	$(_C)$(MAKE) ${_S} clean
	$(_E) [REBUILD]
	$(_C)$(MAKE) src/version.h src/endian.h
	$(_C)$(MAKE) ${_S} all

$(GRFCODEC): $(GRFCODECSRC:%.cpp=objs/%.o)
	$(_E) [LD] $@
ifdef WITH_PNG
	$(_C)$(CXX) -o $@ $(MY_CXXFLAGS) $^ $(LDOPT) $(shell $(LIBPNG_CONFIG) --ldflags)
else
	$(_C)$(CXX) -o $@ $(MY_CXXFLAGS) $^ $(LDOPT)
endif

$(GRFID): $(GRFIDSRC:%.cpp=objs/%.o)
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(MY_CXXFLAGS) $^ $(LDOPT)

$(GRFSTRIP): $(GRFSTRIPSRC:%.cpp=objs/%.o)
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(MY_CXXFLAGS) $^ $(LDOPT)

$(NFORENUM): $(NFORENUMSRC:%.cpp=objs/%.o)
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(MY_CXXFLAGS) $^ $(LDOPT)


clean:
	$(_C)rm -rf objs $(GRFCODEC) $(GRFID) $(GRFSTRIP) $(NFORENUM) bundle bundles grfcodec-* src/endian.h

mrproper: clean
	$(_C)rm -f src/version.h
	$(_C)touch -ct 9901010000 ttdpal.h # don't delete it, so we don't confuse version control, but force it to be remade

distclean: mrproper
	$(_C)rm -rf Makefile.local

FORCE:
	@$(BOOST_ERROR)

include version.def

src/version.h: FORCE
	@echo // Autogenerated by make.  Do not edit.  Edit version.def or the Makefile instead. > $@.tmp
	@echo "#define VERSION \"$(VERSIONSTR)\"" >> $@.tmp
	@echo "#define YEARS \"2004-$(YEAR)\"" >> $@.tmp
	@(diff $@.tmp $@ > /dev/null 2>&1 && rm -f $@.tmp) || (rm -f $@ ; mv $@.tmp $@)

objs/$(ENDIAN_CHECK): src/endian_check.cpp
	$(_C) mkdir -p objs
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(MY_CXXFLAGS) src/endian_check.cpp $(LDOPT)

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

.NOTPARALLEL release: FORCE $(GRFCODEC)_r $(GRFID)_r $(GRFSTRIP)_r $(NFORENUM)_r

src/ttdpal.h: $(PAL_FILES:%=src/%) src/pal2c.pl
	$(_E) [PERL] $@
	$(_C)perl src/pal2c.pl $(PAL_FILES:%=src/%) > $@


# Gnu compiler rules

objs/%.o : src/%.cpp Makefile
	$(_E) [CPP] $@
	$(_C)$(CXX) -c -o $@ $(MY_CXXFLAGS) -MMD -MF $@.d -MT $@ $<

% : objs/%.o Makefile
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(MY_CXXFLAGS) $^ $(LDOPT)
	$(_C)$(CP_TO_EXE)

# same as above but optimized for size not speed
objs/%.os : src/%.cpp Makefile
	$(_E) [CPP] $@
	$(_C)$(CXX) -c -o $@ $(MY_CXXFLAGS) -Os -MMD -MF $@.d -MT $@ $<

% :: objs/%.os Makefile
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(MY_CXXFLAGS) $^ $(LDOPT)

# On some installations a version.h exists in /usr/include. This one is then
# found by the dependency tracker and thus the dependencies do not contain
# a reference to version.h, so it isn't generated and compilation fails.
objs/grfcodec.o: src/version.h
objs/grfid.o: src/version.h
objs/grfstrip.o: src/version.h
objs/message_mgr.o: src/version.h
objs/messages.o: src/version.h

objs/%.o.d: src/%.cpp Makefile src/endian.h
	$(_E) [CPP DEP] $@
	$(_C)$(CXX) $(MY_CXXFLAGS) -DMAKEDEP -MM -MG src/$*.cpp -MF $@

ifndef NO_MAKEFILE_DEP
-include $(GRFCODECSRC:%.cpp=objs/%.o.d)
-include $(GRFIDSRC:%.cpp=objs/%.o.d)
-include $(GRFSTRIPSRC:%.cpp=objs/%.o.d)
-include $(NFORENUMSRC:%.cpp=objs/%.o.d)
endif

include Makefile.bundle
