#!/bin/sh

# set -x

STATUS=$(which rlwrap > /dev/null)

if [ $STATUS -eq 0 ]; then
    echo "Can't find rlwrap."
    exit -1
fi

if [ $# -eq 0 ]; then
    rlwrap -n ficl
else
    rlwrap -n ficl  $*
fi
