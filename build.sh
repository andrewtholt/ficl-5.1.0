#!/bin/sh

# set -x

ARCH=`uname -m`


while getopts a:hx: flag; do
    case $flag in
        a)
            ARGS=$OPTARG
            ;;
        h)
            echo "Help."
            printf "\t-a <makefile args>\n"
            printf "\t-h\t\tHelp.\n"
            printf "\t-x <makefile arch>\n"

            exit 0
            ;;
        x)
            ARCH=$OPTARG
            ;;
    esac
done

MAKEFILE=Makefile.${ARCH}

if [ -f $MAKEFILE ]; then
    echo "Building with $MAKEFILE"
	make -j 4 -f $MAKEFILE $ARGS
else
	echo "$MAKEFILE does not exist."
fi
