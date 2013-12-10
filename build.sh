#!/bin/sh

# set -x

ARCH=`uname -m`

MAKEFILE=Makefile.${ARCH}

if [ -f $MAKEFILE ]; then
	make -f $MAKEFILE $*
else
	echo "$MAKEFILE does not exist."
fi
