
-1 value sock1
-1 value sock2

-1 value buffer
-1 value run-flag

: main
    255 allocate abort" Buffer" to buffer
    buffer 255 erase

    socket to sock1

    clr-errno
    9090 sock1 socket-bind if
        perror
        abort
    then

    sock1 socket-listen abort" listen error."
    sock1 socket-accept to sock2

    begin
        run-flag 
    while
        buffer 255 sock2 socket-recv ?dup 0<> if 
            buffer swap dump
        else
            0 to run-flag
        then
    repeat
;

main
