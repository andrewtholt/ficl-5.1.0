
-1 value ctx 
-1 value ptr 

: init
    255 allocate abort" Allocation failed." to ptr 
    ptr 255 255 fill

    s" /dev/ttyUSB0" modbus-new-rtu abort" modbus-new-rtu failed" to ctx 

    ctx modbus-connect
    ctx 1 modbus-set-slave
    ctx 1 modbus-debug

    cr
    ." Timeout set to " ctx modbus-get-timeout . ." ms" cr

\    ctx 1002 modbus-set-timeout

;

: modbus
    ctx 0 2 ptr modbus-read-registers abort" Read failed"
    ptr 20 dump
;

