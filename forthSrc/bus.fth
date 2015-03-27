load lib.fth

-1 value shmid
-1 value semid
-1 value ptr
-1 value shm-size
00 value my-id

10 constant client-count
32 constant bytes/client

client-count bytes/client to shm-size

999 shm-size 896 shmget to shmid

shmid shmat to ptr
semtran to semid

.( semid is ) semid . cr

: clnt-buffer ( id -- ptr size )
    bytes/client * ptr +
    bytes/client
;

my-id clnt-buffer erase
