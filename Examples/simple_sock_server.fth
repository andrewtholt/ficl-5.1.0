
-1 value sock1
-1 value sock2

-1 value buffer

: main
    255 allocate abort" Buffer" to buffer
    buffer 255 erase

    socket to sock1

    clr-errno
    9091 sock1 socket-bind if
        perror
        abort
    then

    sock1 socket-listen
    sock1 socket-accept to sock2

    buffer 255 sock2 socket-recv 

    .s
;

main
