
-1 value fd
0 value initRun

: init
    s" /STMState" O_RDWR 0x1ff shm-open abort" shm-open failed."
    to fd

    -1 to initRun
;
