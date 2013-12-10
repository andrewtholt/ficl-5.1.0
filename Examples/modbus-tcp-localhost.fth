include /usr/local/lib/ficl/mydump.fth

0x01 constant relay
0x03 constant iox
0xff constant server

0x00 constant OFF
0x01 constant ON

-1 value ctx
-1 value ptr

variable tmp

: ?linux
    os 1 = 
;

: ?macos
    os 2 =
;

: dump-holding ( RTU -- )
    ctx swap modbus-set-slave
    ctx 0 06 ptr modbus-read-registers 
    if
        modbus-perror
    else
        drop
        ptr 20 mdump
    then

\\    ctx 0 20 ptr modbus-read-input-bits abort" Read failed"
\\   ptr 20 dump
;

: dump-input ( RTU -- )
    ctx swap modbus-set-slave
    ctx 0 06 ptr modbus-read-input-registers if
        modbus-perror
    else
        drop
        ptr 20 mdump
    then
;


variable rtd1
variable dig-out
variable baud-rate

: iox-rtd1 ( RTU - value )
    ctx swap modbus-set-slave
    ctx 3 1 rtd1 modbus-read-input-registers abort" Read failed" drop
    rtd1 w@ 
;

: iox-get-dig-out ( RTU -- dd )
    ctx swap modbus-set-slave
    ctx 2 1 rtd1 modbus-read-input-registers abort" Read failed" drop
    rtd1 w@
;

: iox-set-dig-out ( nn RTU -- )
    ctx swap modbus-set-slave
    >r
    ctx 2
    r> 
    modbus-write-register abort" Write failed"
;

: iox-get-dig-in ( RTU -- dd )
    ctx swap modbus-set-slave
    ctx 1 1 rtd1 modbus-read-input-registers abort" Read failed" drop
    rtd1 w@
;
: line-frequency ( RTU -- hz )
    ctx swap modbus-set-slave
    ctx 108 1 rtd1 modbus-read-input-registers abort" Read failed" drop
    rtd1 w@ 
;

: (get-version-model)
    ctx 0 1 ptr modbus-read-registers abort" Read failed" drop
;

: iox-version
    ctx 0x03 modbus-set-slave
    (get-version-model)
    ptr w@ 0xff00 and 8 rshift
;

: iox-device ( RTU -- type )
    ctx swap modbus-set-slave
    (get-version-model)
    ptr w@ 0xff and
;

: .baud
    ctx 0x03 modbus-set-slave
    ctx 120 1 baud-rate modbus-read-input-registers abort" Read failed"  drop
    baud-rate w@ .
    cr
;

: current-l1 ( rtu -- I )
    ctx swap modbus-set-slave
    ctx 0 1 tmp modbus-read-input-registers abort" Read failed"  drop
    tmp w@
;

: current-l2 ( rtu -- I )
    ctx swap modbus-set-slave
    ctx 1 1 tmp modbus-read-input-registers abort" Read failed"  drop
    tmp w@
;

: current-l3 ( rtu -- I )
    ctx swap modbus-set-slave
    ctx 2 1 tmp modbus-read-input-registers abort" Read failed"  drop
    tmp w@
;


: init
    255 allocate abort" Allocation failed." to ptr
    ptr 255 255 fill

    s" 192.168.0.244" 502 modbus-new-net abort" modbus-new-rtu failed" to ctx
\    s" 192.168.0.142" 502 modbus-new-net abort" modbus-new-rtu failed" to ctx
\    s" 127.0.0.1" 1502 modbus-new-net abort" modbus-new-rtu failed" to ctx
\    s" 192.168.0.155" 1502 modbus-new-net abort" modbus-new-rtu failed" to ctx
\    s" 192.168.0.122" 1502 modbus-new-net abort" modbus-new-rtu failed" to ctx

    ctx 0 modbus-debug
    ctx -1 modbus-error-recovery
    ctx modbus-connect
;

: modbus
    ptr -1 = if
        init
    then

    ctx relay modbus-set-slave
    dump-holding

\    ctx modbus-close
;

: pattern
    10 0 do
        i 0x30 +  ptr i + c!
    loop
;

: t1 ( RTU -- )
    ctx swap modbus-set-slave
    pattern

    ctx 0000 ptr 5 modbus-write-multiple-registers if
        modbus-perror
    then
       


;

: do1 ( state -- )
    ctx iox modbus-set-slave
    >r
    ctx 0x10 r> modbus-write-coil

;

.( type modbus to start test ) cr
\ modbus

s" modbus-ok> " 0 set-prompt

init


: op-volts ( mV -- )
    ctx 1 modbus-set-slave

    4097 10000 */ ctx 24 rot modbus-write-register abort" op-volts"
;


