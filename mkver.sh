#!/bin/sh
# Shell to create the defines VERSION and LONG_VERSION.
# Redirect the output of this script to version.h.

BASENAME=lamprop-c
RELEASE=`git tag -l |tail -n 1 | tr -d '\n'`

echo "#define VERSION \""${RELEASE}"\""

echo -n "#define LONG_VERSION \""${BASENAME}" version "${RELEASE}" (commit "
git log -n 1 --pretty=%h|tr -d '\n'
echo ")\""

echo "#define RELEASE_NAME \""${BASENAME}"-w64-"${RELEASE}"\""
