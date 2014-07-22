\
\ Status values from replies
\
1 constant REDIS_REPLY_STRING 
2 constant REDIS_REPLY_ARRAY 
3 constant REDIS_REPLY_INTEGER
4 constant REDIS_REPLY_NIL 
5 constant REDIS_REPLY_STATUS
6 constant REDIS_REPLY_ERROR

0x01 constant REDIS_BLOCK
0x02 constant REDIS_CONNECTED
0x04 constant REDIS_DISCONNECTING 
0x08 constant REDIS_FREEING 
0x10 constant REDIS_IN_CALLBACK 
0x20 constant REDIS_SUBSCRIBED
0x40 constant REDIS_MONITORING


-1 value redis
-1 value reply

: redis-string?
    REDIS_REPLY_STRING =
;

: redis-integer?
    REDIS_REPLY_INTEGER =
;

: redis-array?
    REDIS_REPLY_ARRAY =
;

: tab 09 emit ;

: .redis-reply ( reply -- )
    cr
    >r
    r@ redis-get-type dup ." Type = " .  cr
    tab

    case
        REDIS_REPLY_STRING  of ." String <"  
                r@ redis-get-string type ." >" cr
            endof
        REDIS_REPLY_ARRAY   of ." Array" cr
                tab
                r@ redis-get-elements ." Length " dup . cr
                ?dup 0<> if
                    0 do
                        tab tab i . tab
                        reply i redis-get-element type cr
                    loop
                then
            endof
        REDIS_REPLY_INTEGER of ." Integer" endof
        REDIS_REPLY_NIL     of ." NILL"    endof
        REDIS_REPLY_STATUS  of ." Status <"  
                r@ redis-get-string type ." >" cr
            endof
        REDIS_REPLY_ERROR   of ." Error"  endof
    endcase

    r> drop
;

: .redis-context ( context -- )
    >r
    cr
    ." Error " r@ redis-context-get-err dup . cr
    0= if tab ." No messages" cr then
    ." Flags " r@ redis-context-get-flags dup . cr
    dup REDIS_BLOCK         and if tab ." Blocking" cr then
    dup REDIS_CONNECTED     and if tab ." Connected" cr then
    dup REDIS_DISCONNECTING and if tab ." Disconnecting" cr then
    dup REDIS_FREEING       and if tab ." Freeing" cr then
    dup REDIS_IN_CALLBACK   and if tab ." In Callback" cr then
    dup REDIS_SUBSCRIBED    and if tab ." Subscribed" cr then
        REDIS_MONITORING    and if tab ." Monitoring" cr then

    r> drop
;

: redis-connect-local
    s" 127.0.0.1" 6379 1 redis-connect
;

: sub
    redis s" SUBSCRIBE foo" redis-cmd abort" cmd failed" to reply
    reply .redis-reply
    reply redis-free-reply
;

: tst
    s" redis" environment? if ." Redis known ..." then

    0= abort" ... but unavailable"

    ." ... and available." cr
    redis-connect-local
    
    abort" redis-connect failed" to redis
    
    redis redis-ping abort" ping failed"
    
    sub
\    redis s" SUBSCRIBE foo" redis-cmd abort" cmd failed" to reply


;

tst
