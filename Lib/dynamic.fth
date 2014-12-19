load lib.fth
load struct.fth

struct 
    cell field func-name
    cell field func-len
    cell field func-ptr
    char field in_p
    char field out_p
endstruct /dynamic


: mkfunc ( result-cnt arg-cnt func-ptr -- )
create 
    rot ,
    swap ,
    ,
does>
    dup @ 
    swap cell+ dup @
    swap cell+ @ 
    dlexec
;

: tst ( a b c  -- )
create 
    rot ,
    swap ,
    ,
does>
    dup @ 
    swap cell+ dup @
    swap cell+ @
;

\ 
\ s" libmine.so" dlopen abort" What." value libmine
\ 
\ 1 1 s" test1" libmine dlsym abort" Not found" mkfunc test1
\ 
