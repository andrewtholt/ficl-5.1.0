load struct.fth

: cell 
    1 cells
;

: char
    1 chars
;

struct 
    cell field lib-name
    char field lib-len
    char field in_p
    char field out_p
    cell field lib-func
endstruct dynamic

-1 value func

: create-struct
    create
        dynamic allocate abort" Allocate failed" dup ,
        dynamic erase
    does> @
;

: test ( string len dest-ptr )
    
;
