#!/bin/sh

## usage: extract-files.sh $1 $2
## $1 and $2 are optional
## if $1 = unzip the files will be extracted from zip file (if $1 = anything else 'adb pull' will be used
## $2 specifies the zip file to extract from (default = ../../../${DEVICE}_update.zip)

VENDOR=samsung
DEVICE=galaxy5

BASE=../../../vendor/$VENDOR/$DEVICE/proprietary
rm -rf $BASE/*

if [ -z "$2" ]; then
ZIPFILE=../../../${DEVICE}_update.zip
else
ZIPFILE=$2
fi

if [ "$1" = "unzip" -a ! -e $ZIPFILE ]; then
echo $ZIPFILE does not exist.
else
for FILE in `cat proprietary-files.txt | grep -v ^# | grep -v ^$`; do
DIR=`dirname $FILE`
if [ ! -d $BASE/$DIR ]; then
mkdir -p $BASE/$DIR
fi
if [ "$1" = "unzip" ]; then
unzip -j -o $ZIPFILE system/$FILE -d $BASE/$DIR
else
adb pull /system/$FILE $BASE/$FILE
fi
done
fi
./setup-makefiles.sh
