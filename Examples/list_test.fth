load mydump.fth

-1 value list
-1 value ptr

255 allocate abort" allocate failed." to ptr

mklist to list

list list-len . cr

s" testing" list list-append-string drop
s" testing" list list-append-string drop
s" 1 2 3"   list list-append-string drop
42 list list-append-int drop

list list-len . cr

list list-display

ptr list list-get 

list list-len . cr
list list-display

\ list rmlist

