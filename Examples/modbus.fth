
-1 value ctx
-1 value ptr

: ?linux
    os 1 = 
;

: ?macos
    os 2 =
;


: modbus
    255 allocate abort" Allocation failed." to ptr
    ptr 255 255 fill

    ?macos if
        s" /dev/tty.usbserial-A600drA9"
\    s" /dev/tty.USA28X3d1P2.2"
    then

    ?linux if
        s" /dev/ttyUSB0"
    then
    modbus-new-rtu abort" modbus-new-rtu failed" to ctx

    ctx modbus-connect
    ctx 1 modbus-set-slave

    ctx 0 20 ptr modbus-read-registers abort" Read failed"

    ptr 20 dump

\    ctx modbus-close
;

: tst
    255 0 do 
        i . cr
        i ptr i + !
    loop
;

.( type modbus to start test ) cr
\ modbus
