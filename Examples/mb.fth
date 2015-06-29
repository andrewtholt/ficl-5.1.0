
-1 value ctx 
-1 value ptr 

0 value init-run

: init
    init-run 0= if
        -1 to init-run

        255 allocate abort" Allocation failed." to ptr 
        ptr 255 255 fill

        s" /dev/ttyUSB1" modbus-new-rtu abort" modbus-new-rtu failed" to ctx 

        ctx modbus-connect
        ctx 1 modbus-set-slave
        ctx 1 modbus-debug

        cr
        ." Timeout set to " ctx modbus-get-timeout . ." ms" cr

\       ctx 1002 modbus-set-timeout
    then
;

: modbus
    init-run 0= if
        init
    then

    ctx 0 2 ptr modbus-read-registers abort" Read failed"
    ptr 20 dump
;

: trip
    init-run 0<> if
        0x8000 ptr w!
        ctx 0x10 ptr 1   modbus-write-multiple-registers abort" reset failed."
        drop
    else
        cr ." Run init first." cr
    then
;

: reset
    init-run 0<> if
        0x4000 ptr w!
        ctx 0x10 ptr 1   modbus-write-multiple-registers abort" reset failed."
        drop
    else
        cr ." Run init first." cr
    then
;

: help
    cr
    9 emit ." init"   9 emit ." Setup." cr
    9 emit ." modbus" 9 emit ." Basic test." cr
    9 emit ." trip  " 9 emit ." Trip breaker." cr
    9 emit ." reset " 9 emit ." Reset breaker." cr
    cr
;

.( Try help ) cr
