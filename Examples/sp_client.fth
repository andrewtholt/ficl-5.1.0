load linux.fth


0 value connected

-1 value pid
-1 value pstdin
-1 value pstdout
-1 value pstderr

-1 value buffer

: nonblock ( fd -- )
    dup F_GETFL 0 fcntl O_NONBLOCK or F_SETFL swap fcntl
;

: init
    buffer 255 allocate abort" Allocate failed" to buffer
    buffer 255 erase
    s" toSpread" popen to pid
        to pstdin
        to pstdout
        to pstderr

    drop
;

: read_stdout
    buffer 255 erase
    buffer 255 pstdout read-line 
    nip

    .s cr

    abort" read_stdout error"

    buffer swap 2dup dump
    evaluate
;

