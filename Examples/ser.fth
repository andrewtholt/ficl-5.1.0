load lib.fth
load strings.fth

-1 value serial
-1 value buffer
255 constant /buffer

-1 value ssid
32 constant /ssid

-1 value pwd
64 constant /pwd
0 value init-done

: d=
    rot = -rot = and
;

: init
    /buffer allocate abort" Memory allocation failed" to buffer
    buffer /buffer erase

    /ssid allocate abort" Memory allocation failed" to ssid
    ssid /ssid erase

    /pwd allocate abort" Memory allocation failed" to pwd
    pwd /pwd erase

    s" /dev/ttyUSB0" 115200 open-serial to serial

    c" Office" dup c@ 1+ ssid swap move
    c" anthony050192" dup c@ 1+  pwd swap move

    -1 to init-done
;

: nothing? ( cnt err -- flag )
\    0 35 d= 
    0 11 d= 
;

: get-stuff
    buffer 32 serial read-file 2dup \ cnt err cnt err
    nothing? if
        cr ." nothing" 2drop
    else
        cr . cr .
    then
    cr
    buffer 32 dump
;

: main
    init-done 0= if
        init
    then
    get-stuff
;

: add-crlf ( addr len -- addr len + 2)
    2dup ( a l a l )
    +    ( a l b )
    dup     ( a l b b )
    0x0d swap c! ( a l b )
    1+ 0x0a swap c! ( a l )
    2+
;

: send-stuff ( addr len )
    serial write-file abort" at cmd write failed"
    buffer 32 erase
    10 ms
    get-stuff
;

\ 
\ Stack: flag
\ If flag is true, enable echo, otherwise disable.
\ 
: at-echo 
    if
        s" ATE1" 
    else
        s" ATE0"
    then

    buffer swap move
    buffer 4 add-crlf

    serial write-file abort" at-echo: write failed"
    buffer 32 erase
;

\ 
\ Set station mode
\ 1 =  Station (client)
\ 2 =  AP
\ 3 =  Both
: at-cwmode 
    0x30 +
    s" AT+CWMODE=" buffer swap move
    10 buffer + c!

    buffer 11 add-crlf
    send-stuff
    20 ms
    get-stuff
;

: at-connect { | ptr }
    s" AT+CWJAP=" buffer swap move
    buffer 9 + to ptr

    [char] " ptr c!++ to ptr

    ssid count dup >r
    ptr swap move
    ptr r> + to ptr

    [char] " ptr c!++ to ptr
    [char] , ptr c!++ to ptr
    [char] " ptr c!++ to ptr


    pwd count dup >r ptr swap move r>
    ptr + to ptr

    [char] " ptr c!++ to ptr
    0x0d ptr c!++ to ptr
    0x0a ptr c!++ to ptr

    ptr .h cr


    buffer 48 dump
;


init
\ main
