include /usr/local/lib/ficl/mydump.fth

-1 value ctx
-1 value ptr

variable tmp

: ?linux
    os 1 = 
;

: ?macos
    os 2 =
;

: tst
    ctx 0 06 ptr modbus-read-input-registers abort" Read failed"
    ptr 20 mdump

\\    ctx 0 20 ptr modbus-read-input-bits abort" Read failed"
\\   ptr 20 dump
;

variable rtd1
variable dig-out
variable baud-rate

: .rtd1
    ctx 0x03 modbus-set-slave
    ctx 3 1 rtd1 modbus-read-input-registers abort" Read failed"
    rtd1 w@ .
    cr
;

: .dig-out
    ctx 0x03 modbus-set-slave
    ctx 2 1 rtd1 modbus-read-input-registers abort" Read failed"
    rtd1 w@ .
    cr
;

: .baud
    ctx 0x03 modbus-set-slave
    ctx 120 1 baud-rate modbus-read-input-registers abort" Read failed"
    baud-rate w@ .
    cr
;

: .current-l1 ( rtu -- I )
    ctx swap modbus-set-slave
    ctx 0 1 tmp modbus-read-input-registers abort" Read failed" 
    tmp w@
;

: init
    255 allocate abort" Allocation failed." to ptr
    ptr 255 255 fill
;

: modbus

    ptr -1 = if
        init
    then

    s" 127.0.0.1" 1502 modbus-new-net abort" modbus-new-rtu failed" to ctx
\    s" 192.168.0.155" 1502 modbus-new-net abort" modbus-new-rtu failed" to ctx

    ctx -1 modbus-debug
    ctx -1 modbus-error-recovery
    ctx 0x01 modbus-set-slave
    ctx modbus-connect

    tst

\    ctx modbus-close
;

.( type modbus to start test ) cr
\ modbus

