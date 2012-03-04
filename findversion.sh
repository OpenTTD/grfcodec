#!/bin/sh

# $Id: findversion.sh 16462 2009-05-29 21:24:51Z rubidium $

# Arguments given? Show help text.
if [ "$#" != "0" ]; then
	cat <<EOF
Usage: ./findversion.sh
Finds the current revision and if the code is modified.

Output: <REV>\t<REV_NR>\t<MODIFIED>\t<CLEAN_REV>
REV
    a string describing what version of the code the current checkout is
    based on. The exact format of this string depends on the version
    control system in use, but it tries to identify the revision used as
    close as possible (using the svn revision number or hg/git hash).
    This also includes an indication of whether the checkout was
    modified and which branch was checked out. This value is not
    guaranteed to be sortable, but is mainly meant for identifying the
    revision and user display.

    If no revision identifier could be found, this is left empty.
REV_NR
    the revision number of the svn revision this checkout is based on.
    This can be used to determine which functionality is present in this
    checkout. For trunk svn checkouts and hg/git branches based upon it,
    this number should be accurate. For svn branch checkouts, this
    number is mostly meaningless, at least when comparing with the
    REV_NR from other branches or trunk.

    This number should be sortable. Within a given branch or trunk, a
    higher number means a newer version. However, when using git or hg,
    this number will not increase on new commits.

    If no revision number could be found, this is left empty.
MODIFIED
    Whether (the src directory of) this checkout is modified or not. A
    value of 0 means not modified, a value of 2 means it was modified.
    Modification is determined in relation to the commit identified by
    REV, so not in relation to the svn revision identified by REV_NR.

    A value of 1 means that the modified status is unknown, because this
    is not an svn/git/hg checkout for example.

CLEAN_REV
    the same as REV but without branch name

By setting the AWK environment variable, a caller can determine which
version of "awk" is used. If nothing is set, this script defaults to
"awk".
EOF
exit 1;
fi

# Allow awk to be provided by the caller.
if [ -z "$AWK" ]; then
	AWK=awk
fi

# Find out some dirs
cd `dirname "$0"`
ROOT_DIR=`pwd`
SRC_DIR=`pwd`

# Determine if we are using a modified version
# Assume the dir is not modified
MODIFIED="0"
if [ -d "$ROOT_DIR/.hg" ]; then
	# We are a hg checkout
	if [ -n "`hg status | grep -v '^?'`" ]; then
		MODIFIED="2"
	fi
	REV_NR=`LC_ALL=C hg id -n | cut -d+ -f1`
	if [ -n "`hg id -t | grep -v tip`" ]; then
		REV=`hg id -t | grep -v tip`
		BRANCH=""
	else
		REV=r$REV_NR
		BRANCH=`hg branch | sed 's@^default$@@'`
	fi
elif [ -f "$ROOT_DIR/.ottdrev" ]; then
	# We are an exported source bundle
	cat $ROOT_DIR/.ottdrev
	exit
else
	# We don't know
	MODIFIED="1"
	BRANCH=""
	REV=""
	REV_NR=""
fi

if [ "$MODIFIED" -eq "2" ]; then
	REV="${REV}M"
fi

CLEAN_REV=${REV}

if [ -n "$BRANCH" ]; then
	REV="${REV}-$BRANCH"
fi

echo "$REV	$REV_NR	$MODIFIED	$CLEAN_REV"
