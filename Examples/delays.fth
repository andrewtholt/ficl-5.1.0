
2147483647 constant biggest
500 constant interval

: later ( n -- n' )
    elapsed +
;
\ Here's the idea.
\ A 32 bit timer, that count milliseconds will rolll over fairly quickly.
\ So to construct an interval timer:
\ 1. Read the count now, and store it
\ 2. Read the count again, subtract it from the stored time and take tha abs value.
\ 3. If the difference from abive is greater than the interval the timer has expired.
\
\ return tru if time expired
: expired ( start duration -- f )
\    elapsed <
    swap elapsed - abs \ dur e
    <
;

: timedout?
    expired
;

-1 value timer1
-1 value timer2



: run1
    timer1 0 < if
        interval later to timer1
    then
    timer1 interval expired if 
        [char] 1 emit -1 to timer1
    then
;

: run2
    timer2 0 < if
        interval later to timer2
    then
    timer2 interval expired if 
        [char] 2  emit -1 to timer2
    then
;

: main
    now
    begin
        run1
        run2
        1 ms
\        timer1 . cr
\        timer2 . cr
    again
;

