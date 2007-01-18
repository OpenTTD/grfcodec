NFORenum v3.4.4
NFO file re-numberer and linter


Compiling
=========

Known to compile with g++ 3.4.4, g++ 4.0.2, and VS .NET, assuming Boost is
installed. Get Boost from www.boost.org.
The required boost header is included. If you do not have the rest of boost,
make should detect this and remove the boost dependency.
For more information consult 0compile.txt in the source code.


Installation
============

To install, copy renum.exe into any directory, preferably one in your path.
NFORenum will (if necessary) create a .renum directory. All its data files
will be placed in that directory, and updated as needed.
It is not necessary to remove the .renum directory before upgrading.
To uninstall, delete renum.exe and any .renum directories.


Usage
=====

To renumber NFO files (for example MyGRF1.NFO and MyGRF2.NFO), use

 > renum [options] MyGRF1 MyGRF2

(Run renum -? or renum --help for a list of command line options.)

This will correct sprite numbers, sprite sizes, and sprite 0, and do a
partial lint of the NFO. If a NFO header is not detected, one will be added.
If the file does not start with a digit, an asterisk, whitespace, or one of
the three comment characters (/, ;, #), and -f/--force was not specified,
the file will not be processed.

Sprite numbers are required on real sprites if the filename starts with a
digit, but are optional in all other cases.

Messages will be output to the console and to the NFO file.
For a detailed explanation of the sanity checker's messages,
see SANITY.en.txt.

Sprites that for some reason cannot be processed will be commented out, and
processing will continue.

NFORenum will look for the NFO in the following locations, in this order:
  <filename>
  <filename>.nfo
  sprites/<filename>
  sprites/<filename>.nfo

If no command-line arguments are specified, NFORenum will read from standard
input and write to standard output.

The default search paths for the .renum directory are as follows:
  .
  $HOME
  $HOMEDRIVE$HOMEPATH
If the .renum directory is not found in any of these locations, NFORenum will
attempt to create one in each location, starting from the bottom of the list
and working up. This can be overridden with the --data option.

The NFORenum will exit with one of the following error codes: (The highest
applicable code will be returned)
8: Fatal program error. (A can't-happen happened.)
7: Data file error. (Failure to create the data dir or open a file.)
6: NFO file error. (Failure to open a file or rename the temp file.)
5: NFO parse error. (Sprite with invalid chars/length, or version too high.)
4: Lint error. (Linter issued an error or fatal error.)
3: Lint warning. (Linter issued a warning.)
0: Successful run, NFO file is clean.

Codes 7 and 8 are supposed to be can't-happens; if they do, NFORenum will
send a message to the console and immediately die.
Codes 1 and 2 are reserved pending discussion; see http://tinyurl.com/acudt.


Known Issues
============

- An Action 6 modifying any checked sprite can cause erroneous sanity messages.
  Status: Low priority, hard to do correctly.
  Workaround: use the @@USEID2 and @@USESET commands to supress incorrect
    messages.
- @@VERSIONCHECK does not put its second action 9 in the correct place.
  Workaround: Manually move that action 9 after the action 8.

