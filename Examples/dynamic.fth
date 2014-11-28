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
    cell field lib-name
    cell field lib-len
    cell field lib-handle
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

: set-lib-name { string len struct-ptr }
    len allocate abort" test:allocate failed" \ addr
    >r
    string r@ len move
    r> struct-ptr lib-name drop !
    len struct-ptr lib-len drop c!
;

: set-func-name { string len struct-ptr }
    len allocate abort" test:allocate failed" \ addr
    >r
    string r@ len move
    r> struct-ptr func-name drop !
    len struct-ptr func-len drop c!
;

: get-lib-name ( struct-ptr -- string len )
    dup >r lib-name drop @
    r> lib-len drop c@
;

: get-func-name ( struct-ptr -- string len )
    dup >r func-name drop @
    r> func-len drop c@
;

: set-lib-handle ( handle struct-ptr )
    lib-handle drop !
;

: set-func-ptr ( ptr struct-ptr )
    func-ptr drop !
;

: get-func-ptr ( struct-ptr -- func-ptr )
    func-ptr drop @
;

: get-lib-handle ( struct-ptr -- handle )
    lib-handle drop @
;

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

: (open-lib) ( struct-ptr ) 
    dup get-lib-name dlopen abort" (open-lib) dlopen failed." \ sp handle
    swap set-lib-handle
;

: (dllookup) { fred }
    fred get-lib-handle 0= abort" Library not opened"

    fred get-func-name fred get-lib-handle dlsym abort" Lookup failed"
    fred set-func-ptr
    
;

: dynamic-dump { fred }
    cr
    ." Library Name    : " fred get-lib-name type cr
    ." Library Handle  : " fred get-lib-handle .h cr
    ." Function Name   : " fred get-func-name type cr
    ." Function Pointer: " fred get-func-ptr .h cr
    ." Inputs          : " fred get-inputs . cr
    ." Outputs         : " fred get-outputs . cr
;

: setup-func { fred }
    fred (open-lib)
    fred (dllookup)
;

create-struct test1

s" libmine.so" test1 set-lib-name
s" test1" test1 set-func-name
3 test1 set-inputs
1 test1 set-outputs
test1 setup-func


test1 /dynamic dump

test1 dynamic-dump

5 1 1 test1 get-func-ptr dlexec

