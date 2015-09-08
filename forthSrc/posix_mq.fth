
0 value initRun
-1 value mqd
-1 value buffer

: init
    initRun 0= if
        s" /fred" O_RDWR mq-open abort" mq_open" to mqd
        -1 to initRun

        1024 allocate abort" Allocate failed." to buffer
        buffer 1024 erase
    then
;

: main
    init

    mqd MQ_CURMSGS mq-getattr abort" mq-getattr failed" 

    ." Number of waiting messages:" . cr

    begin
        mqd MQ_CURMSGS mq-getattr abort" mq-getattr failed"  0<>
    while

        mqd buffer 1024 0 mq-recv abort" mq-recv Failed." 

        buffer swap dump
        ." =====================================" cr
    repeat

    mqd mq-close abort" mq-close failed."
    buffer free  abort" free failed."

    -1 to buffer
    0 to initRun
;

main

