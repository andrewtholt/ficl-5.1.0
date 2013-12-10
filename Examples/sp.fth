load lib.fth
\ load mydump.fth

os .
cr

0. 2value user
0. 2value group
0. 2value buffer
255 constant /buffer
-1 value mbox



/buffer allocate abort" Failed to allocate space" /buffer to buffer

buffer erase

: init
    s" andrewh:4803" sp-connect abort" SP-connect failed" to mbox
    s" POWER" strsave to group
    mbox group sp-join

;

: main
    begin
        mbox sp-poll 0<>
    while
        mbox buffer sp-recv drop buffer drop swap type cr
    repeat
;

init

vocabulary toSpread
only forth also toSpread definitions
here 255 allot constant scratch
scratch 255 erase

: >cstring ( addr len -- cstring )
    dup scratch c!
    scratch 1+
    swap move
;

: $^get ( addr len -- )
    >cstring
    scratch 10 dump
;

: ^get
    bl parse $^get
;

: bye bye ;
seal


\ -1 value spread

\ os OS_LINUX = [if]
\    .( LINUX ) cr
\    s" /usr/local/lib/libspread.so"
\    s" /usr/local/spread-4.0.0/lib/libspread.so"
\     
\[else]
\    s" /usr/local/lib/libspread.dylib"
\ [then]
\ dlopen ?dup 0= abort" Library error"
\ .s
\ to spread

\ 255 cbuffer sp
\ s" 4803" sp swap move
\ 
\ s" fred" sp 4 spread sp-connect 0= abort" sp-connect" value mbox
\ 
\ mbox s" global" spread sp-leave abort" sp-leave"
\ 
\ .s
\ 
