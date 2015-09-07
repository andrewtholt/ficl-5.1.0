
-1 value fd
0 value initRun

-1 value semid

: init
    initRun 0= if
        s" /STMState" O_RDWR 0x1ff shm-open abort" shm-open failed."
        to fd

        s" /STMStateLock" O_RDWR 0x1ff 1 sem-create abort" sem-open failed." 
        to semid

        -1 to initRun
    then
;

: main
    init
    semid sem-getvalue abort" sem-getvalue failed."

    ." sem-getvalue : " . cr
;

