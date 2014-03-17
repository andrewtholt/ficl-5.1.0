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
            echo "\t-a <makefile args>"
            echo "\t-h\t\tHelp."
            echo "\t-x <makefile arch>"

            exit 0
            ;;
        x)
            ARCH=$OPTARG
            ;;
    esac
done

MAKEFILE=Makefile.${ARCH}

if [ -f $MAKEFILE ]; then
	make -f $MAKEFILE $ARGS
else
	echo "$MAKEFILE does not exist."
fi
