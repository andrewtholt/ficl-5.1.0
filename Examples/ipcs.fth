-1 value shmid
-1 value semid
-1 value ptr
-1 value shm-size
-1 value my-id

: main
    s" socket" environment? 0= abort" No sysv ipc env"

    1024 to shm-size

    999 shm-size 896 shmget to shmid

    shmid shmat to ptr

    shmid . cr

    ptr 128 dump

    shmid shmdt abort" Failed to release memory."
    shmid shmrm abort" Failed to delete memeory."
;

main

