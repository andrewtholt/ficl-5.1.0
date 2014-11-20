: off  
  false swap ! 
;

  : on  
    true swap ! 
;

: bounds ( addr len -- addr+len addr )
    over + swap
;
\ 
\ Return the address, and length follwoing the first occurence
\ of the character, c
\
: scan  { addr len c -- addr+i len-i }
    len 0 do
        addr i + c@ c = if
            addr i +
            len i -
            exit
        then
    loop
    i .
;
\
\ delete deletes the first ( n ) bytes from a buffer and fills the 
\ rest at the end with blanks. 
\
: delete   ( addr u n - )  
    over min >r  r@ - ( left over )  dup 0> IF  
        2dup swap dup  r@ +  -rot swap move  
    then  
    + r> bl fill 
;

\ insert inserts as string at the front of a buffer. 
\ The remaining bytes are moved on.

\ To avoid exhausting our memory management, there are only certain 
\ string sizes; $padding takes care of rounding up to multiplies 
\ of four cells. 

: insert   ( string length buffer size - )  
    rot over min >r  r@ - ( left over )  
    over dup r@ +  rot move   r> move  ;

: $padding ( n - n' )  
    [ 6 cells ] Literal + [ -4 cells ] Literal and 
;

: $! ( addr1 u addr2 - )
    dup @ if
    dup @ free throw
    then
    over $padding allocate throw over ! @
    over >r rot over cell+ 
    r> move 2dup ! + cell+ bl swap c! 
;

\ $@ returns the stored string. 
: $@ ( addr1 - addr2 u )  @ dup cell+ swap @ ;

\ $@len returns just the length of a string. 
: $@len ( addr - u )  @ @ ;

\  $!len changes the length of a string. 
\ Therefore we must change the memory area and adjust address and count cell as well. 
\
: $!len ( u addr - )  
  over $padding over @ swap resize throw over ! @ ! ;

\ $del deletes \( u \) bytes from a string with offset \( off \). 
: $del ( addr off u - )   >r >r dup $@ r> /string r@ delete  
    dup $@len r> - swap $!len ;

\ $ins inserts a string at offset ( off ). 

 : $ins ( addr1 u addr2 off - ) >r  
      2dup dup $@len rot + swap $!len  $@ 1+ r> /string insert ;

\ $+! appends a string to another. 
: $+! ( addr1 u addr2 - ) dup $@len $ins ;

\ $off releases a string.  
\ As a bonus, there are functions to split strings up.

: $off ( addr - )  dup @ free throw off ;

\ $split divides a string into two, with one char as 
\ separator (e.g. '? for arguments) 

: $split ( addr u char - addr1 u1 addr2 u2 )  
    >r 2dup r> scan dup >r dup IF  1 /string  THEN  
    2swap r> - 2swap ;

: $iter ( .. $addr char xt - .. ) { char xt }  
    $@ BEGIN  dup  WHILE  char $split >r >r xt execute r> r>  
    REPEAT  2drop ;

