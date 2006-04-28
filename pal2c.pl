#!/usr/bin/perl -w

use strict;
use File::Basename;

my @files = map glob, @ARGV;

print "  U8 defaultpalettes[".@files."][256*3] = {\n";

my $palind = 0;

for (@files) {
	open FILE, "<$_" or die "Can't open $_: $!";
	binmode FILE;
	local $/ = \3;		# read three bytes at a time
	$.=0;

	$_ = basename $_;
	s/\..*//;
	print "#define PAL_$_ $palind\n";
	$palind++;

	print "    {\n";

	while (<FILE>) {
		s/(.)/sprintf "%3d,",ord $1/ge;
		print "\t" if $. % 4 == 1 ;
		print "$_   ";
		printf "\t// %d-%d\n", $.-4, $.-1 if $. % 4 == 0;
	}
	close FILE;
	print "    },\n";
}
printf("  };\n");
