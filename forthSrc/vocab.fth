
vocabulary testing
only forth also testing definitions

0 value interact

: bye-bye
    interact 0= if bye then
;

: hi
    ." Hello there !!!" cr
    bye-bye
;

: interactive
    -1 to interact
;

: seal
    seal
;

: bye
    bye
;

: words
    words
    bye-bye
;

: .ver
    .ver
    bye-bye
;

: tab
    09 emit
;

: help
    tab tab
    ." Help" cr cr
    tab ." help: This." cr
    tab ." hi  : Wellcome message." cr
    tab ." bye : Disconnect." cr
    cr
    bye-bye
;


only testing
seal

