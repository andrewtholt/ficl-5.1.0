load struct.fth
load events.fth

2048 constant O_NONBLOCK \ 04000 in ocatl

0 constant F_DUPFD ( Duplicate file descriptor.  )
1 constant F_GETFD ( Get file descriptor flags.  )
2 constant F_SETFD ( Set file descriptor flags.  )
3 constant F_GETFL ( Get file status flags.  )
4 constant F_SETFL ( Set file status flags.  )

-1 value buffer
-1 value file
-1 value fd
0 value syn-flag

\ 0 constant EV_SYN
\ 2 constant EV_REL


0 constant X_REL
1 constant Y_REL

struct
    16 chars field ev-timestamp
    2 chars field ev-type
    2 chars field ev-code
    4 chars field ev-value
endstruct event-test


event-test constant /buffer

: .code { buff }
    buff ev-code drop w@
    case
        X_REL of buff ev-value drop w@ . cr endof
        Y_REL of buff ev-value drop w@ . cr endof
    endcase
;

: .rel { buff }   
    ." EV_REL "
    buff ev-code drop w@ 
    case
        X_REL of ." X_REL " buff .code endof
        Y_REL of ." Y_REL " buff .code endof
    endcase
;

: .event { addr }
    addr ev-type drop w@ 
    case
        EV_SYN of ." EV_SYN " endof
        EV_REL of buffer .rel  endof
        EV_KEY of ." EV_KEY rx " addr ev-code drop w@ . cr endof
    endcase

    cr
;

: nonblock
    fd F_GETFL 0 fcntl .s
    O_NONBLOCK or
    >r fd F_SETFL r> fcntl abort" fcntl failed."
;

: init
    /buffer allocate abort" Allocate failed." to buffer
    buffer /buffer erase

    s" /dev/input/event3" r/o open-file abort" Open failed" to file
    file fd@ to fd
;

: test
\    s" Testing 1 2 3" file write-line abort" Write failed"
\    file flush-file

\    1000 ms
    buffer /buffer erase
    buffer /buffer file read-file abort" read failed." 
    buffer swap dump
    cr
    buffer .event
    ." =======================" cr
;

: main
    init
    begin
        test
        buffer ev-type drop w@ 0=
    until

    begin test again
;


