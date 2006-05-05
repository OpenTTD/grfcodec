NFORenum v3.2.0
NFO file re-numberer and linter

Known to compile with g++ 3.4.x and VS.NET, assuming the Boost headers are
installed. Get Boost from www.boost.org.
The required boost header is currently included, but this may change.

To install, copy renum.exe into any directory, preferably one in your path.
NFORenum will (if necessary) create a .renum directory in the current
directory. All its data files will be placed in that directory, and updated
as needed.
It is not necessary to remove the .renum directory before upgrading.
To uninstall, delete renum.exe and any .renum directories.

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
For a detailed explanation of the sanity checker's messages, see SANITY or
SANITY.txt (depending on package downloaded).

Sprites that for some reason cannot be processed will be commented out, and
processing will continue.

NFORenum will look for the NFO in the following locations, in this order:
filename
filename.NFO
sprites/filename
sprites/filename.NFO
(The *nix version uses lower-case extensions instead)

The old verson, that uses standard input and standard output, can be invoked
by starting renum with no command-line arguements.


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


Known Issues:
- An Action 6 modifying any checked sprite can cause erroneous sanity messages.
  Status: Medium priority, probably hard to do correctly.
  Workaround: use the @@USEID2 and @@USESET commands to supress incorrect
    messages.
- @@VERSIONCHECK does not put its second action 9 in the correct place.
  Manually move that action 9 after the action 8.


--------------------------------------------------------------------------

NFORenum can be controlled by adding the following commands to the NFO file.
All of them must be commented out and prefixed with "@@". Commands take
effect immediately, but do not affect previous lines. To place a command in
the middle of a pseudo-sprite is an error, and will comment the trailing
portion of the sprite.
Some commands may be specified on the command line; the output of renum -?
has details on which command-line option(s) match up which commands.

On the command line, the single characters + and - may be used in place of ON
and OFF, respectively. This allows, for example, -l- to turn the linter off,
or -b+ to turn the beautifier on.

The equals sign (=) may be used in place of whitespace anywhere in a comment
command; this is to make it easier to specify their command-line equivalent.
(eg -b setcookie=686031766)

As of 2.8.6, comment commands are no longer case sensitive, but the all-caps
version remains canon.

Unless marked othewise, NOPRESERVE may be appended any comment command. If
appended, the command will be removed from the NFO, otherwise it will be
written back.


@@PRESERVEMESSAGES
@@REMOVEMESSAGES

These will preserve/remove messages that exist in the NFO. The default is to
remove messages.


@@BEAUTIFY [option [value]]

This collection of commands controls the beautifier. The settings, their
ranges, and their default values are listed below.

ON|OFF
Turns the beautifier on or off. OFF is default, but ON is implied when
setting any other option (including GETCOOKIE)

CONVERTONLY ON|OFF (default OFF)
When on, NFORenum will do its best to preserve the format of the input
sprites while changing them according to HEXGRFID, QUOTEUTF8, and
QUOTEHIGHASCII. When CONVERTONLY is ON, LEADINGSPACE, LINEBREAKS, and MAXLEN
are ignored.

HEXGRFID ON|OFF (default OFF)
When on, all GRFIDs will be output entirely in hex. When off, the first two
bytes will be quoted if both are printable, and the second two will be in
hex. The character FF is considered non-printable in GRFIDs, so FF FF FF FF
will be output in hex, not as "ÿÿ" FF FF.

LEADINGSPACE <num-beaut1>[,<num-beaut2>[,<num-linelen>]]
  (range 1..32 for each, default 5,8,11)
This specifies the number of spaces to be added after line breaks.
<num-beaut1> is for what is normally the shorter space (after 00s in
action 4s, for example), <num-beaut2> for what is normally the longer space
(after 0Ds in strings, for example), and <num-linelen> for line breaks added
because the line got too long.
Specifying 0 for any <num-*> is equivalent to specifying its old setting.
Failing to specify a <num-*> is equivalent to specifying a number three larger
than the previous one, or three larger than the old setting of the previous
one if the previous number is 0. (So LEADINGSPACE 1 and LEADINGSPACE 1,4,7 are
equivalent.)
Specifying a value greater than 32 for any number is an error.

LINEBREAKS <num> (0..3, default 2)
This sets the frequency of beautifier-controlled linebreaks. 0 is the lowest,
meaning only break lines when MAXLEN says to do so. 3 is the highest,
breaking lines well-nigh everywhere.

MAXLEN <num> (0..255, default 78)
The maximum line length, in characters. The sprite leader (0 * 0 ...) is
taken to be sixteen characters long. Specifying 0 means that sprites will not
be broken except by the beautifier.

QUOTEHIGHASCII ON|OFF (default ON)
When off, high-ASCII bytes (80..FF) will always be outputted in hex. When on,
high-ASCII bytes in strings will be quoted. Regardless of the setting of this
option, the bytes 00..1F, 7B..A0, AA, AC, AD, AF, and B4..B8 will never be
quoted, unless in a UTF-8 sequence. High-ASCII bytes that are part of a UTF-8
sequence are controlled by the next option. 

QUOTEUTF8 ON|OFF (default ON)
When off, UTF-8 sequences in strings will always be outputted in hex. When
on, UTF-8 sequences will be quoted. High-ASCII bytes in strings that do not
begin with an uppercase thorn (U+00DE, Þ) are controlled by the previous
option.

GETCOOKIE
This is replaced with @@BEAUTIFY SETCOOKIE <cookie>, where <cookie> is the
current magic cookie. Specifying -b getcookie or its equivalents on the
command-line causes the SETCOOKIE command to be sent to standard output.

SETCOOKIE <cookie>
This restores all the above options to the state they were in when GETCOOKIE
returned <cookie>. <cookie> has no other useful purpose.


@@CLEARACTION2

Causes the action 1/2/3 parser to behave as if an end-of-file has been hit.
Any unused sprite sets from the preceeding action 1 are warned about, and
the end-of-file unused-cargoID checks are run, after which all cargo IDs
are marked as undefined.


@@CLEARACTIONF

As CLEARACTION2, but for the action F parser and town name IDs instead.


@@DEFINEID2 <feature> <id>

Marks <id> as defined for <feature>. Tests for unused previous definition
are not run.


@@DIFF

This will cause the NFO to be formatted in an attempt to help produce useful
diffs. It implies @@REMOVEMESSAGES and @@SANITY OFF. Every sprite in the NFO
will be numbered -1 and given a length of 0, sprite 0 will report 0 sprites,
and no further messages will be written to the NFO.

This command will never be written back to the NFO.


@@LET <var> [=] <calculation>

This is useful for variable assignment to be used in real-sprite calculations.
The calculations in real sprites have the same format as the calculations in
@@LET.
<var> is any valid C identifier (first character must be A..Z, a..z or _, all
others must be A..Z, a..z, 0..9 or _)
<calculation> is one of:
1) an RPN expression, enclosed in parentheses. (See README.RPN)
2) a signed integer value.


@@LINT {OFF|NONE|FATAL|ERROR|WARNING(1-4)}

This controls the linter. OFF will turn it off for the remainder of the file.
NONE supresses all messages, but checking continues. The remaining options
suppress all messages below that urgency. Failures occur when the linter
cannot or will not continue with that sprite, errors occur when it can
continue, but TTDPatch will not accept the sprite, and warnings when TTDPatch
will accept the sprite, but it is probably wrong anyway. The default level is
WARNING3.
This does not effect parse error messages, which alert you to things GRFCodec
will not accept.


@@LOCATEID2 <feature> <id>

Issues a message of the form //!!LOCATEID2 <feature> <id>: <spritenum>, where
<spritenum> is the most recent definition of <id>.
If the most recent definition of <id> was made by a @@DEFINEID2, <spritenum>
is undefined.


@@REALSPRITES {RPNON|RPNOFF|COMMENTON|COMMENTOFF}

This controls the real-sprite checker.
RPNON/RPNOFF turns the RPN expression evaluator on or off. RPNOFF also sets
COMMENTOFF. This may be overridden by following @@REALSPRITES RPNOFF with
@@REALSPRITES COMMENTON. If the metadata apparently contains an RPN
expression and both RPNOFF and COMMENTOFF are set, no unparsable-metadata
warning message will be issued.
COMMENTON/COMMENTOFF turns the comment-on-unparsable-metadata feature on or
off. Realsprites that are apparently missing filenames will always get
commented; parsing them as real sprites risks losing data if they are
instead continuations of pseudosprites with an invalid character.


@@TESTID2 <feature> <id>

Test to see if <id> is defined for <feature> but does not mark it as used.


@@USEID2 $feature $id

This is a work-around for NFORenum's current inability to correctly parse the
Action 6s; it will supress "Unused ID" messages for that feature/id pair,
until the next definition. $feature and $id both MUST be two characters long,
and written in hex. NFORenum's behaviour is undefined otherwise.


@@USESET $set

This is a work-around for NFORenum's current inability to correctly parse the
Action 6s; it will supress "Unused sprite set" messages for that set, until
the next Action 1. $set MUST be two characters long, and written in hex.
NFORenum's behaviour is undefined otherwise.


@@VERSIONCHECK <ver> <name>

This will insert a TTDPatch version check.
<ver> can be in one of three different formats:
 1) The dotted-decimal version number, eg 2.0.10.640 for 2.0.1 alpha 64.
 2) The 8-character version code, best acquired by copying from switches.xml.
 3) The coloquial version number, either dotted or undotted (201a<num>, 
    2.0.1a<num>, 25b<num>, or 2.5b<num>), eg 201a66 for 2.0.1 alpha 66.
Note that #3 cannot be used to specify a version that is not in the 2.0.1
alpha or 2.5 beta series, nor a non-official version.
<name> is the human-readable name of that version, either as a series of hex
characters or as a quoted string. A 00 will be appended by NFORenum.
For example, to check for at least alpha 63, all of these are valid:
//@@VERSIONCHECK 020A0276 "2.0.1 alpha 63"
//@@VERSIONCHECK 2.0.10.630 32 2e 30 2e 31 20 61 6c 70 68 61 20 36 33
//@@VERSIONCHECK 201a63 "2.0.1 alpha 63"
//@@VERSIONCHECK 2.0.1a63 "2.0.1 alpha 63"

This command will never be written back to the NFO.


@@WARNING {ENABLE|DEFAULT|DISABLE} <num>

This provides finer control over which warnings and errors are issued than
@@SANITY does. ENABLE causes this warning/error to be issued regardless of
the warning level. DISABLE suppresses this warning/error regardless of the
warning level.  DEFAULT reverts to the most recent @@SANITY setting. <num>
is read from the message: "Warning (<num>):" or "Error (<num>):" It is not
possible to DISABLE fatal messages. They can be ENABLEd. The corresponding
message for a given value of <num> may change from version to verson.
