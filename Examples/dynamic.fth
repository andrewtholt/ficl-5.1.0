\ 
\ Dynamic library 
\ 
\ Initialise a structure and call functions dynamically
\ 
\ A C function declared "void name(void)" only needs to 
\ define the library and function name as the in & out params
\ default to 0.
\ 
load lib.fth
load struct.fth

: cell 
    1 cells
;

: char
    1 chars
;

struct 
    cell field func-name
    cell field func-len
    cell field func-ptr
    char field in_p
    char field out_p
endstruct /dynamic

-1 value func

: create-struct
    create
        /dynamic allocate abort" Allocate failed" dup ,
        /dynamic erase
    does> @
;
\ 
: set-func-name { string len struct-ptr }
    len allocate abort" test:allocate failed" \ addr
    >r
    string r@ len move
    r> struct-ptr func-name drop !
    len struct-ptr func-len drop c!
;
\ 
: get-func-name ( struct-ptr -- string len )
    dup >r func-name drop @
    r> func-len drop c@
;
\ 
: set-func-ptr ( ptr struct-ptr )
    func-ptr drop !
;

: get-func-ptr ( struct-ptr -- func-ptr )
    func-ptr drop @
;
\ 
: set-inputs ( n struct-ptr -- )
    in_p drop c!    
;

: get-inputs ( struct-ptr -- n )
    in_p drop c@    
;

: set-outputs ( n struct-ptr -- )
    out_p drop c!    
;

: get-outputs ( struct-ptr -- n )
    out_p drop c@    
;
\ 
\ 
: (dllookup) { fred handle }
    fred get-func-name handle dlsym abort" Lookup failed"
    fred set-func-ptr
;
\ 
: dynamic-dump { fred }
    cr
    ." Function Name   : " fred get-func-name type cr
    ." Function Pointer: " fred get-func-ptr .h cr
    ." Inputs          : " fred get-inputs . cr
    ." Outputs         : " fred get-outputs . cr
;

: setup-func { fred handle }
    fred handle (dllookup)
;


: define-function { func-name len ins outs handle func-struct }
    func-name len func-struct set-func-name
    ins func-struct set-inputs
    outs func-struct set-outputs
    func-struct handle setup-func
;

-1 value libmine
s" libmine.so" dlopen abort" Failed to open lib." to libmine

create-struct (test1)
s" test1" 1 1 libmine (test1) define-function

(test1) dynamic-dump
(test1) /dynamic dump
\ 
: test1
    1 1 (test1) get-func-ptr dlexec
;
\ 
5 test1
