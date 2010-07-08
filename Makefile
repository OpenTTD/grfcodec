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

# A command to return the current SVN revision of the source tree; it should
# it in the format [##:]##, where the second set of digits is the current
# revision (used for adding the revision to the version string)
SVNVERSION = svnversion -c .	# standard SVN client (e.g. cygwin)

# Gnu compiler settings
SHELL = /bin/sh
CC = g++
CXX = g++
STRIP = strip
UPX = upx
AWK = awk

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

TYPESIZE = GCC32

INSTALLPATH_CYGWIN=/usr/local/bin
INSTALLPATH_CYGWIN_MINGW=$(shell echo $$SYSTEMROOT | sed -e "s/\(.\):/\\/cygdrive\\/\\1/" -e s/\\\\/\\//g)
INSTALLPATH_MSYS_MINGW=$(SYSTEMROOT)
INSTALLPATH_LINUX=/usr/local/bin

-include ${MAKEFILELOCAL}


ifndef INSTALLPATH

# make an educated guess on the install path.
ifeq ($(EXE),.exe)
ifeq ($(ISCYGWIN),1)
ifeq ($(NOCYGWIN),1)
INSTALLPATH=$(INSTALLPATH_CYGWIN_MINGW)
else
INSTALLPATH=$(INSTALLPATH_CYGWIN)
endif
else
INSTALLPATH=$(INSTALLPATH_MSYS_MINGW)
endif
else 
INSTALLPATH=$(INSTALLPATH_LINUX)
endif
endif

# use 386 instructions but optimize for pentium II/III
CFLAGS = -g -D$(TYPESIZE) -O3 -I. -O1 -idirafter$(BOOST_INCLUDE) -Wall -Wno-uninitialized $(CFLAGAPP)

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

CXXFLAGS = $(CFLAGS)


# Somewhat automatic detection of the correct boost include folder
ifndef BOOST_INCLUDE
BOOST_INCLUDE=$(shell \
find /usr/include /usr/local/include -maxdepth 1 -name 'boost-*' 2> /dev/null | sort -t - -k 2 | tail -n 1 )
ifeq ($(BOOST_INCLUDE),)
BOOST_INCLUDE=$(shell \
( [ -d /usr/include/boost/date_time ] && echo /usr/include ) || \
( [ -d /usr/local/include/boost/date_time ] && echo /usr/local/include ) )
endif
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

GRFMERGESRC=getopt.c grfmerge.c

ifndef NOREV
NOREV = 0
endif

ifndef NO_BOOST
NO_BOOST = 0
endif

ifeq ($(BOOST_INCLUDE),)
BOOST_CMD=-DNO_BOOST
BOOST_WARN = @echo "Warning: boost::date_time not found.  \\w<date> and \\d<date> will not be" ; echo "  supported.  If you have recently installed boost, try sudo updatedb."
else
BOOST_CMD=-I$(BOOST_INCLUDE)
endif

ifneq ($(NO_BOOST),0)
BOOST_CMD=-DNO_BOOST
BOOST_WARN = @echo "Warning: \\w<date> and \\d<date> support disabled by NO_BOOST setting."
endif

PAL_FILES = pals/$(subst &,.bcp pals/,$(PALORDER)).bcp

# deafult targets
all: $(GRFCODEC) $(GRFDIFF) $(GRFMERGE)
remake:
	$(_E) [CLEAN]
	$(_C)$(MAKE) ${_S} clean
	$(_E) [REBUILD]
	$(_C)$(MAKE) ${_S} all

${MAKEFILELOCAL}:
	@/bin/sh -c "export PATH=\"/bin\" && \
	echo ${MAKEFILELOCAL} did not exist, using defaults. Please edit it if compilation fails. && \
	cp ${MAKEFILELOCAL}.sample $@"

$(GRFCODEC): $(GRFCODECSRC:%.c=%.o)
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(CFLAGS) $^ $(LDOPT)

$(GRFDIFF):  $(GRFDIFFSRC:%.c=%.o) grfmrg.o
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(CFLAGS) $^ $(LDOPT)

$(GRFMERGE): $(GRFMERGESRC:%.c=%.o)
	$(_E) [LD] $@
	$(_C)$(CXX) -o $@ $(CFLAGS) $^ $(LDOPT)


clean:
	rm -rf *.o *.os *.bin $(GRFCODEC) $(GRFDIFF) $(GRFMERGE) bundle bundles

mrproper: clean
	rm -f *.d .rev version.h grfmrg.c version.h.tmp
	@touch -ct 9901010000 ttdpal.h	# don't delete it, so we don't confuse svn, but force it to be remade

FORCE:
	@$(BOOST_WARN)

.rev: #FORCE
	$(_C) [ -e $@ ] || echo SVNREV=0 > $@
	$(_C) REV=`${SVNVERSION}` perl rev.pl $@ < $@

-include .rev
include version.def

version.h: FORCE
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

release: FORCE $(GRFCODEC)_r $(GRFDIFF)_r $(GRFMERGE)_r

# make grfmerge.exe (as grfmrgc.bin) optimized for size instead of speed
grfmrgc.bin:	grfmerge.os $(GRFMERGESRC:%.c=%.os)
	$(_C)rm -f $@
	$(_E) [LD] $@
	$(_C)$(CC) -o $@ $(CFLAGS) -Os $^
	$(_E) [STRIP] $@
	$(_C)$(STRIP) $@
ifneq ($(UPX),)
	$(_E) [UPX] $@
	$(_C)$(UPX) $(_Q) --best $@
endif

grfmrg.c:	grfmrgc.bin grfmrgc.pl
	$(_E) [PERL] $@
	$(_C)perl -w grfmrgc.pl > $@

ttdpal.h:	$(PAL_FILES) pal2c.pl
	$(_E) [PERL] $@
	$(_C)perl pal2c.pl $(PAL_FILES) > $@

# Gnu compiler rules

%.o : %.c
	$(_E) [CC] $@
	$(_C)$(CC) -c -o $@ -MMD -MF $@.d $(CFLAGS) $<

%.o : %.cc
	$(_E) [CPP] $@
	$(_C)$(CXX) -c -o $@ -MMD -MF $@.d $(CXXFLAGS) $(BOOST_CMD) $<

% : %.o
	$(_E) [LD] $@
	$(_C)$(CC) -o $@ $(CFLAGS) $^ $(LDOPT)
	$(_C)$(CP_TO_EXE)

%.S : %.c
	$(_E) [CC] $@
	$(_C)$(CC) -S -o $@ $(CFLAGS) $<

%.S : %.cc
	$(_E) [CPP] $@
	$(_C)$(CC) -S -o $@ $(CXXFLAGS) $<


# same as above but optimized for size not speed
%.os : %.c
	$(_E) [CC] $@
	$(_C)$(CC) -c -o $@ -MMD -MF $@.d $(CFLAGS) -Os $<

% :: %.os
	$(_E) [LD] $@
	$(_C)$(CC) -o $@ $(CFLAGS) $^ $(LDOPT)

# Borland compiler rules

%.obj : %.c
	$(_E) [CC] $@
	$(_C)$(BCC) $(COPTS) -c $< -o$@

%.obj : %.cc
	$(_E) [CPP] $@
	$(_C)$(BCC) -P $(CPPOPTS) -c $< -o$@

%.exe : %.obj
	$(_E) [LD] $@
	$(_C)$(BCC) $(LOPTS) $^ -e$@

%.o.d:
	$(_E) [CPP DEP] $@
	$(_C)$(CC) $(CFLAGS) -DMAKEDEP -MM -MG $*.c* -MF $@

%.os.d:
	$(_E) [CPP DEP] $@
	$(_C)$(CC) $(CFLAGS) -DMAKEDEP -MM -MG -MT ${subst .d,,$@} -MF $@ $*.c*

%.obj.d: $(wildcard %.c*)
	$(_E) [CPP DEP] $@
	$(_C)$(CC) $(CFLAGS) -DMAKEDEP -MM -MG -MT ${subst .d,,$@} -MF $@ $*.c*

ifndef NO_MAKEFILE_DEP
-include $(GRFCODECSRC:%.c=%.o.d)
-include $(GRFMERGESRC:%.c=%.o.d)
-include $(GRFDIFFSRC:%.c=%.o.d)
-include $(GRFMERGESRC:%.c=%.os.d)
-include $(GRFCODECSRC:%.c=%.obj.d)
-include $(GRFMERGESRC:%.c=%.obj.d)
-include $(GRFDIFFSRC:%.c=%.obj.d)
endif

include Makefile.bundle

install: $(GRFCODEC) $(GRFMERGE) $(GRFDIFF)
	$(_E) [INSTALL]
	$(_C)cp $(GRFCODEC) $(INSTALLPATH)
	$(_C)cp $(GRFMERGE) $(INSTALLPATH)
	$(_C)cp $(GRFDIFF) $(INSTALLPATH)
