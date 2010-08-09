GRFCodec README
Last updated:    2010-08-09
Release version: 1.0.0
------------------------------------------------------------------------


Table of Contents:
------------------
1) About
2) Contacting
3) Installing
4) Running
5) Compiling


1) About:
-- ------
GRFCodec decodes and encodes GRF files for Transport Tycoon (Deluxe and
Original), TTDPatch and OpenTTD. These GRFs are a collection of sprites,
although for TTDPatch and OpenTTD these can be so-called pseudosprites
which allow changes of behaviour of the game.

GRFCodec comes with a number of small extra tools for making and applying
"diffs" for GRF files or for extracting the so-called GRF ID from a
GRF.

GRFCodec, GRFDiff, GRFMerge and GRFID are licensed under the GNU General
Public License version 2.0. For more information, see the file 'COPYING'.


2) Contact:
-- --------
Contact can be made via the issue tracker / source repository at
http://dev.openttdcoop.org/projects/grfcodec or via IRC on the
#openttdcoop.devzone channel on OFTC.


3) Installation:
-- -------------
Installing GRFCodec is fairly straightforward. Just copy the executable into
any directory. It is advised to put the executable in one of the directories
in your path so it can be easily found. For example when compiling OpenGFX.
To uninstall simply remove the executable.


4) Compiling:
-- ----------
GCC/ICC:
  Just use "make", or on non-GNU systems "gmake".
