# grfcodec
GRF development tools README

Last updated: 2024-05-03

Release version: 6.1.0

## Table of Contents

- 1.0) About
- 2.0) Contact
- 3.0) Installation
- 4.0) Running
- 5.0) Compiling

## 1.0) About

The GRF development tools are a set of tools for developing (New)GRFs.
It includes a number of smaller programs, each with a specific task.

GRFCodec decodes and encodes GRF files for Transport Tycoon (Deluxe and
Original), TTDPatch and OpenTTD. These GRFs are a collection of sprites,
although for TTDPatch and OpenTTD these can be so-called pseudosprites
which allow changes of behaviour of the game.

GRFID is a small tool for extracting the so-called GRF ID from a GRF.

NFORenum is a tool checking NFO code for errors, and for beautifying that
code -- inasmuch as NFO can be beautified :)

GRFCodec, GRFID and GRFStrip are licensed under the
GNU General Public License version 2.0. For more information, see the
file 'COPYING'. GRFID contains the MD5 implementation written by
L. Peter Deutsch and this MD5 implementation is licensed under the
zlib license.

NFORenum is licensed under the GNU General Public License version 2, or at
your option, any later version. For more information, see 'COPYING'
(GPL version 2), or later versions at <http://www.gnu.org/licenses/>.

## 2.0) Contact

Contact can be made via the issue tracker / source repository at
https://github.com/OpenTTD/grfcodec or via IRC on the
#openttd channel on OFTC.

## 3.0) Installation

Installing the GRF development tools is fairly straightforward. Just copy
the executable into any directory. It is advised to put the executable in
one of the directories in your path so it can be easily found. For example
when compiling OpenGFX.

NFORenum will (if necessary) create a .nforenum directory. All its data files
will be placed in that directory, and updated as needed.
It is not necessary to remove the .nforenum directory before upgrading.

To uninstall, delete the executables and any .nforenum directories.

## 4.0) Usage

Information about the usage of the different tools can be found in their
respective man pages as well as grfcodec.txt, grftut.txt and nforenum.txt.

## 5.0) Compiling

GCC/ICC:
  Just use "make", or on non-GNU systems "gmake".
