\ 
\ signal.fth
\ 
\ Enable interception of a signal (in this case 14 SIGALRM)
\ 
\ Then set a short alarm, and enter a blocking read
\ The alarm 'fires' causing exit from the read.
\ 
\ The retrieve the cause of the signal.
\ 
\ 

-1 value fd
-1 value buffer

: init
    14 signal
    255 allocate abort" allocate failed." to buffer

    buffer 255 erase
    s" /dev/tty" r/w open-file abort" Open file failed." to fd

;

: test
    fd 0< if
        ." Open file." cr
        init
    then

    1 alarm
    clr-errno

    buffer 255 fd read-line 
    cr .s cr
    drop 2drop

    ." Errno is      :" errno . cr
    ." Last-signal is:" last-signal . cr


;
