
s" iniparser" environment? 0= abort" No ini file parser."

-1 value ini

: init
    ini 0 < if
        s" ./newServer.ini" ini-load
        to ini
    then
;

: test
    init
    ini s" modbus:LOCAL_RTU" 1 ini-getint
    cr
    ." Get int (LOCAL_RTU):" . cr

    ini s" modbus:debug" 0 ini-getbool
    ." Get Boolean (DEBUG):" . cr

    ini s" modbus:tty" ini-getstring
    ." Get String    (TTY):" type cr
;

cr 
.( Type test to start ) cr


