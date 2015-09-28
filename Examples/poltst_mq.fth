

-1 value ptr
0 value initRun
-1 value mqd

-1 value buffer
255 constant /buffer

: .pending
    mqd MQ_CURMSGS mq-getattr abort" mq-getattr failed"
    ." Pending " . cr
;

: init
    initRun 0= if
        16 allocate abort" allocate failed" to ptr
        ptr 16 erase

        /buffer allocate abort" allocate failed" to buffer
        buffer /buffer erase

        s" /Remote" O_RDWR mq-open abort" mq-open failed" to mqd
        -1 to initRun


        ." Before" cr
        ptr 8 dump
    then
;

: rx
    init

    ptr 16 erase
    1 ptr c!
    mqd ptr 1+ c!
    
    ." Poll" cr
    ptr 8 dump
    ptr poll
    ." After" cr
    ptr 8 dump

    ptr 1+ c@ 1 = if
        ." message q" cr
        mqd buffer 1024 0 mq-recv abort" mq-recv Failed." .s

        buffer /buffer dump
        buffer /buffer erase

    then

\    buffer 16 file read-file abort" read-file" .s

\    buffer 16 dump

\    16 allocate abort" allocate failed" to buffer
\    buffer 16 erase
;

0 value count

: send 
    init

    buffer count s>d <# # # # #s #> 0 mq-send abort" mq-send" .s

;
