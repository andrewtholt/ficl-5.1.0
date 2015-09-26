

-1 value ptr
0 value initRun
-1 value file
-1 value file1

-1 value buffer
255 constant /buffer

: init
    initRun 0= if
        16 allocate abort" allocate failed" to ptr
        ptr 16 erase

        /buffer allocate abort" allocate failed" to buffer
        buffer /buffer erase

        s" /tmp/crap.txt" r/w open-file abort" open-file" to file
        s" /tmp/crap1.txt" r/w open-file abort" open-file" to file1

        -1 to initRun


        ." Before" cr
        ptr 8 dump
    then
;

: main
    init

    ptr 16 erase
    2 ptr c!
    file fd@ ptr 1+ c!
    file1 fd@ ptr 2+ c!
    file fd@ . cr
    
    ." Poll" cr
    ptr poll
    ." After" cr
    ptr 8 dump

    ptr 1+ c@ 1 = if
        ." file" cr
        buffer /buffer file read-line abort" read-file" 0<> if
            buffer /buffer dump
            buffer /buffer erase
        then
    then

    ptr 2+ c@ 1 = if
        ." file1" cr
        buffer /buffer file1 read-line abort" read-file" 0<> if
            buffer /buffer dump
            buffer /buffer erase
        then
    then

\    buffer 16 file read-file abort" read-file" .s

\    buffer 16 dump

\    16 allocate abort" allocate failed" to buffer
\    buffer 16 erase
;
