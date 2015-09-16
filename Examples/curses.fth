

-1 value mainwin

: test
    curses-init abort" Ooops" to mainwin

    10 10 curses-move
    s" Hello" curses-addstr
    curses-refresh

    curses-getch
    curses-clear
    curses-refresh

    20 10 curses-move
    s" Again" curses-addstr
    curses-refresh
    2000 ms

    mainwin curses-end
    cr
    ." Done " cr
    .
;

test

