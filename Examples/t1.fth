
0x1b constant ESC

0 value main-exit

: cls
    ESC emit
    ." [2J"
    
;

: cursor ( t|f -- )
    ESC emit
    if
        ." [?25h"
    else
        ." [?25l"
    then
;

: home
    ESC emit
    ." [f"
;

: goto-xy ( row col )
    swap
    ESC emit
    ." ["
    .
    ." ;"
    .
    ." H"
    
;


: tst

    begin
        cls home
    
        5 20 goto-xy ." 1: Test LEDs"
        6 20 goto-xy ." 2: Test Relays"
        8 20 goto-xy ." q: Quit       "
     
        10 22 goto-xy
        1 cursor
        ." prompt >" key
        0 cursor
    
    
        15 22 goto-xy
        case
            [char] 1 of 
                ." You pressed one." cr
                500 ms
            endof
            [char] 2 of 
                ." You pressed two. "
                500 ms
            endof
            [char] q of
                ." You pressed quit." cr
                -1 to main-exit
            endof
            07 emit
        endcase

\        stdout-flush
        main-exit
    until
    1 cursor
    bye
;


