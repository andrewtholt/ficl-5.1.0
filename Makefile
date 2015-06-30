OBJECTS= dictionary.o system.o fileaccess.o float.o double.o prefix.o search.o softcore.o stack.o tools.o vm.o primitives.o bit.o lzuncompress.o unix.o utility.o hash.o callback.o word.o extras.o libser.o
HEADERS= ficl.h ficlplatform/unix.h
#
# Flags for shared library
TARGET= -DLINUX -DMUSL -DSYSV_IPC -DSOCKET
SHFLAGS = -fPIC
CFLAGS= $(SHFLAGS) -g -m32
CPPFLAGS+= $(TARGET) -I. -I/home/andrewh/Source/openwrt/staging_dir/toolchain-i386_i486_gcc-4.8-linaro_musl-1.1.10/include

LDFLAGS=-Wl,--no-as-needed

CC = gcc
CFLAGS= $(SHFLAGS) -g -m32
AR = ar cr
LIB = ar cr
RANLIB = ranlib

MAJOR = 4
MINOR = 1.0

all:	lib ficl # ficls

# ficl: main.o $(HEADERS) libficl.a
#	$(CC) $(CFLAGS) main.o -o ficl -L. -lficl -lm -ldl 
#
# Build statically linked ficls.
#
ficl: main.o $(HEADERS) libficl.a
	$(CC) $(CFLAGS) main.o -o ficl -lm -ldl -L. -lficl -lm
#	$(CC) $(CFLAGS) --static main.o -o ficl -L. -lficl -lm

lib: libficl.so.$(MAJOR).$(MINOR)

# static library build
libficl.a: $(OBJECTS)
	$(LIB) libficl.a $(OBJECTS)
	$(RANLIB) libficl.a

# shared library build
libficl.so.$(MAJOR).$(MINOR): $(OBJECTS)
	$(CC) $(CFLAGS) -shared -Wl,-soname,libficl.so.$(MAJOR).$(MINOR) \
	-o libficl.so.$(MAJOR).$(MINOR) $(OBJECTS)
#	ln -sf libficl.so.$(MAJOR).$(MINOR) libficl.so

main: main.o ficl.h libficl.so.$(MAJOR).$(MINOR)
	$(CC) $(CFLAGS) main.o -o main -L. -lficl -lm
	ln -sf libficl.so.$(MAJOR).$(MINOR) libficl.so.$(MAJOR)

unix.o: ficlplatform/unix.c $(HEADERS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ ficlplatform/unix.c

#
#       generic object code
#
.SUFFIXES: .cxx .cc .c .o

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

.cxx.o:
	$(CPP) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

.cc.o:
	$(CPP) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
#
#       generic cleanup code
#
clean:
	rm -f *.o *.a libficl.* ficl cscope.out


install:        ficl libficl.so.$(MAJOR).$(MINOR) libficl.a
	strip ./ficl
	cp ./ficl /usr/local/bin
	cp ficl.h /usr/local/include
	cp ficllocal.h /usr/local/include
	cp ficltokens.h /usr/local/include
	mkdir -p /usr/local/include/ficlplatform
	cp ficlplatform/unix.h /usr/local/include/ficlplatform
	cp libficl.so.$(MAJOR).$(MINOR) /usr/local/lib
	cp libficl.a /usr/local/lib
	ln -sf /usr/local/lib/libficl.so.$(MAJOR).$(MINOR) /usr/local/lib/libficl.so.$(MAJOR)
	ln -sf /usr/local/lib/libficl.so.$(MAJOR).$(MINOR) /usr/local/lib/libficl.so
	mkdir -p /usr/local/lib/ficl
	cp Lib/* /usr/local/lib/ficl


