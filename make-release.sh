#!/bin/sh
# Script to make a new release.

# Get variables
BASENAME=lamprop-c
# Get the latest tag from git.
RELEASE=`git tag -l |tail -n 1 | tr -d '\n'`
# Get the current date.
DATE=` date +%Y.%m.%d`
# Get the abbreviated commit hash of the latest commit.
COMMIT=`git log -n 1 --pretty='%h'|tr -d '\n'`

# Remove duplicate tag, if any
git tag -d ${DATE}

echo "Updating version.h"
echo "#define VERSION \""${DATE}"\"" >version.h
echo "#define LONG_VERSION \""${BASENAME}" version "${DATE}" (commit "${COMMIT}")\"" >>version.h

echo "Updating Makefile"
sed -i '' -e "s/^RELDATE.*/RELDATE=${DATE}/" Makefile

if git status -s -- version.h | grep 'M' >/dev/null; then
    echo "version.h modified; committing it."
    git add version.h Makefile
    git commit -m "Updated version.h"
fi

# Tag release.
echo "Tagging latest commit with date-derived version."
git tag ${DATE}
