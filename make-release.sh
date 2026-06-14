#!/bin/sh
# Script to make and commit a new release.

# Get variables
BASENAME=lamprop-c
# Get the latest tag from git.
RELEASE=`git tag -l |tail -n 1 | tr -d '\n'`
# Get the current date.
DATE=` date +%Y.%m.%d`
# Get the abbreviated commit hash of the latest commit.
COMMIT=`git rev-parse --short HEAD|tr -d '\n'`

# Remove duplicate tag, if any
git tag -d ${DATE}

# Updating version.h
echo "#define VERSION \""${DATE}"\"" >version.h
echo "#define LONG_VERSION \""${BASENAME}" version "${DATE}" (commit "${COMMIT}")\"" >>version.h

# Updating Makefile
sed -i '' -e "s/^RELDATE.*/RELDATE=${DATE}/" Makefile

# Committing modified files.
if git status -s -- version.h Makefile| grep 'M' >/dev/null; then
    git add version.h Makefile
    git commit -m "Update version."
fi

# Tag release.
echo "Tagging latest commit with date-derived version."
git tag ${DATE}
