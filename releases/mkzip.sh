#!/bin/sh
# Create a zip-file from the latest release

ZIPNAME=`ls -d lamprop* | tail -n 1|sed 's/\/$//'`

rm -f *.zip
zip -qr ${ZIPNAME}.zip ${ZIPNAME}/
