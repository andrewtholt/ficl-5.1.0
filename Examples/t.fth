load lib.fth
load struct.fth

struct 
    cell field func-name
    cell field func-len
    cell field func-ptr
    char field in_p
    char field out_p
endstruct /dynamic


: mkfunc ( result-cnt arg-cnt func-name len handle -- )
create 
    dlsym abort" Failed to find function." \ rc ac fp
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


