
-1 value fd
0 value initRun
-1 value ptr

-1 value semid

: init
    initRun 0= if
        s" /STMState" O_RDWR 0x1ff shm-open abort" shm-open failed."
        to fd

        fd 0 53 mmap abort" mmap failed" to ptr

        s" /STMStateLock" O_RDWR 0x1ff 1 sem-create abort" sem-open failed." 
        to semid

        -1 to initRun
    then
;

: main
    init
    semid sem-getvalue abort" sem-getvalue failed."

    ." sem-getvalue : " . cr

    semid sem-wait abort" sem-wait failed."
    ptr 53 dump
    semid sem-post abort" sem-post failed."

;

