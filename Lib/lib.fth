
0 constant CPU_UNKNOWN
1 constant CPU_X86
2 constant CPU_PPC
3 constant CPU_8XX
4 constant CPU_AMD64
5 constant CPU_SPARC
6 constant CPU_COLDFIRE
7 constant CPU_ARM
8 constant CPU_MIPS

0 constant OS_UNKNOWN
1 constant OS_LINUX
2 constant OS_DARWIN
3 constant OS_FREEBSD
4 constant OS_SOLARIS
5 constant OS_UCLINUX
6 constant OS_QNX
\
\ fcntl definitions
\
0x004 constant O_NONBLOCK
04 constant F_SETFL
\
\ SYSV IPC definitions
\
0x800 constant IPC_NOWAIT \ 004000 octal
\
\ Mask for setting, clearing and testing 16 bit values.
\
create mask 1 , 2 , 4 , 8 , 0x10 , 0x20 , 0x40 , 0x80 , 0x100 , 0x200 , 0x400 , 0x800 , 0x1000 , 0x2000 , 0x4000 , 0x8000 ,
\
\ mask for 8 bit values
\
create bmask 0x01 c, 0x02 c, 0x04 c, 0x08 c, 0x10 c, 0x20 c, 0x40 c, 0x80 c,
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

: (bsetbit) { bit addr }
	bmask bit + c@
	addr c@ or
	addr c!
;

: (clrbit) { bit addr }
	mask bit cells + @ invert
	addr @ and
	addr !
;

: (bclrbit) { bit addr }
	bmask bit + c@ invert
	addr c@ and
	addr c!
;

: (getbit) { bit addr }
    mask bit cells + @ 
	addr w@ and 0<>
;

: (bgetbit) { bit addr }
    bmask bit + c@ 
	addr c@ and 0<>
;

: (btogglebit) { bit addr }
    bmask bit + c@ 
	addr c@ xor
    addr c!
;

\ create a buffer for n chars/bytes
\ 
: cbuffer ( n -- )
create
    chars allot
does>
;

\ create a buffer for n cells
: buffer ( n -- )
create
     cells allot
 does>
;

: mk-buffer ( size -- addr size )
    dup allocate abort" Allocate failed"
    swap 2dup erase
;
\
\ Defined as primitive (13 April 2015)
\
\ : addcr ( addr len -- addr len+1 )
\     2dup
\     +       \ addr len ptr
\     0x0a swap c! \ addr len
\     1+
\ ;



: >counted \ addr n target -- 
\ *G Covert a string of the form:
\ *P addr n
\ *P To a counted string where the first byte is the length at the target address
\
    2dup c!  \ addr n target
    1+
    swap
    move
;


: c!++ ( bb addr -- addr+1 )
\ *G Write bb to addr, increment addr and leave thae addres on the stack.
    dup >r
    c!
    r> 1+
;

: wbsplit \ ww -- ll hh
\ *G Split a 16 bit word into two bytes.
    dup 0xff and
    swap 0xff00 and 8 rshift
;

: bwjoin ( ll hh -- word )
\ *G Join two bytes into a 16 bit word.
    0xff and 8 lshift swap
    0xff and or
;

: wljoin ( lw hw -- long )
\ *G Join two 16 bit words into a 32 bit cell.
    0x10 lshift or
;

: bljoin ( b.low b2 b3 b.hi -- long )
\ *G Join four 8 bit bytes into a 32 bit cell.
    bwjoin -rot
    bwjoin swap
    wljoin
;


: between ( n min max -- flag )
\ *G Return true if min <= n <= max.
    rot >r  \ min max
    r@      \ min max n
    < invert      \ min flag
    swap r> \ flag min n
    > invert and
;

: bounds  
  over + swap 
; 

: [defined]  
    bl word find nip 0<> 
; immediate 

: [undefined] 
    postpone [defined] 
    0= 
; immediate
\
\ Create  a counted string holding the newline sequence
\ 
create (newline) 1 c, 10 c,

: newline (newline) count ;

: cell 1 cells ;

: char 1 chars ;

