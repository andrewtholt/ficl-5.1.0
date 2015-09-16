#!/bin/sh

# set -x

CPU=`uname -m`
OS=`uname -s`

echo "building for $CPU"
echo "       OS is $OS"

if [ "$OS" = "Linux" ]; then
    ARCH=$CPU
else
    ARCH="${OS}_${CPU}"
fi

LIST="NO"
echo 
while getopts la:hx:o:p: flag; do
    case $flag in
        a)
            ARGS=$OPTARG
            ;;
        h)
            echo "Help."
            printf "\t-a <makefile args>\n"
            printf "\t-h\t\tHelp.\n"
            printf "\t-o <variant>\tBuild a variant based on an architecture\n"
            printf "\t-p <profile>\tUse the profile to set defines, and libraries.\n"
            printf "\t-x <makefile arch>\n"
            printf "\t-l\t\tList available profiles\n"

            exit 0
            ;;
        o)
            OPT=_${OPTARG}
            ;;
        p)
            PROFILE=${OPTARG}.mk
            ;;
        l)
            LIST="YES"
            ;;
        x)
            ARCH=${OPTARG}
            ;;
    esac
done

if [ $LIST = "YES" ]; then
    echo "List of Profiles"
    echo
    P=$(ls *.mk | egrep -v "ficl|profile" | cut -f1 -d'.' )

    for A in $P; do
        echo $A
    done
    exit
fi


if [ ! -f profile.mk ]; then
    ln -s ./basic.mk ./profile.mk
fi

if [ ! -z "$PROFILE" ]; then
    echo "Profile set"

    if [ -f "profile.mk" ]; then
        echo "Profile exists"
        rm ./profile.mk
    fi
    ln -s ./$PROFILE ./profile.mk
fi

MAKEFILE=Makefile.${ARCH}${OPT}

if [ -f $MAKEFILE ]; then
    echo "Building with $MAKEFILE"
    PROFILE=$(readlink ./profile.mk)
    echo "Profile $PROFILE"
    echo "=========================="
    sleep 1
	make -j 4 -f $MAKEFILE $ARGS
else
	echo "$MAKEFILE does not exist."
fi
