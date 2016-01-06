
-1 value mosq
1883 constant port

s" mqtt" environment? 0= abort" No mqtt env" drop

: init
    mqtt-lib-init abort" mqtt-lib-init"

    mqtt-new to mosq

    mosq mqtt-connect-callback
    mosq s" localhost" port 60 mqtt-connect
;
