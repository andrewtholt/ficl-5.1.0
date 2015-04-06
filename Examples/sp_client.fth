


0 value connected

-1 value pid
-1 value pstdin
-1 value pstdout
-1 value pstderr

-1 value buffer


: init
    buffer 255 allocate abort" Allocate failed" to buffer
    buffer 255 erase
    s" toSpread" popen to pid
        to pstdin
        to pstdout
        to pstderr
;

