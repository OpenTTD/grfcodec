/*
 * help_english.h
 * Program help in English language.
 *
 * Copyright 2005-2006 by Dale McCoy.
 * Copyright 2006 by Dan Masek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// START_HELP_TEXT(<language>)
// - <language> Language id as defined in language_list.h
//
// The actual help is just one long string.
START_HELP_TEXT(RL_ENGLISH)
"Usage: nforenum [options] [file1 [file2 [file3 [...]]]]\n"
"   Any number of files may be specified.\n"
"   If no files or options are specified, NFORenum will read a file from\n"
"   standard input and write it to standard output.\n"
"Options:\n"
"   --auto-correct -a\n"
"       Perform some rudimentary automatic correction of incorrect pseudo\n"
"       sprites. This may be specified twice to enable the corrections that\n"
"       are more likely to be incorrect.\n"
"       See the README for detailed information on the auto-correcter.\n"
"   --comments=<type> -c <type>\n"
"       <type> is one character, either /, ;, or #, and specifies the comment\n"
"       style that NFORenum will use. This will not change the header, because\n"
"       grfcodec requires that the header be commented in C++-style.\n"
"   --data[=<dir>] -D[<dir>]\n"
"       If <dir> is specified, look for the .nforenum directory in <dir>, and\n"
"       create it if not found.\n"
"       If <dir> is not specified, report the location of the .nforenum directory.\n"
"       In either case, eliminate the 5-second wait used to ensure that those\n"
"       not running NFORenum from a command line can see the directory-created\n"
"       message.\n"
"       Default: Look for  the .nforenum directory in the current directory, and\n"
"       then in the environment variable HOME, if defined. If not found attempt\n"
"       to create in HOME, then in .\n"
"   --force -f\n"
"       Forces processing of files that do not look like NFO files.\n"
"       The default is to treat such files as if they specified a too-high info\n"
"       version.\n"
"   --help -h\n"
"       Display this message and exit.\n"
"   --lock\n"
"       Locks the current comment command state. Commands will continue to be\n"
"       parsed as normal (so NOPRESERVE will be honored, @@DIFF will be\n"
"       removed, &c.) but their changes (such as turning on the diff-assister)\n"
"       will not be honored.\n"
"   --no-replace --keep-old -k\n"
"       Do not replace the old NFO file; write new file to file.new.nfo.\n"
"       Default: Use file[.nfo].new as temporary, rename it to file[.nfo]\n"
"       when done.\n"
"   --silent -s\n"
"       Suppress progress messages.\n"
"   --version -v\n"
"       Display version information and exit.\n"
"   --write-data\n"
"       Refresh all data files, unless they are newer.\n"
"\n"
"The following options cause NFORenum to behave as if all files started with\n"
"the associated command. The readme has full details on the comment commands.\n"
"Options associated with comment commands that require additional information\n"
"take that information as an argument. With the exception of -L/--let, the\n"
"options to the command line versions are case insensitive.\n"
"\"ON\" and \"OFF\" may be specified with \"+\" and \"-\", respectively.\n"
"   --beautify -b               @@BEAUTIFY\n"
"   --diff -d                   @@DIFF\n"
"   --let -L                    @@LET\n"
"   --lint -l                   @@LINT\n"
"   --preserve-messages -p      @@PRESERVEMESSAGES\n"
"   --real-sprites -r           @@REALSPRITES\n"
"   --use-old-nums -o           @@USEOLDSPRITENUMS\n"
"   --warning-disable -w        @@WARNING DISABLE\n"
"   --warning-enable -W         @@WARNING ENABLE\n"
"       -w and -W (and their long counterparts) also accept a comma-separated\n"
"       list of messages, all of which will be ENABLEd or DISABLEd.\n"
"\n"
"NFORenum is Copyright 2004-2009 by Dale McCoy\n"
"Portions Copyright 2006 Dan Masek\n"
"Portions Copyright 2010 Thijs Marinussen, Remko Bijker, Christoph Elsenhans\n"
"You may copy and redistribute it under the terms of the GNU General Public\n"
"License, as stated in the file 'COPYING'.\n"
"\n"
"If this message scrolls by too quickly, you may want to try\n"
"   nforenum -h | more\n"
END_HELP_TEXT()
