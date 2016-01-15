#
# Profile for uclinux
# Set platform specific configs in here
#
PREFIX=arm-uclinuxeabi-
CC=$(PREFIX)gcc
LIB=$(TOOLS)/bin/ar cr
LIB=$(PREFIX)ar cr
RANLIB=$(PREFIX)ranlib
STRIP=$(PREFIX)strip

