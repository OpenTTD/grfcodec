
# Makefile for GRFCodec using gcc on Linux, or on Cygwin in Cygwin mode.

# Order of the palettes compiled in
# (note, this must match the text in grfcodec.cc and grftut.txt)
PALORDER = ttd_norm&ttw_norm&ttd_cand&ttw_cand&tt1_norm&tt1_mars

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
CFLAGS = -g -DWIN32 -O3 -I. $(CFLAGAPP) -Wall
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
NASMFORMAT = $(shell [ \( $(ISCYGWIN) -eq 1 \) ] && echo coff || echo elf )
GRFMERGE = $(shell [ \( $(ISCYGWIN) -eq 1 \) ] && echo grfmerge.exe || echo grfmerge)


# auxiliary sources to be linked with each of these programs
GRFCODECSRC=grfcomm.c pcx.c sprites.c pcxsprit.c info.c \
	error.c getopt.c path.c

GRFDIFFSRC=grfcomm.c error.c sprites.c getopt.c

GRFMERGESRC=getopt.c

# default targets
all: grfcodec grfdiff grfmerge
allb: grfcodec.exe grfdiff.exe grfmerge.exe

grfcodec:	grfcodec.o $(GRFCODECSRC:%.c=%.o) path.o 
grfdiff:	grfdiff.o $(GRFDIFFSRC:%.c=%.o) grfmrg.o path.o
grfmerge:	grfmerge.o $(GRFMERGESRC:%.c=%.o)

grfcodec.exe:	grfcodec.obj $(GRFCODECSRC:%.c=%.obj)
grfdiff.exe:	grfdiff.obj $(GRFDIFFSRC:%.c=%.obj) grfmrg.obj
grfmerge.exe:	grfmerge.obj $(GRFMERGESRC:%.c=%.obj)

clean:
	rm -rf *.o *.os *.obj *.OBJ *.exe *.EXE *.map *.MAP *.bin grfmrg.ah

# grfmrg.bin (the binary code included in grfdiff) can be either BCC or GCC code
grfmrg.bin:	grfmerge.exe
	upx --best $< -o $@

# remake grfmerge.exe optimized for size instead of speed
grfmrgc.bin:	grfmerge.os $(GRFMERGESRC:%.c=%.os)
	rm -f $@
	$(CC) -o grfmerge $(CFLAGS) -Os $^
	strip $(GRFMERGE)
	upx -qq --best $(GRFMERGE) -o $@
	rm -f $(GRFMERGE)

# making an assembly file which includes the above code; first for BCC
grfmrg.ah:	grfmrg.bin 
	perl mkmrg.pl "db " '0$${_}h' < $< > $@
	perl -le "print '_grfmrgsize dd ', (stat '$<')[7]" >> $@

grfmrg.obj:	grfmrg.asm grfmrg.ah
	tasm32 /dM32 /ml $< -c -o $@

# and then for GCC
grfmrg.o:	grfmrgc.asm grfmrgc.bin
	nasm -f $(NASMFORMAT) $< -o $@

ttdpal.h:	pals/$(subst &,.bcp pals/,$(PALORDER)).bcp
	perl pal2c.pl $^ > $@

# Gnu compiler rules

%.o : %.c
	$(CC) -c -o $@ $(CFLAGS) $<

%.o : %.cc
	$(CXX) -c -o $@ $(CXXFLAGS) $<

% : %.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDOPT)

%.S : %.c
	$(CC) -S -o $@ $(CFLAGS) $<

%.S : %.cc
	$(CC) -S -o $@ $(CXXFLAGS) $<


# same as above but optimized for size not speed
%.os : %.c
	$(CC) -c -o $@ $(CFLAGS) -Os $<

% :: %.os
	$(CC) -o $@ $(CFLAGS) $^ $(LDOPT)

Makefile.dep:
	[ -e ttdpal.h ] || touch ttdpal.h
	$(CC) $(CFLAGS) -MM *.c *.cc > $@
	perl -e "open DEP, '+<$@';@dep=<DEP>;s/.o:/.obj:/g for @dep;seek DEP,0,2;print DEP @dep"
	[ ! -s ttdpal.h ] && rm ttdpal.h

# Borland compiler rules

%.obj : %.c
	$(BCC) $(COPTS) -c $< -o$@

%.obj : %.cc
	$(BCC) -P $(CPPOPTS) -c $< -o$@

%.exe : %.obj
	$(BCC) $(LOPTS) $^ -e$@

include Makefile.dep
