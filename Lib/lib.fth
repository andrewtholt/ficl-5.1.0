load mydump.fth
\
\ A few constants
\
0 constant CPU_UNKNOWN
1 constant CPU_X86
2 constant CPU_PPC
3 constant CPU_8XX
4 constant CPU_AMD64
5 constant CPU_SPARC
6 constant CPU_COLDFIRE

0 constant OS_UNKNOWN
1 constant OS_LINUX
2 constant OS_DARWIN
3 constant OS_FREEBSD
4 constant OS_SOLARIS
5 constant OS_UCLINUX
\
\ Mask for settin, clearing and testing 16 bit values.
\
create mask 1 , 2 , 4 , 8 , 0x10 , 0x20 , 0x40 , 0x80 , 0x100 , 0x200 , 0x400 , 0x800 , 0x1000 , 0x2000 , 0x4000 , 0x8000 ,
\
\ Print the top of the stack in hex, irrespective of the system base.
\
: .h
        base @ >r
        hex . r> base !
;

\
\ Print the top of the stack in decimal irrespective of the system base.
\
: .d
        base @ >r
        decimal . r> base !
;

\
\ Print the top of the stack in octal irrespective of the system base.
\
: .o 
	base @ >r
	8 base ! . r> base !
;

\
\ Print the top of the stack in binary irrespective of the system base.
\
: .b
        base @ >r
        2 base !
        . r> base !
;
\
\ set base to octal
\
: octal
	8 base !
;
\ 
\ raw set, clr and get bits.
\
: (setbit) { bit addr }
	mask bit cells + @
	addr @ or
	addr !
;

: (clrbit) { bit addr }
	mask bit cells + @ invert
	addr @ and
	addr !
;

: (getbit) { bit addr }
        mask bit cells + @ 
	addr w@ and 0<>
;

: mk-buffer ( size -- addr size )
    dup allocate abort" Allocate failed"
    swap 2dup erase
;

: bounds ( addr len -- addr+len addr )
    over + swap
;


