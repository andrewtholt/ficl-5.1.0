-1 value serial
-1 value buffer
0 value init-done

: d=
    rot = -rot = and
;

: init
    255 allocate abort" Memory allocation failed" to buffer

    buffer 255 erase

    s" /dev/tty.KeySerial1" 115200 open-serial to serial
    -1 to init-done
;

: nothing? ( cnt err -- flag )
    0 35 d= 
;

: get-stuff
    buffer 32 serial read-file 2dup \ cnt err cnt err
    nothing? if
        cr ." nothing" 2drop
    else
        cr . cr .
    then
    cr
    buffer 64 dump
;

: main
    init-done 0= if
        init
    then
    get-stuff
;

main
