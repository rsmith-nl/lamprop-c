#!/bin/sh
# Script to update *only* the commit hash.
# To make a release, use make-release.sh!

# Get variables
BASENAME=lamprop-c
# Get the latest tag from git.
RELEASE=`git tag -l |tail -n 1 | tr -d '\n'`
# Get the abbreviated commit hash of the latest commit.
COMMIT=`git rev-parse --short HEAD|tr -d '\n'`

# Updating version.h
echo "#define VERSION \""${RELEASE}"\"" >version.h
echo "#define LONG_VERSION \""${BASENAME}" version "${RELEASE}" (commit "${COMMIT}")\"" >>version.h
