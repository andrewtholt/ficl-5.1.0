include /usr/local/lib/ficl/mydump.fth

-1 value ctx
-1 value ptr

: ?linux
    os 1 = 
;

: ?macos
    os 2 =
;

: tst
    ctx 0 40 ptr modbus-read-registers abort" Read failed"
    ptr 20 mdump

\\    ctx 0 20 ptr modbus-read-input-bits abort" Read failed"
\\   ptr 20 dump
;

variable rtd1
variable dig-out
variable baud-rate

: .rtd1
    ctx 3 1 rtd1 modbus-read-input-registers abort" Read failed"
    rtd1 w@ .
    cr
;

: .dig-out
    ctx 2 1 rtd1 modbus-read-input-registers abort" Read failed"
    rtd1 w@ .
    cr
;

: .baud
    ctx 120 1 baud-rate modbus-read-input-registers abort" Read failed"
    baud-rate w@ .
    cr
;

: modbus
    255 allocate abort" Allocation failed." to ptr
    ptr 255 255 fill


\    s" 192.168.0.190" 502 modbus-new-net abort" modbus-new-rtu failed" to ctx
    s" 192.168.0.244" 502 modbus-new-net abort" modbus-new-rtu failed" to ctx

    ctx modbus-connect
    ctx -1 modbus-error-recovery
    ctx 3 modbus-set-slave

    tst

\    ctx modbus-close
;

.( type modbus to start test ) cr
\ modbus
