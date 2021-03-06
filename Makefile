OBJECTS= dictionary.o system.o fileaccess.o float.o double.o prefix.o search.o softcore.o stack.o tools.o vm.o primitives.o bit.o lzuncompress.o unix.o utility.o hash.o callback.o word.o extras.o libser.o
HEADERS= ficl.h ficlplatform/unix.h
#
# Flags for shared library
#
#TARGET= -DLINUX -DDYNLIB -DFICL_WANT_LZ_SOFTCORE=0 -DPOSIX_IPC -DSYSV_IPC -DLIBMODBUS -DSOCKET -DINIPARSER # -DI2C # -DFICL_WANT_FLOAT=0 

include profile.mk
# TARGET= -DLINUX -DDYNLIB -DFICL_WANT_LZ_SOFTCORE=0 -DSOCKET -DPOSIX_IPC # -DSYSV_IPC # -DLIBMODBUS -DINIPARSER 

SHFLAGS = -fPIC

CFLAGS += $(SHFLAGS)#  -Wno-format
INCLUDES += -I. -I/usr/local/include
CPPFLAGS= $(TARGET) $(INCLUDES) # -I. -I/usr/local/include
# CC = cc -g -Wl,--no-as-needed
# CC = clang -g 
LIB = ar cr
RANLIB = ranlib

# ifeq ($(CC),gcc)
# 	@echo "Compiler is gcc"
# 	CFLAGS +="-g -Wl,--no-as-needed"
# endif
# 
# ifeq ($(CC),clang)
# 	@echo "Compiler is clang"
# 	CFLAGS +="-g"
# endif


# LIBS=-L/usr/local/lib -lmodbus -liniparser

MAJOR = 4
MINOR = 1.0

all:	lib ficl # ficls

ficl: main.o $(HEADERS) libficl.a
	@echo $(CFLAGS)
	$(CC) $(CFLAGS) main.o -o ficl -L. -lficl -lm -lrt -lpthread $(LIBS) # -L/usr/local/lib -lmodbus -liniparser

ficls: main.o $(HEADERS) libficl.so.$(MAJOR).$(MINOR)
	$(CC) $(CFLAGS) --static main.o -o ficls -L. -lficl -lm -ldl

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
	rm -f *.o *.a libficl.* cscope.out ficl ficls


install:	ficl libficl.so.$(MAJOR).$(MINOR)
	cp ./ficl /usr/local/bin
	strip /usr/local/bin/ficl
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

deb:	ficl
	./install-deb-x86_64 

local_install:	ficl libficl.so.$(MAJOR).$(MINOR)
	strip ./ficl
	mkdir -p $(HOME)/bin
	mkdir -p $(HOME)/lib/ficl
	mkdir -p $(HOME)/include/ficlplatform
	mkdir -p $(HOME)/lib/ficl
	cp ./ficl $(HOME)/bin
	cp ficl.h $(HOME)/include
	cp ficllocal.h $(HOME)/include
	cp ficltokens.h $(HOME)/include
	cp ficlplatform/unix.h $(HOME)/include/ficlplatform
	cp libficl.so.$(MAJOR).$(MINOR) $(HOME)/lib
	cp libficl.a $(HOME)/lib
	ln -sf $(HOME)/lib/libficl.so.$(MAJOR).$(MINOR) $(HOME)/lib/libficl.so.$(MAJOR)
	ln -sf $(HOME)/lib/libficl.so.$(MAJOR).$(MINOR) $(HOME)/lib/libficl.so

help:
	@echo "Targets:"
	@printf "\tficl\tExecutable.\n"
	@printf "\tdeb\tDebian package.\n"
