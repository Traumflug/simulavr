#!/bin/bash

THIS=`dirname $0`
pushd $THIS > /dev/null
THIS=`pwd`
popd > /dev/null
BUILDDIR=$THIS/../web_build/html

failure() {
  echo "error: $1. Abort!" && exit 1
}
message() {
  echo "**** $1 ..."
}

# check prerequsites: make, python, python-sphinx, sphinx-build, rsync
message "check prerequisites"
rsync --version > /dev/null || false
if [ $? = 1 ]; then
  failure "rsync not found"
fi

# check CVS web repo setting
if [ "$CVSREPO" = "" ]; then
  failure "environment variable CVSREPO not set"
fi
if [ ! -d $CVSREPO ]; then
  failure "$CVSREPO not found or not a directory"
fi
if [ ! -d $CVSREPO/CVS ]; then
  failure "$CVSREPO isn't a CVS repo"
fi
pushd $CVSREPO > /dev/null
CVSREPO=`pwd`
popd > /dev/null

# check build directory
if [ ! -d $BUILDDIR ]; then
  failure "web site not built, run 'make web-html' before"
fi
TMPDIR=`dirname $BUILDDIR`

# set language to built in default (english)
unset LANG

# print out, what's happen
CHANGED_FILES=`diff -rq -x CVS -x .buildinfo -x files $BUILDDIR $CVSREPO | grep differ | wc -l`
message "$CHANGED_FILES file(s) are changed"

NEW_FILES=`diff -rq -x CVS -x .buildinfo -x files $BUILDDIR $CVSREPO | grep "Only in $BUILDDIR" | wc -l`
message "$NEW_FILES file(s) are new"

OLD_FILES=`diff -rq -x CVS -x .buildinfo -x files $BUILDDIR $CVSREPO | grep "Only in $CVSREPO" | wc -l`
message "$OLD_FILES file(s) are deleted"

# sync files
message "sync"
rsync -avzPc --safe-links --delete-before --exclude '.buildinfo' --exclude 'CVS' --exclude 'files' $BUILDDIR/ $CVSREPO

# check CVS repo
message "cvs diff"
pushd $CVSREPO > /dev/null
cvs diff --brief > $TMPDIR/cvs-out.txt 2> $TMPDIR/cvs-err.txt

for I in `cat $TMPDIR/cvs-out.txt | grep "^\\?" | cut "-d " -f 2`; do
  message "please add in CVS repo: $I"
done
for I in `cat $TMPDIR/cvs-err.txt | grep "cannot find" | cut "-d " -f 5`; do
  message "please remove in CVS repo: $I"
done

popd > /dev/null

# EOF
