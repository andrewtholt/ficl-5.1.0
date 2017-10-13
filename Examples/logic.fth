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


\ 
\ Redis stuff
\ 
-1 value redis
-1 value reply
\
\ MQTT Stuff
\
-1 value client
0 0 2value mqtt-host
0 value msg 
100 constant /msg

1883 constant port
\ 
\ General stuff
\ 
0 value init-run

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

: redis-connect-picobob
    s" 192.168.0.65" 6379 1 redis-connect
;

: redis-connect-local
    s" 127.0.0.1" 6379 1 redis-connect
;
\ 
\ Initialise and run
\ 
0 value day
0 value back-floodlight

:  /home/environment/day
    evaluate to day

\    day . cr
;

: subscribe
\    client s" /home/office/proliant/power" mqtt-sub abort" mqtt-sub failed"
\    client s" /home/office/proliant/state" mqtt-sub abort" mqtt-sub failed"
    client s" /home/environment/day"       mqtt-sub abort" mqtt-sub failed"
;

: init-redis
    s" redis" environment? if ." Redis known ..." cr then
    0= abort" ... but unavailable"
    ." ... available...." cr
\        redis-connect-local
    redis-connect-picobob
    abort" redis-connect failed" to redis
    
    redis redis-ping abort" ping failed"
    ." ... and up...." cr
;


: init-mqtt
    s" mqtt" environment? 0= abort" No mqtt env" drop
    ." MQTT known ..." cr

    s" 192.168.0.65" strsave to mqtt-host

    /msg allocate abort" allocate failed" to msg
    msg /msg erase

    mqtt-init abort" mqtt-init"
    ." ... Initialised..."  cr

    "FICL" msg mqtt-new abort" mqtt-new failed" to client
    ." ... client created ..." cr
    client mqtt-host port mqtt-client abort" mqtt-client failed"
    ." ... connected...." cr
    subscribe
    ." ... and subscribed." cr
;

: init
    init-run 0= if
        init-redis
        init-mqtt

        -1 to init-run
    then
;
\ 
\ MQTT
\
: logic
    ." logic." cr
    day invert to back-floodlight

    ." Floodlight "
    back-floodlight if 
        ." ON"
        msg s" /home/outside/BackFloodlight/cmnd/power" mqtt-topic!
        msg s" ON" mqtt-payload!
        client msg mqtt-pub abort" mqtt-pub"
    else
        ." OFF"
        msg s" /home/outside/BackFloodlight/cmnd/power" mqtt-topic!
        msg s" OFF" mqtt-payload!
        client msg mqtt-pub abort" mqtt-pub"
    then
    cr
;

: mqtt-process
    init
    msg /msg erase
    0xff msg c!
    begin
        client 750 mqtt-loop 
        msg c@ 0= if
            msg mqtt-topic@   type cr  

            msg mqtt-payload@ type cr  

            msg mqtt-payload@ 
            msg mqtt-topic@ evaluate
            0xff msg c!
            logic
        then
\        depth . cr

    -1 = until
;


: redis-tst
    init
    
    ." GET foo" cr
    redis s" GET foo" redis-cmd abort" cmd failed" to reply
    reply .redis-reply
    reply redis-free-reply

    redis s" SET fred test" redis-cmd abort" cmd failed" to reply
    cr
    ." SET fred test" cr 
    reply .redis-reply
    reply redis-free-reply

    cr
    ." LRANGE mylist 0 -1" cr
    redis s" LRANGE mylist 0 -1" redis-cmd abort" cmd failed" to reply

    reply .redis-reply
    reply redis-free-reply

;

: logic
;

\ tst



