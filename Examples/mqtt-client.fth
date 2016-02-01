
-1 value mosq

: init
    mqtt-new to mosq
    mqtt-lib-init 
    mosq mqtt-connect-callback
    
    mosq s" localhost" 1883 120 mqtt-connect abort" Connect failed"

;
