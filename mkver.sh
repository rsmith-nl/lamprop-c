#!/bin/sh
# Shell to create the defines VERSION and LONG_VERSION.
# Redirect the output of this script to version.h.

BASENAME=lamprop-c
RELEASE=`git tag -l |tail -n 1 | tr -d '\n'`

echo "#define VERSION \""${RELEASE}"\""

echo -n "#define LONG_VERSION \""${BASENAME}" version "${RELEASE}
awk '/refs\/heads\/main/ {print " (commit "substr($1,1,7)")\""}' .git/info/refs

echo "#define RELEASE_NAME \""${BASENAME}"-w64-"${RELEASE}"\""
