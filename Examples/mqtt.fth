
-1 value client
1883 constant port
0 value init-run

s" 192.168.0.65" strsave 2value hostname
0 value msg
100 constant /msg


s" mqtt" environment? 0= abort" No mqtt env" drop

: init
    init-run 0= if
        /msg allocate abort" allocate failed" to msg
        msg /msg erase

        mqtt-init abort" mqtt-init"
    
        "FICL" msg mqtt-new abort" mqtt-new failed" to client
    
\        client mqtt-connect-callback
        client hostname port mqtt-client abort" mqtt-client failed"

        -1 to init-run
    then
;

: stuff
    init
    msg /msg erase
    0xff msg c!
    begin
        client -1 mqtt-loop 
        msg c@ 0= if
            msg mqtt-topic@   type cr 
            msg mqtt-payload@ type cr 
            0xff msg c!
        then

    -1 = until
;

: publish
    init
    msg /msg erase
    
    ." HERE" cr
    msg s" /home/office/proliant/testing" mqtt-topic!
    msg s" TEST" mqtt-payload!
    
    msg /msg dump
    
    client msg mqtt-pub abort" mqtt-pub"
;

: test
    init

\    client s" /home/office/ups/#" mqtt-sub abort" mqtt-sub failed"
    client s" /home/office/proliant/#" mqtt-sub abort" mqtt-sub failed"

    stuff
;
