#!/bin/sh

# set -x

ARCH=`uname -m`

MAKEFILE=Makefile.${ARCH}

if [ -f $MAKEFILE ]; then
	make -f $MAKEFILE $1
else
	echo "$MAKEFILE does not exist."
fi
