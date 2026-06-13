#!/bin/sh
# Script to make a new release.

# Get variables
BASENAME=lamprop-c
# Get the latest tag from git.
RELEASE=`git tag -l |tail -n 1 | tr -d '\n'`
# Get the date of the latest commit
CDATE=`git log -n 1 --pretty='%as'|tr '-' '.'`
# Get the abbreviated commit hash of the latest commit.
COMMIT=`git log -n 1 --pretty='%h'|tr -d '\n'`

# Tag release if nesessary.
if test ${CDATE} != ${RELEASE}; then
    echo "Tagging latest commit with date-derived version."
    git tag ${CDATE}
    RELEASE=${CDATE}
else
    echo "Latest commit is tagged: OK."
fi


echo "Updating version.h"
echo "#define VERSION \""${RELEASE}"\"" >version.h
echo "#define LONG_VERSION \""${BASENAME}" version "${RELEASE}" (commit "${COMMIT}")\"" >>version.h
#echo "#define RELEASE_NAME \""${BASENAME}"-w64-"${RELEASE}"\"" >>version.h

if git status -s -- version.h | grep 'M' >/dev/null; then
    echo "version.h modified; committing it."
    git add version.h
    git commit -m "Updated version.h"
fi
