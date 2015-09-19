

-1 value mainwin
-1 value mywin

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

    10 10 25 15 curses-newwin to mywin
    mywin curses-box
\    mywin curses-touchwin

    mywin 1 1 curses-wmove
    mywin s" Hello" curses-waddstr
    mywin curses-wrefresh
    1000 ms
    mywin curses-wclear
    mywin 2 1 curses-wmove
    mywin s" Hello" curses-waddstr
    mywin curses-box
    mywin curses-wrefresh

    2000 ms

    mywin curses-delwin
    curses-clear
    21 10 curses-move
    s" More " curses-addstr
    curses-refresh
    2000 ms

    mainwin curses-end
    cr
    ." Done " cr
    .
;


