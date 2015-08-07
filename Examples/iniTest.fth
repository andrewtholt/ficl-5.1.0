
-1 value ini

: init
    ini 0 < if
        s" /tmp/newServer.ini" ini-load
        to ini
    then
;

: test
    init
    ini s" modbus:LOCAL_RTU" 1 ini-getint
    cr
    ." Get int    :" . cr

    ini s" modbus:debug" 0 ini-getbool
    ." Get Boolean:" . cr

    ini s" modbus:tty" ini-getstring
    ." Get String :" type cr
;




